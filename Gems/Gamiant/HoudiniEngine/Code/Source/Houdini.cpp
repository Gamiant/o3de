/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <HoudiniCommon.h>
#include <HoudiniEngine/HoudiniVersion.h>
#include <HoudiniSettings.h>

#include <Util/EditorUtils.h>

#include <AzCore/IO/FileIO.h>
#include <AzFramework/IO/LocalFileIO.h>
#include <AzFramework/Process/ProcessWatcher.h>

#include <AzCore/Component/TickBus.h>

#if defined (AZ_PLATFORM_WINDOWS)
#include <Shlobj.h>
#endif


#if defined(AZ_PLATFORM_WINDOWS)

//AZ_Warning("HOUDINI-COOK", false, (AZStd::string(__FILE__)+ AZStd::string(": ") + QString::number(__LINE__)).c_str());\

#define ENSURE_SUCCESS( result ) \
if ( (result) != HAPI_RESULT_SUCCESS ) \
{ \
    CheckForErrors(true);\
}
#define ENSURE_COOK_SUCCESS( result ) \
if ( (result) != HAPI_RESULT_SUCCESS ) \
{ \
    CheckForErrors(true);\
}
#endif

#define HOUDINI_MODE_EMBEDDED 0 
#define HOUDINI_MODE_ASYNC 1 


namespace HoudiniEngine
{

    Houdini::Houdini() 
        : m_isActive(false)
        , m_initialized(false)
    {
        SessionRequestBus::Handler::BusConnect();

        AZ_Info("Houdini", "Houdini Engine Sync Init ------------------------------------------------------------------------------------\n");

        HAPI_Result result = HAPI_RESULT_FAILURE;

        m_houdiniPath = AZStd::string::format("C:\\Program Files\\Side Effects Software\\Houdini %s\\bin\\", HoudiniVersionString.data());
        AZ_Info("Houdini", "Houdini Path: %s\n", m_houdiniPath.c_str());

        const auto gemAlias = AZ::StringFunc::Path::FixedString::format("@gemroot:HoudiniEngine@/External/%s/bin", HoudiniVersionString.data());

        char resolvedLevelPath[AZ_MAX_PATH_LEN] = { 0 };
        AZ::IO::FileIOBase::GetDirectInstance()->ResolvePath(gemAlias.c_str(), resolvedLevelPath, AZ_MAX_PATH_LEN);
        m_HAPIlibPath = resolvedLevelPath;
        AZ_Info("Houdini", "HAPI Lib Path: %s\n", m_HAPIlibPath.c_str());

        int houdiniVersionMajor = -1, houdiniVersionMinor = -1, houdiniVersionBuild = -1, houdiniVersionPatch = -1;
        HAPI_GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_MAJOR, &houdiniVersionMajor);
        HAPI_GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_MINOR, &houdiniVersionMinor);
        HAPI_GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_BUILD, &houdiniVersionBuild);
        HAPI_GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_PATCH, &houdiniVersionPatch);
        AZ_Info("Houdini", "Houdini Version: %d.%d.%d.%d\n", houdiniVersionMajor, houdiniVersionMinor, houdiniVersionBuild, houdiniVersionPatch);

        int engineMajor = -1, engineMinor = -1;
        HAPI_GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_ENGINE_MAJOR, &engineMajor);
        HAPI_GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_ENGINE_MINOR, &engineMinor);
        AZ_Info("Houdini", "Houdini Engine Version: %d.%d\n", engineMajor, engineMinor);

        result = HAPI_IsInitialized(nullptr);
        if (result == HAPI_RESULT_NOT_INITIALIZED || result != HAPI_RESULT_SUCCESS)
        {
            AZ_Info("Houdini", "HAPI needs to be initialized\n");
        }

        SessionSettings* settings = nullptr;
        SettingsBus::BroadcastResult(settings, &SettingsBusRequests::GetSessionSettings);
        AZ_Assert(settings, "Settings cannot be null");

        m_namedPipe = settings->GetNamedPipe();
        m_sessionType = settings->GetSessionType();
        m_serverHost = settings->GetServerHost();
        m_serverPort = settings->GetServerPort();

        AZ_Info("Houdini", "Session Type: %s\n", (m_sessionType == SessionSettings::ESessionType::TCPSocket) ? "TCP Socket" : "Named Pipe");
        AZ_Info("Houdini", "Server Host: %s\n", m_serverHost.c_str());
        AZ_Info("Houdini", "Server Port: %d\n", m_serverPort);
        AZ_Info("Houdini", "Named Pipe: %s\n", m_namedPipe.c_str());

        ComputeSearchPaths();

        for (auto& path : m_searchPaths)
        {
            AZ_Info("Houdini", "Search Path: %s\n", path.c_str());
        }

        AZ_Info("Houdini", "---------------------------------------------------------------------------------------------------------------\n");

        AZ::TickBus::Handler::BusConnect();
        AZ::SystemTickBus::Handler::BusConnect();

    }

    Houdini::~Houdini()
    {
        AZ::TickBus::Handler::BusDisconnect();
        //Unload all assets and cleanup, destroy session.
        Shutdown();
        m_rootNode.reset();
    }

    void Houdini::SetupEnvironment()
    {
        HoudiniEngineUtils::SetEnvironmentVariable("HAPI_CLIENT_NAME", "o3de");

        if (!m_environmentSet)
        {
            const char* pathDelimiter = HoudiniEngineUtils::GetPathVarDelimiter();
            AZStd::string sourcePath = HoudiniEngineUtils::GetEnvironmentVariable("PATH");
            AZStd::string updatedPath = m_houdiniPath + pathDelimiter + sourcePath;
            HoudiniEngineUtils::SetEnvironmentVariable("PATH", updatedPath);
            m_environmentSet = true;
        }

        // The HFS environment variable is requiredfor a connection
        AZStd::string HFSSourcePath = HoudiniEngineUtils::GetEnvironmentVariable("HFS");
        if (HFSSourcePath.empty())
        {
            HFSSourcePath = m_houdiniPath;
            HoudiniEngineUtils::SetEnvironmentVariable("HFS", HFSSourcePath);
        }
    }

    void Houdini::ComputeSearchPaths()
    {
        SessionSettings* settings = nullptr;
        SettingsBus::BroadcastResult(settings, &SettingsBusRequests::GetSessionSettings);
        AZ_Assert(settings, "Settings cannot be null");

        std::string otl = std::string(settings->GetOTLPath().c_str());
        auto start = 0U;
        auto end = otl.find(';');
        while (end != std::string::npos)
        {
            m_searchPaths.push_back(AZStd::string(otl.substr(start, end - start).c_str()));
            start = (unsigned int)(end + 1);
            end = otl.find(';', start);
        }

        m_searchPaths.push_back(AZStd::string(otl.substr(start, end).c_str()));
    }

    bool Houdini::ConnectSession(SessionSettings::ESessionType sessionType, const AZStd::string& namedPipe, const AZStd::string& serverHost, AZ::u32 serverPort)
    {
        // If we have a session, we're connected
        if (HAPI_IsSessionValid(&m_session) == HAPI_RESULT_SUCCESS)
        {
            AZ_Info("Houdini", "Houdini::ConnectSession valid session");
            return true;
        }

        SetupEnvironment();

        SessionSettings* settings = nullptr;
        SettingsBus::BroadcastResult(settings, &SettingsBusRequests::GetSessionSettings);
        AZ_Assert(settings, "Settings cannot be null");

        HAPI_ThriftServerOptions ServerOptions = {};
        ServerOptions.autoClose = true;
        ServerOptions.timeoutMs = settings->GetAutoCloseTimeOut();

        HAPI_ClearConnectionError();

        HAPI_Result sessionResult = HAPI_RESULT_FAILURE;

        switch (sessionType)
        {
        case SessionSettings::ESessionType::TCPSocket:

            // Try to connect to an existing socket session first
            sessionResult = HAPI_CreateThriftSocketSession(&m_session, serverHost.c_str(), serverPort);
            break;

        case SessionSettings::ESessionType::NamedPipe:

            // Try to connect to an existing socket session first
            sessionResult = HAPI_CreateThriftNamedPipeSession(&m_session, namedPipe.c_str());
            break;

        default:
            AZ_Error("Houdini", false, "Invalid Session Type");
            break;
        }

        if (sessionResult != HAPI_RESULT_SUCCESS)
        {
            // This may happen is Houdini has not yet loaded, we'll keep trying
            return false;
        }

        sessionResult = HAPI_GetSessionEnvInt(&m_session, HAPI_SESSIONENVINT_LICENSE, (int*)&m_licenseType);
        if (sessionResult != HAPI_RESULT_SUCCESS)
        {
            AZStd::string codeString;
            HoudiniEngineRequestBus::BroadcastResult(codeString, &HoudiniEngineRequests::GetHoudiniResultByCode, sessionResult);
            AZ_Error("Houdini", false, "License verification failed: %s - %s", codeString.c_str(), HoudiniEngineUtils::GetLastError().c_str());
            return false;
        }

        return true;
    }

    bool Houdini::InitializeHAPISession()
    {
        // If we have a session, we're connected
        if (HAPI_IsSessionValid(&m_session) != HAPI_RESULT_SUCCESS)
        {
            AZ_Info("Houdini", "HAPI session is invalid");
            return false;
        }

        m_cookOptions = HAPI_CookOptions_Create();
        m_cookOptions.curveRefineLOD = 8.0f;
        m_cookOptions.clearErrorsAndWarnings = false;
        m_cookOptions.maxVerticesPerPrimitive = 3;
        m_cookOptions.splitGeosByGroup = false;
        m_cookOptions.refineCurveToLinear = true;
        m_cookOptions.handleBoxPartTypes = false;
        m_cookOptions.handleSpherePartTypes = false;
        m_cookOptions.splitPointsByVertexAttributes = false;
        m_cookOptions.packedPrimInstancingMode = HAPI_PACKEDPRIM_INSTANCING_MODE_DISABLED;
        m_cookOptions.clearErrorsAndWarnings = true;

        bool useCookingThread = true;
        int cookingThreadStackSize = -1;

        HAPI_Result result = HAPI_RESULT_FAILURE;
        result = HAPI_Initialize(&m_session, &m_cookOptions, useCookingThread, cookingThreadStackSize, nullptr, nullptr, nullptr, nullptr, nullptr);
        if (result == HAPI_RESULT_SUCCESS)
        {
            AZ_Info("Houdini", "Successfully initialized Houdini Engine");
        }

        HAPI_SetServerEnvString(&m_session, "HAPI_CLIENT_NAME", "o3de");

        ConfigureSession();

        return true;
    }

    void Houdini::ConfigureSession()
    {
        SessionSettings* settings = nullptr;
        SettingsBus::BroadcastResult(settings, &SettingsBusRequests::GetSessionSettings);
        AZ_Assert(settings, "Settings cannot be null");

        // Set the Session Sync settings to Houdini
        HAPI_SessionSyncInfo sessionSyncInfo;
        sessionSyncInfo.cookUsingHoudiniTime = settings->GetCookUsingHoudiniTime();
        sessionSyncInfo.syncViewport = settings->GetSyncViewports();

        if (HAPI_SetSessionSyncInfo(&m_session, &sessionSyncInfo) != HAPI_RESULT_SUCCESS)
        {
            AZ_Warning("Houdini", false, "Failed to configure Houdini Engine Session");
        }
        else
        {
            AZ_Info("Houdini", "Houdini Engine Session Configured");
        }
    }

    bool Houdini::StartSession(SessionSettings::ESessionType sessionType, const AZStd::string& namedPipe, const AZStd::string& serverHost, AZ::u32 serverPort)
    {
        // If we have a session, we're connected
        if (HAPI_IsSessionValid(&m_session) == HAPI_RESULT_SUCCESS)
        {
            AZ_Info("Houdini", "Houdini::ConnectSession valid session");
            return true;
        }

        HAPI_Result SessionResult = HAPI_RESULT_FAILURE;

        SessionSettings* settings = nullptr;
        SettingsBus::BroadcastResult(settings, &SettingsBusRequests::GetSessionSettings);
        AZ_Assert(settings, "Settings cannot be null");

        HAPI_ThriftServerOptions ServerOptions = {};
        ServerOptions.autoClose = true;
        ServerOptions.timeoutMs = settings->GetAutoCloseTimeOut();

        switch (sessionType)
        {
            case SessionSettings::ESessionType::TCPSocket:
            {
                SessionResult = HAPI_CreateThriftSocketSession(&m_session, serverHost.c_str(), serverPort);

                // Start a session and try to connect to it if we failed
                if (SessionResult != HAPI_RESULT_SUCCESS)
                {
                    SetupEnvironment();
                    HAPI_StartThriftSocketServer(&ServerOptions, serverPort, nullptr, nullptr);

                    SessionResult = HAPI_CreateThriftSocketSession(&m_session, serverHost.c_str(), serverPort);
                }
            }
                break;

            case SessionSettings::ESessionType::NamedPipe:
            {
                SessionResult = HAPI_CreateThriftNamedPipeSession(&m_session, namedPipe.c_str());

                // Start a session and try to connect to it if we failed
                if (SessionResult != HAPI_RESULT_SUCCESS)
                {
                    SetupEnvironment();
                    HAPI_StartThriftNamedPipeServer(&ServerOptions, namedPipe.c_str(), nullptr, nullptr);

                    SessionResult = HAPI_CreateThriftNamedPipeSession(&m_session, namedPipe.c_str());
                }
            }
            break;

            default:
                AZ_Error("Houdini", false, "Invalid Session Type");
                break;
        }

        HAPI_Session* sessionPtr = &m_session;
        if (SessionResult != HAPI_RESULT_SUCCESS || !sessionPtr)
        {
            AZ_Error("Houdini", false, HoudiniEngineUtils::GetConnectionError().c_str());

            return false;
        }

        SessionResult = HAPI_GetSessionEnvInt(&m_session, HAPI_SESSIONENVINT_LICENSE, (int*)&m_licenseType);
        if (SessionResult != HAPI_RESULT_SUCCESS)
        {
            AZStd::string codeString;
            HoudiniEngineRequestBus::BroadcastResult(codeString, &HoudiniEngineRequests::GetHoudiniResultByCode, SessionResult);
            AZ_Error("Houdini", false, "HAPI_SESSIONENVINT_LICENSE failed: %s - %s", codeString.c_str(), HoudiniEngineUtils::GetLastError().c_str());
            return false;
        }

        return true;
    }

    void Houdini::OpenHoudini()
    {
        SessionSettings* settings = nullptr;
        SettingsBus::BroadcastResult(settings, &SettingsBusRequests::GetSessionSettings);
        AZ_Assert(settings, "Settings cannot be null");

        AZStd::string sessionArgs = "-hess=";
        SessionSettings::ESessionType sessionType = settings->GetSessionType();
        if (sessionType == SessionSettings::ESessionType::TCPSocket)
        {
            sessionArgs += AZStd::string::format("port:%d", settings->GetServerPort());
        }
        else
        if (sessionType == SessionSettings::ESessionType::NamedPipe)
        {
            sessionArgs += AZStd::string::format("pipe:%s", settings->GetNamedPipe().c_str());
        }
        else
        {
            AZ_Error("Houdini", false, "Houdini - Invalid Session Type");
            return;
        }

        AzFramework::ProcessLauncher::ProcessLaunchInfo processLaunchInfo;
        processLaunchInfo.m_commandlineParameters = AZStd::string::format("%shoudini %s", m_houdiniPath.c_str(), sessionArgs.c_str());
        processLaunchInfo.m_showWindow = true;
        processLaunchInfo.m_tetherLifetime = true;
        m_houdiniProcessWatcher = AZStd::unique_ptr<AzFramework::ProcessWatcher>(AzFramework::ProcessWatcher::LaunchProcess(processLaunchInfo, AzFramework::ProcessCommunicationType::COMMUNICATOR_TYPE_NONE));

        // We'll connect to the system tick bus where we'll try to create a session while Houdini loads
        m_startSyncTime = std::chrono::steady_clock::now();
        m_startingSession = true;
    }

    void Houdini::OnSystemTick()
    {
        SessionSync(); // Used during loading to connect the Houdini session

        if (m_viewportSync == SessionSettings::EViewportSync::Both || m_viewportSync == SessionSettings::EViewportSync::HoudiniToO3DE)
        {
            m_viewport.SyncToO3DE();
        }

        if (m_viewportSync == SessionSettings::EViewportSync::Both || m_viewportSync == SessionSettings::EViewportSync::O3DEToHoudini)
        {
            m_viewport.SyncToHoudini();
        }

    }

    void Houdini::SessionSync()
    {
        if (m_startingSession)
        {
            SessionSettings* settings = nullptr;
            SettingsBus::BroadcastResult(settings, &SettingsBusRequests::GetSessionSettings);
            AZ_Assert(settings, "Settings cannot be null");

            SessionSettings::ESessionType sessionType = settings->GetSessionType();

            if (!ConnectSession(sessionType, settings->GetNamedPipe(), settings->GetServerHost(), settings->GetServerPort()))
            {
                // Update timeout, Houdini may not be done loading
                auto elapsedMs = AZStd::chrono::duration_cast<AZStd::chrono::milliseconds>(AZStd::chrono::steady_clock::now() - m_startSyncTime);
                if (elapsedMs.count() >= settings->GetAutoCloseTimeOut())
                {
                    AZ::SystemTickBus::Handler::BusDisconnect();
                    AZ_Error("Houdini", false, "Houdini Engine Sync Session connection timed out\n");
                    return;
                }
            }
            else
            {
                AZ_Info("Houdini", "Houdini Session Estabished ------------------------\n");
                AZ_Info("Houdini", "Session Type: %s\n", sessionType == SessionSettings::ESessionType::TCPSocket ? "TCP Socket" : "Named Pipe");
                if (sessionType == SessionSettings::ESessionType::TCPSocket)
                {
                    AZ_Info("Houdini", "Server Host: %s\n", settings->GetServerHost().c_str());
                    AZ_Info("Houdini", "Server Port: %d\n", settings->GetServerPort());
                }
                else
                {
                    AZ_Info("Houdini", "Named Pipe %s\n", settings->GetNamedPipe().c_str());
                }
                AZ_Info("Houdini", "----------------------------------------------------\n");

                ConfigureSession();

                m_startingSession = false;
            }
        }

    }

    void Houdini::StartSession()
    {
        SessionSettings* settings = nullptr;
        SettingsBus::BroadcastResult(settings, &SettingsBusRequests::GetSessionSettings);
        AZ_Assert(settings, "Settings cannot be null");

        if (!StartSession(settings->GetSessionType(), settings->GetNamedPipe(), settings->GetServerHost(), settings->GetServerPort()))
        {
            AZ_Error("Houdini", false, "Failed to start Houdini Engine session");
        }

        if (!InitializeHAPISession())
        {
            AZ_Error("Houdini", false, "Failed to initialize Houdini Engine");
        }
    }

    void Houdini::StopSession()
    {
        if (HAPI_IsSessionValid(&m_session) == HAPI_RESULT_SUCCESS)
        {
            HAPI_Cleanup(&m_session);
            HAPI_CloseSession(&m_session);
        }

        m_session.id = -1;
        m_session.type = HAPI_SESSION_MAX;

        AZ_Info("Houdini", "Houdini Engine Session Stopped\n");
    }

    void Houdini::RestartSession()
    {
        AZ_Info("Houdini", "Restarting Houdini Engine Session\n");

        // Stop the current session
        StopSession();

        SessionSettings* settings = nullptr;
        SettingsBus::BroadcastResult(settings, &SettingsBusRequests::GetSessionSettings);
        AZ_Assert(settings, "Settings cannot be null");

        if (!StartSession(settings->GetSessionType(), settings->GetNamedPipe(), settings->GetServerHost(), settings->GetServerPort()))
        {
            AZ_Error("Houdini", false, "Failed to restart Houdini Engine session");
            return;
        }

        if (!InitializeHAPISession())
        {
            AZ_Error("Houdini", false, "Failed to initialize Houdini Engine");
        }
    }

    void Houdini::SetViewportSync(int index)
    {
        m_viewportSync = static_cast<SessionSettings::EViewportSync>(index);
    }

    HAPI_Session* Houdini::GetSessionPtr()
    {
        return &m_session;
    }

    void Houdini::ExecuteCommand(AZ::EntityId newId, AZStd::function<bool()> functionToCall)
    {
        auto thisThreadId = AZStd::this_thread::get_id();

        if (m_workerThread.size() > 0 && m_workerThreadIds.size() > 0 && thisThreadId != m_workerThreadIds[0])
        {
            AZStd::unique_lock<AZStd::mutex> theLock(m_lock);
            m_work.push(AZStd::make_pair(newId, functionToCall));
        }
        else
        {
            //Inline call since we are not in multi-threaded mode, or we are already on the processor thread!
            functionToCall();
        }
    }

    void Houdini::RaiseCommandPriority(AZ::EntityId /*newId*/)
    {
        AZStd::unique_lock<AZStd::mutex> theLock(m_lock);

        /*auto& buffer = m_work.get_container();

        AZStd::vector<AZStd::pair<AZ::EntityId, AZStd::function<bool()>>> items;
        for( auto it = buffer.begin(); it < buffer.end(); it++)
        {
            if (it->first == newId)
            {
                items.push_back(*it);
                buffer.erase(it);
            }
        }

        if (items.size() > 0)
        {
            for (auto& item : items)
            {
                buffer.push_front(item);
            }
        }*/
    }

    void Houdini::ThreadProcessor()
    {
        m_workerThreadIds.push_back(AZStd::this_thread::get_id());
        while (true)
        {
            //Lock the work thread, this lock signals that we are processing an item:
            {
                AZStd::unique_lock<AZStd::mutex> theWorkLock(m_threadWorkingLock);

                bool hasFunction = false;
                AZStd::function<bool()> function;
                {
                    //Lock the fetch for new work, so no one adds something while we're getting work.
                    AZStd::unique_lock<AZStd::mutex> theFetchNewWorkLock(m_lock);
                    if (m_work.empty() == false)
                    {
                        hasFunction = true;
                        function = m_work.front().second;
                        m_work.pop();
                    }
                }

                if (hasFunction)
                {
                    function();
                }
            }

            //Work finished, should we keep going? Or Sleep for a bit?
            bool shouldSleep = false;
            {
                AZStd::unique_lock<AZStd::mutex> theFetchNewWorkLock(m_lock);
                if (m_work.empty())
                {
                    shouldSleep = true;
                }
            }

            if (shouldSleep) 
            {
                AZStd::this_thread::sleep_for(AZStd::chrono::milliseconds(32));
            }
            else
            {
                AZStd::this_thread::yield();
            }
        }
    }

    void Houdini::CancelProcessorThread()
    {
        //Make sure we have the queue lock and remove all future work:
        {
            AZStd::unique_lock<AZStd::mutex> theFetchNewWorkLock(m_lock);
            while (m_work.empty() == false)
            {
                //Clear out the future work.
                m_work.pop();
            }
        }

        //Wait until the thread finishes whatever its working on now:
        AZStd::unique_lock<AZStd::mutex> theWorkLock(m_threadWorkingLock);
        
        //Thread is done at this point.
    }

    void Houdini::CancelProcessorJob(AZ::EntityId /*entityIdToRemove*/)
    {
        //Grab the work list:
        AZStd::unique_lock<AZStd::mutex> theFetchNewWorkLock(m_lock);

        //Wait for whatever is currently processing to finish:
        AZStd::unique_lock<AZStd::mutex> theWorkLock(m_threadWorkingLock);

        //We now have full control of the list:
        /*auto& buffer = m_work.get_container();

        AZStd::remove_if(buffer.begin(), buffer.end(), [this, entityIdToRemove](AZStd::pair<AZ::EntityId, AZStd::function<bool()>> pair)
        {
            if (pair.first == entityIdToRemove)
            {
                return true;
            }
            
            return false;
        });*/

    }

    void Houdini::JoinProcessorThread()
    {
        //Check if the work to do is empty:
        while (true)
        {
            AZStd::unique_lock<AZStd::mutex> theFetchNewWorkLock(m_lock);
            if (m_work.empty() == false)
            {
                AZStd::this_thread::yield();
            }
            else 
            {
                //ALL work is done now
                break;
            }
        }                
        
        //Wait until the thread finishes whatever its working on now:
        AZStd::unique_lock<AZStd::mutex> theWorkLock(m_threadWorkingLock);
    }

    int Houdini::GetHoudiniMode()
    {
        if (m_workerThread.size() > 0 )
        {
            return HOUDINI_MODE_ASYNC;
        }

        return HOUDINI_MODE_EMBEDDED;
    }

    void Houdini::GetProgress(AZStd::string& statusText, int& statusPercent, int& assetsInQueue)
    {
        AZStd::unique_lock<AZStd::mutex> theWorkLock(m_statusLock);
        statusText = m_currentStatus;
        statusPercent = m_currentPercent;

        AZStd::unique_lock<AZStd::mutex> theFetchNewWorkLock(m_lock);        
        assetsInQueue = (int)m_work.size();
    }

    void Houdini::SetProgress(const AZStd::string& statusText, int statusPercent)
    {
        AZStd::unique_lock<AZStd::mutex> theWorkLock(m_statusLock);
        m_currentStatus = statusText;
        m_currentPercent = statusPercent;
    }

    void Houdini::OnTick(float /*deltaTime*/, AZ::ScriptTimePoint /*time*/)
    {

        //Apply any requested refreshes
        //Uses a time slice to keep framerate high in entity heavy levels.
        {
            AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);

            AZ::EntityId start = m_timeSlice;
            auto firstEntity = m_lookups.lower_bound(start);

            if (firstEntity == m_lookups.end())
            {
                firstEntity = m_lookups.begin();
                m_timeSlice = AZ::EntityId();
            }

            int count = 0;
            for (auto& entityPair = firstEntity; entityPair != m_lookups.end(); )
            {
                if (entityPair->first.IsValid() == false)
                {
                    entityPair++;
                    continue;
                }

                LookupId(entityPair->first);

                count++;
                if (count == m_maxSliceCount)
                {
                    m_timeSlice = entityPair->first;
                    break;
                }                

                entityPair++;
                if (entityPair == m_lookups.end())
                {
                    //Clear the last scan.
                    m_timeSlice = AZ::EntityId();
                }
            }
        }

        
    }

    //Collects data that might be needed by update systems or creation systems. This is required to ensure thread safety.
    void Houdini::RemoveLookupId(const AZ::EntityId& id)
    {
        AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);
        auto existing = m_lookups.find(id);
        if (existing != m_lookups.end())
        {
            m_lookups.erase(existing);
        }
    }


    //Collects data that might be needed by update systems or creation systems. This is required to ensure thread safety.
    void Houdini::LookupId(const AZ::EntityId& id)
    {
        AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);
        auto existing = m_lookups.find(id);
        if (existing == m_lookups.end())
        {
            m_lookups[id] = HoudiniEntityContext();
        }

        HoudiniEntityContext& data = m_lookups[id];
        AZ::ComponentApplicationBus::BroadcastResult(data.m_entity, &AZ::ComponentApplicationRequests::FindEntity, id);
        AZ::ComponentApplicationBus::BroadcastResult(data.m_entityName, &AZ::ComponentApplicationRequests::GetEntityName, id);
        LmbrCentral::SplineComponentRequestBus::EventResult(data.m_spline, id, &LmbrCentral::SplineComponentRequests::GetSpline);
        AzToolsFramework::ToolsApplicationRequests::Bus::BroadcastResult(data.m_isSelected, &AzToolsFramework::ToolsApplicationRequests::IsSelected, id);
        AZ::TransformBus::EventResult(data.m_transform, id, &AZ::TransformBus::Events::GetWorldTM);

        AZ::EBusAggregateResults<AZStd::string> attribList;
        HoudiniCurveAttributeRequestBus::EventResult(attribList, id, &HoudiniCurveAttributeRequests::GetName);        

        data.m_attributeNames.clear();
        for (auto attrib : attribList.values)
        {
            LmbrCentral::SplineAttribute<float>* attribData;
            HoudiniCurveAttributeRequestBus::Event(id, &HoudiniCurveAttributeRequests::GetFloatAttribute, attrib, attribData);

            if (attribData)
            {
                data.m_attributeNames.push_back(attrib);
                data.m_attributes[attrib] = attribData;
            }
        }
    }

    bool Houdini::LookupIsSelected(const AZ::EntityId& id)
    {
        AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);
        auto existing = m_lookups.find(id);
        if (existing == m_lookups.end())
        {
            return false;
        }

        return m_lookups[id].m_isSelected;
    }

    AZ::Entity* Houdini::LookupFindEntity(const AZ::EntityId& id)
    {
        AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);
        auto existing = m_lookups.find(id);
        if (existing == m_lookups.end())
        {
            return nullptr;
        }

        return m_lookups[id].m_entity;
    }

    AZStd::string Houdini::LookupEntityName(const AZ::EntityId& id)
    {
        AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);
        auto existing = m_lookups.find(id);
        if (existing == m_lookups.end())
        {
            return "";
        }

        return m_lookups[id].m_entityName;
    }

    AZ::SplinePtr Houdini::LookupSpline(const AZ::EntityId& id)
    {
        AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);
        auto existing = m_lookups.find(id);
        if (existing == m_lookups.end())
        {
            return nullptr;
        }
        return m_lookups[id].m_spline;
    }

    AZStd::vector<AZStd::string> Houdini::LookupAttributeNames(const AZ::EntityId& id)
    {
        AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);
        auto existing = m_lookups.find(id);
        if (existing == m_lookups.end())
        {
            return AZStd::vector<AZStd::string>();
        }
        return m_lookups[id].m_attributeNames;
    }

    LmbrCentral::SplineAttribute<float>* Houdini::LookupAttributeData(const AZ::EntityId& id, const AZStd::string& attribName)
    {
        AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);
        auto existing = m_lookups.find(id);
        if (existing == m_lookups.end())
        {
            return nullptr;
        }

        auto existingData = m_lookups[id].m_attributes.find(attribName);
        if (existingData == m_lookups[id].m_attributes.end())
        {
            return nullptr;
        }

        return m_lookups[id].m_attributes[attribName];
    }

    AZ::Transform Houdini::LookupTransform(const AZ::EntityId& id)
    {
        AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);
        auto existing = m_lookups.find(id);
        if (existing == m_lookups.end())
        {
            return AZ::Transform::CreateIdentity();
        }

        return m_lookups[id].m_transform;
    }

    HoudiniEngine::HoudiniEntityContext Houdini::LookupEntityData(const AZ::EntityId& id)
    {
        AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);
        auto existing = m_lookups.find(id);
        if (existing == m_lookups.end())
        {
            return HoudiniEntityContext();
        }
        
        return m_lookups[id];
    }

    bool Houdini::CheckForErrors(bool printErrors, bool includeCookingErrors)
    {
        AZ_PROFILE_FUNCTION(Editor);

        bool hasError = false;

        auto err = GetLastHoudiniError();

        if (err.length() > 0)
        {
            AZ_Warning("HOUDINI", printErrors == false, err.c_str());
            hasError = true;
        }

        err = GetLastHoudiniCookError();
        if (err.length() > 0)
        {
            //This error is spammy and more of a warning really, so we will not print it:
            QString error = err.c_str();
            if (error.contains("No geometry generated!", Qt::CaseInsensitive) == false)
            {
                AZ_Warning("HOUDINI-COOKING", printErrors == false, err.c_str());                
            }

            if (includeCookingErrors)
            {
                hasError = true;
            }
        }

        return hasError;
    }

    AZStd::string Houdini::FindHda(const AZStd::string& hdaFile)
    {
        AZ_PROFILE_FUNCTION(Editor);

        if (IsActive())
        {
            for (auto searchPath : m_searchPaths)
            {
                AZStd::string file = searchPath + "/" + hdaFile;

                char filePath[AZ_MAX_PATH_LEN] = { 0 };
                AZ::IO::FileIOBase::GetInstance()->ResolvePath(file.c_str(), filePath, AZ_MAX_PATH_LEN);
                file = AZStd::string(filePath);

                if (AZ::IO::FileIOBase::GetInstance()->Exists(file.c_str()))
                {
                    return file;
                }
            }
        }
        return hdaFile;
    }

    void Houdini::LoadAllAssets()
    {
        AZ_PROFILE_FUNCTION(Editor);

        if (IsActive() == false)
        {
            return;
        }

        for (auto searchPath : m_searchPaths)
        {
            //AZStd::string folder = searchPath + "/";
            AZStd::string folder = searchPath;

            char filePath[AZ_MAX_PATH_LEN] = { 0 };
            AZ::IO::FileIOBase::GetInstance()->ResolvePath(folder.c_str(), filePath, AZ_MAX_PATH_LEN);
            folder = AZStd::string(filePath);

            if (AZ::IO::FileIOBase::GetInstance()->Exists(folder.c_str()))
            {
                AZ::IO::LocalFileIO fileIo;
                AZStd::string errorString;

                // Handles each file and directory found
                // Safe to capture all by reference because the find will run sync
                AZ::IO::LocalFileIO::FindFilesCallbackType fileFinderCb;
                fileFinderCb = [&](const char* fullPath) -> bool
                {
                    if (fileIo.IsDirectory(fullPath) == false)
                    {
                        LoadHoudiniDigitalAsset(fullPath);
                    }

                    return true; // keep searching
                };

                // Scans subdirectories
                fileIo.FindFiles(folder.c_str(), "*.hda", fileFinderCb);
            }
        }
    }

    void Houdini::ReloadAllAssets()
    {
        m_assetCache.clear();

        LoadAllAssets();
    }

    HAPI_NodeId Houdini::FindNode(HAPI_NodeType networkType, const AZStd::string& path)
    {
        AZ_PROFILE_FUNCTION(Editor);

        int childCount = 0;
        HAPI_NodeId rootId;

        *this << "FindNode: " << networkType << " " << path << "";

        HAPI_GetManagerNodeId(&m_session, networkType, &rootId);
        HAPI_ComposeChildNodeList(&m_session, rootId, networkType, HAPI_NODEFLAGS_ANY, true, &childCount);

        AZStd::vector<HAPI_NodeId> finalChildList(childCount);
        if (childCount > 0) 
        {
            *this << "HAPI_GetComposedChildNodeList: " << rootId << " Reading: "<< childCount << " into buffer sized: " << finalChildList.size() << "";
            HAPI_GetComposedChildNodeList(&m_session, rootId, &finalChildList.front(), childCount);

            //TODO: Need Houdini 17 for string batch:
            /*
            AZStd::vector<HAPI_StringHandle> nameHandles;
            for (auto child : finalChildList)
            {
                HAPI_NodeInfo childInfo;
                HAPI_GetNodeInfo(&m_session, child, &childInfo);
                nameHandles.push_back(childInfo.internalNodePathSH);
            }

            int size = 0;            
            HAPI_GetStringBatchSize(&m_session, nameHandles.data(), nameHandles.size(), &size);

            if ( size > 0 )
            {
                char* allNames = new char[size];
                HAPI_GetStringBatch(&m_session, allNames, size);

                delete[] allNames;
            }
            */

            for (auto child : finalChildList)
            {
                HAPI_NodeInfo childInfo;
                HAPI_GetNodeInfo(&m_session, child, &childInfo);

                AZStd::string childPath = GetString(childInfo.internalNodePathSH);
                if (childPath == path)
                {
                    return childInfo.id;
                }
            }
        }

        return HOUDINI_INVALID_ID;
    }

    void Houdini::RemoveNode(const AZStd::string& oldNodeName, IHoudiniNode* node)
    {
        //Check the cache to see if we already created this node.
        auto cacheOutput = m_nodeNameCache.find(oldNodeName);
        auto cacheOutputPtr = m_nodeCache.find(node);

        if (cacheOutput != m_nodeNameCache.end())
        {
            m_nodeNameCache.erase(cacheOutput);            
        }
        
        if (cacheOutputPtr != m_nodeCache.end())
        {
            m_nodeCache.erase(cacheOutputPtr);
        }
    }


    void Houdini::RenameNode(const AZStd::string& oldNodeName, IHoudiniNode* node)
    {
        //Check the cache to see if we already created this node.
        auto cacheOutput = m_nodeNameCache.find(oldNodeName);
        auto cacheOutputPtr = m_nodeCache.find(node);

        if (cacheOutput != m_nodeNameCache.end() && cacheOutputPtr != m_nodeCache.end())
        {
            m_nodeNameCache.erase(cacheOutput);
            m_nodeNameCache[node->GetNodeName()] = cacheOutputPtr->second;
        }
        else
        {
            AZ_Warning("HOUDINI", false, "Could not rename node %s to %s", oldNodeName.c_str(), node->GetNodeName().c_str());
        }
    }


    HoudiniNodePtr Houdini::CreateNode(const AZStd::string& operatorName, const AZStd::string& nodeName, IHoudiniNode* parent)
    {
        AZ_PROFILE_FUNCTION(Editor);

        if (IsActive() == false)
        {
            return nullptr;
        }

        //Check the cache to see if we already created this node.
        auto cacheOutput = m_nodeNameCache.find(nodeName);
        if (cacheOutput != m_nodeNameCache.end())
        {
            return cacheOutput->second;
        }

        //Find the best casing incase someone wrote it differently:
        AZStd::string safeOperatorName = operatorName;

        for (auto& asset : m_assetCache)
        {
            for (auto& opName : asset.second->getAssets())
            {
                if (AzFramework::StringFunc::Equal(opName.c_str(), operatorName.c_str(), false))
                {
                    safeOperatorName = opName;
                }
            }
        }

        HAPI_NodeId nodeId = -1;
        HAPI_NodeId parentId = -1;

        if (parent != nullptr || parent == m_rootNode.get())
        {
            parentId = parent->GetId();
        }
            
        //Lock the status and update it:
        {
            AZStd::unique_lock<AZStd::mutex> theLock(m_statusLock); 
            m_currentStatus = "Creating HDA: " + nodeName + " as type: " + safeOperatorName;
            m_currentPercent = 0;
        }

        HAPI_Result result = HAPI_CreateNode(&m_session, parentId, safeOperatorName.c_str(), nullptr, false/*nodeName.c_str(), true*/, &nodeId);

        //If the system was activated late, we'll need to reload assets:
        QString error = GetLastHoudiniError().c_str();
        if (result == HAPI_RESULT_INVALID_ARGUMENT && error.contains("HAPI_LoadAssetLibraryFromFile"))
        {
            LoadAllAssets();
            result = HAPI_CreateNode(&m_session, parentId, safeOperatorName.c_str(), nodeName.c_str(), true, &nodeId);
            error = GetLastHoudiniError().c_str();
            if (result == HAPI_RESULT_INVALID_ARGUMENT && error.contains("HAPI_LoadAssetLibraryFromFile"))
            {
                return nullptr;
            }
        }

        //Check if there was an error creating the node:
        //An error about geometry missing is normal. This happens when a node is created but not configured yet
        if (result != HAPI_RESULT_SUCCESS && result != HAPI_RESULT_CANT_LOAD_GEO)
        {
            *this << " HAPI_CreateNode: FAILED TO CREATE NODE! parent:" << parentId << " operator: " << safeOperatorName << " name:" << nodeName << " nodeId: " << nodeId << " RESULT:" << result;
            return nullptr;
        }

        *this << " HAPI_CreateNode: parent:" << parentId << " operator: " << safeOperatorName << " name:" << nodeName << " nodeId: " << nodeId << "";
        CheckForErrors(result != HAPI_RESULT_CANT_LOAD_GEO);

        HoudiniNodePtr node = HoudiniNodePtr(new HoudiniNode(this, nullptr, nodeId, safeOperatorName, nodeName));
        if (node->GetNodeInfo().isValid == false)
        {
            *this << " HAPI_CreateNode: FAILED TO CREATE NODE! parent:" << parentId << " operator: " << safeOperatorName << " name:" << nodeName << " nodeId: " << nodeId << "Operator not valid";
            return nullptr;
        }

        m_nodeCache[node.get()] = node;
        m_nodeNameCache[nodeName] = node;

        if (parent != nullptr && m_nodeCache[parent] != nullptr)
        {
            node->SetParent(m_nodeCache[parent]);
        }

        //Lock the status and update it:
        {
            AZStd::unique_lock<AZStd::mutex> theLock(m_statusLock);
            m_currentStatus = "Creating HDA: " + nodeName + " as type: " + safeOperatorName + " Finished";
            m_currentPercent = 100;
        }

        return node;
    }

    void Houdini::CookNode(HAPI_NodeId node, const AZStd::string& entityName)
    {
        AZ_PROFILE_FUNCTION(Editor);

        if (IsActive() == false)
        {
            return;
        }
        
        HAPI_NodeInfo info = HAPI_NodeInfo_Create();
        HAPI_GetNodeInfo(&m_session, node, &info);
        
        if (info.id == HOUDINI_INVALID_ID)
        {
            AZ_Error("HOUDINI", false, (entityName + " has invalid node id! Cannot Cook this node!").c_str());
            return;
        }

        //Save the name:
        {
            auto nodeName = GetString(info.nameSH);
            AZ_PROFILE_SCOPE(Editor, nodeName.c_str());

            *this << " HAPI_CookNode: node:" << node << " name:" << nodeName << " cooked " << info.totalCookCount << " times" << "";
            HAPI_CookNode(&m_session, node, &m_cookOptions);
            int cookStatus;
            HAPI_Result cookResult;

            bool done = false;
            do
            {
                //Embedded mode doesn't benefit from this since it cannot be displayed!  No point in slowing it down by asking:
                if (GetHoudiniMode() == HOUDINI_MODE_ASYNC)
                {
                    int statusBufSize = 0;
                    HAPI_GetStatusStringBufLength(&m_session, HAPI_STATUS_COOK_STATE, HAPI_STATUSVERBOSITY_ERRORS, &statusBufSize);

                    if (statusBufSize > 0)
                    {
                        AZStd::unique_lock<AZStd::mutex> theLock(m_statusLock);
                        m_currentStatus.resize(statusBufSize);
                        HAPI_GetStatusString(&m_session, HAPI_STATUS_COOK_STATE, m_currentStatus.data(), statusBufSize);

                        m_currentStatus = entityName + " [" + nodeName + "]=>" + m_currentStatus;

                        int cur = 0;
                        int tot = 1;
                        HAPI_GetCookingCurrentCount(&m_session, &cur);
                        HAPI_GetCookingTotalCount(&m_session, &tot);
                        m_currentPercent = (int)(((double)cur / (double)(tot == 0 ? 1 : tot)) * 100);
                    }
                }
                
                cookResult = HAPI_GetStatus(&m_session, HAPI_STATUS_COOK_STATE, &cookStatus);                
                done = !(cookStatus > HAPI_STATE_MAX_READY_STATE && cookResult == HAPI_RESULT_SUCCESS);

                if (cookResult == HAPI_RESULT_USER_INTERRUPTED)
                {
                    done = true;
                }

                AZStd::this_thread::yield();

            } while (!done);

            bool isReady = false;
            {
                AZStd::unique_lock<AZStd::mutex> theWorkLock(m_threadWorkingLock);
                isReady = m_work.size() == 0;
            }

            if (isReady)
            {
                SetProgress("Ready", 100);
            }

            *this << " HAPI_CookNode Done: node:" << node << " name:" << nodeName << " cooked " << info.totalCookCount << " times" << "";            
        }
    }

    void Houdini::SaveDebugFile()
    {
        AZ_PROFILE_FUNCTION(Editor);

        char desktopPath[1024];

#ifdef AZ_PLATFORM_WINDOWS
        SHGetFolderPathA(NULL, CSIDL_DESKTOPDIRECTORY | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, desktopPath);
#endif

        *this << " HAPI_SaveHIPFile to " << desktopPath << "\\DebugAsset.hip" << "";
        HAPI_SaveHIPFile(&m_session, (AZStd::string(desktopPath) + "\\DebugAsset.hip").c_str(), false);
    }

    void Houdini::OnEditorNotifyEvent(EEditorNotifyEvent event)
    {
        switch (event)
        {
        case eNotify_OnBeginGameMode:
        case eNotify_OnBeginLayerExport:
        case eNotify_OnBeginSceneSave:
        {
            JoinProcessorThread();
            break;
        }
        case eNotify_OnCloseScene:
        {
            CancelProcessorThread();
            break;
        }
        }
    }

    void Houdini::Test()
    {
        HoudiniAssetPtr asset = LoadHoudiniDigitalAsset("MetalRail.hda");

        auto err = GetLastHoudiniError();

        HoudiniNodePtr paintCanNode = CreateNode("Object/MetalRail", "MyMetalRail");
        HoudiniNodePtr paintCanNode2 = CreateNode("Object/MetalRail", "MyMetalRail_clone");
        HoudiniNodePtr curve = CreateNode("sop/curve", "NURBS");

        HAPI_NodeId curveNode = curve->GetId();

        int cookStatus;
        HAPI_Result cookResult;
        do
        {
            cookResult = HAPI_GetStatus(&m_session, HAPI_STATUS_COOK_STATE, &cookStatus);
        } while (cookStatus > HAPI_STATE_MAX_READY_STATE && cookResult == HAPI_RESULT_SUCCESS);


        HAPI_NodeInfo curveNodeInfo;
        (HAPI_GetNodeInfo(&m_session, curveNode, &curveNodeInfo));

        AZStd::vector<HAPI_ParmInfo> parmInfos(curveNodeInfo.parmCount);
        (HAPI_GetParameters(&m_session, curveNode, &parmInfos[0], 0, curveNodeInfo.parmCount));

        int coordsParmIndex = -1;
        int typeParmIndex = -1;

        for (int i = 0; i < curveNodeInfo.parmCount; i++)
        {
            AZStd::string parmName = GetString(parmInfos[i].nameSH);
            if (parmName == "coords")
            {
                coordsParmIndex = i;
            }
            if (parmName == "type")
            {
                typeParmIndex = i;
            }
        }
        if (coordsParmIndex == -1 || typeParmIndex == -1)
        {
            //std::cout << "Failure at " << __FILE__ << ": " << __LINE__ << std::endl;
            //std::cout << "Could not find coords/type parameter on curve node" << std::endl;
        }
        HAPI_ParmInfo parm;
        HAPI_GetParameters(&m_session, curveNode, &parm, typeParmIndex, 1);
        int typeValue = 1;
        HAPI_SetParmIntValues(&m_session, curveNode, &typeValue, parm.intValuesIndex, 1);
        HAPI_GetParameters(&m_session, curveNode, &parm, coordsParmIndex, 1);
        HAPI_SetParmStringValue(&m_session, curveNode, "-4,0,4 -4,0,-4 4,0,-4 4,0,4", parm.id, 0);

        CheckForErrors();
    }

    HoudiniAssetPtr Houdini::LoadHoudiniDigitalAsset(const AZStd::string& hdaName)
    {
        AZ_PROFILE_FUNCTION(Editor);

        if (m_assetCache.find(hdaName) == m_assetCache.end())
        {
            HAPI_AssetLibraryId id;
            AZStd::string hdaFile = FindHda(hdaName);

            HAPI_LoadAssetLibraryFromFile(&m_session, hdaFile.c_str(), true, &id);
            auto err = GetLastHoudiniError();

            if (err.size() > 0)
            {
                AZ_Warning("HOUDINI", false, (AZStd::string("[HOUDINI] ") + err).c_str());
            }
            else
            {
                HoudiniAssetPtr asset = HoudiniAssetPtr(new HoudiniAsset(this, id, hdaFile.c_str(), hdaName.c_str()));
                m_assetCache[hdaName] = asset;
                return asset;
            }

            return nullptr;
        }
        else
        {
            return m_assetCache[hdaName];
        }
    }

    AZStd::vector<AZStd::string> Houdini::GetAvailableAssetNames()
    {
        AZStd::vector<AZStd::string> output;

        for (auto asset : m_assetCache)
        {
            output.push_back(asset.second->GetHdaName());
        }
        return output;
    }

    AZStd::vector<HoudiniAssetPtr> Houdini::GetAvailableAssets()
    {
        AZStd::vector<HoudiniAssetPtr> output;

        for (auto asset : m_assetCache)
        {
            output.push_back(asset.second);
        }
        return output;
    }

    HoudiniAssetPtr Houdini::GetAsset(const AZStd::string& assetName)
    {
        if (m_assetCache.find(assetName) != m_assetCache.end())
        {
            return m_assetCache[assetName];
        }

        return nullptr;
    }

    /* Houdini Util functions */
    AZStd::string Houdini::GetString(HAPI_StringHandle string_handle)
    {
        AZ_PROFILE_FUNCTION(Editor);

        // A string handle of 0 means an invalid string handle -- similar to
        // a null pointer.  Since we can't return NULL, though, return an empty
        // string.
        if (string_handle == 0)
        {
            return "";
        }

        int buffer_length = 0;
        HAPI_GetStringBufLength(&m_session, string_handle, &buffer_length);

        if (buffer_length <= 0)
        {
            return "";
        }

        char * buf = new char[buffer_length];
        HAPI_GetString(&m_session, string_handle, buf, buffer_length);
        AZStd::string result(buf);

        delete[] buf;
        return result;
    }

    AZStd::string Houdini::GetLastHoudiniError()
    {
        AZ_PROFILE_FUNCTION(Editor);

        int bufferLength = 0;
        HAPI_GetStatusStringBufLength(&m_session, HAPI_STATUS_CALL_RESULT, HAPI_STATUSVERBOSITY_ERRORS, &bufferLength);

        if (bufferLength <= 1)
        {
            return "";
        }

        char * buffer = new char[bufferLength];
        HAPI_GetStatusString(&m_session, HAPI_STATUS_CALL_RESULT, buffer, bufferLength);

        AZStd::string result(buffer);

        delete[] buffer;
        return result;
    }

    AZStd::string Houdini::GetLastHoudiniCookError()
    {
        AZ_PROFILE_FUNCTION(Editor);

        int bufferLength = 0;
        HAPI_GetStatusStringBufLength(&m_session, HAPI_STATUS_COOK_RESULT, HAPI_STATUSVERBOSITY_ERRORS, &bufferLength);

        if (bufferLength <= 1)
        {
            return "";
        }

        char * buffer = new char[bufferLength];
        HAPI_GetStatusString(&m_session, HAPI_STATUS_COOK_RESULT, buffer, bufferLength);
        AZStd::string result(buffer);

        delete[] buffer;
        
        if (result == "Cook succeeded.")
        {
            result = "";
        }
        
        return result;
    }

    //Specialization has to be in the CPP to prevent multi-instantiation
    template<>
    IHoudini& operator<< (IHoudini& os, const char* dt)
    {
        AZ_PROFILE_FUNCTION(Editor);
        os.Log(AZStd::string(dt));
        return os;
    }

    template<>
    IHoudini& operator<< (IHoudini& os, char* dt)
    {
        AZ_PROFILE_FUNCTION(Editor);
        os.Log(AZStd::string(dt));
        return os;
    }
    

    template<>
    IHoudini& operator<< (IHoudini& os, AZStd::string dt)
    {
        AZ_PROFILE_FUNCTION(Editor);
        os.Log(dt);
        return os;
    }

    template<>
    IHoudini& operator<< (IHoudini& os, const AZStd::string& dt)
    {
        AZ_PROFILE_FUNCTION(Editor);
        os.Log(dt);
        return os;
    }


}
