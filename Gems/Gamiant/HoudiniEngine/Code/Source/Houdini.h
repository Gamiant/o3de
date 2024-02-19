/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <HAPI/HAPI.h>
#include <HoudiniEngine/HoudiniApi.h>
#include <HoudiniEngine/HoudiniCommonForwards.h>
#include <HoudiniEngine/HoudiniEngineBus.h>

#include <IConsole.h>
#include <IEditor.h>

#include <AzCore/Debug/Profiler.h>

#include <HoudiniSettings.h>
#include <HoudiniViewport.h>

#define HOUDINI_INVALID_ID -1
#define HOUDINI_ROOT_NODE_ID -1

namespace AzFramework
{
    class ProcessWatcher;
}

namespace HoudiniEngine 
{
    class Houdini
        : public IHoudini
        , public IEditorNotifyListener
        , SessionRequestBus::Handler
        , AZ::SystemTickBus::Handler
        , AZ::TickBus::Handler
    {
    protected:


        SessionSettings::ESessionType m_sessionType = SessionSettings::ESessionType::TCPSocket;
        SessionSettings::EViewportSync m_viewportSync = SessionSettings::EViewportSync::Disabled;

        AZStd::string m_namedPipe;
        AZStd::string m_serverHost;
        AZ::u32 m_serverPort;
        AZStd::string m_houdiniPath;
        HAPI_Session m_session;
        AZStd::string m_HAPIlibPath; // Path to the HAPI dynamic library
        AZ::u32 m_licenseType;
        bool m_environmentSet = false;
        Viewport m_viewport;
        bool m_startingSession = false;
        ///


        bool m_isActive;
        bool m_initialized;

        AZStd::string smallBuffer;
        ICVar * m_globalStateVar = nullptr; //O3DECONVERT
        ICVar * cVar_THRIFT = nullptr;
        ICVar * cVar_T = nullptr;

        HAPI_CookOptions m_cookOptions;

        HAPI_ProcessId m_thriftServerProcId;
        HAPI_ThriftServerOptions m_thriftServerOptions;

        HoudiniNodePtr m_rootNode;
        InputNodeManagerPtr m_inputNodeManager;

        bool m_printHistory = false;
        AZStd::string m_historyLine;
        AZStd::queue<AZStd::string> m_history;

        AZStd::vector<AZStd::string> m_searchPaths;
        AZStd::map<AZStd::string, HoudiniAssetPtr > m_assetCache;
        AZStd::map<IHoudiniNode*, HoudiniNodePtr> m_nodeCache;
        AZStd::map<AZStd::string, HoudiniNodePtr> m_nodeNameCache;
        
        //Lookups
        AZStd::mutex m_lookupLock;
        AZStd::map<AZ::EntityId, HoudiniEntityContext> m_lookups;

        AZStd::string FindHda(const AZStd::string& hdaFile);

    public:

        Houdini();
        ~Houdini();

        const HAPI_Session& GetSession() override { return m_session; }

        void Test();

        int m_maxSliceCount = 500;
        AZ::EntityId m_timeSlice;

        AZStd::queue< AZStd::pair<AZ::EntityId, AZStd::function<bool()>>> m_work;
        AZStd::thread_desc m_workerThreadDesc;
        AZStd::vector<AZStd::thread*> m_workerThread;
        AZStd::vector<AZStd::thread_id> m_workerThreadIds;
        AZStd::mutex m_lock;
        AZStd::mutex m_logLock;
        AZStd::mutex m_threadWorkingLock;

        AZStd::mutex m_statusLock;
        int m_currentPercent = 0;
        AZStd::string m_currentStatus;
                
        void ExecuteCommand(AZ::EntityId currentId, AZStd::function<bool()> functionToCall) override;
        void RaiseCommandPriority(AZ::EntityId newId) override;
                
        void ThreadProcessor();
        void CancelProcessorThread() override;
        void CancelProcessorJob(AZ::EntityId) override;
        void JoinProcessorThread() override;

        int GetHoudiniMode() override;

        void GetProgress(AZStd::string& statusText, int & statusPercent, int & assetsInQueue) override;
        void SetProgress(const AZStd::string& statusText, int statusPercent) override;

        // AZ::TickBus::Handler...
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        ///
        
        //Lookups: (Updated per tick)
        void RemoveLookupId(const AZ::EntityId& id) override;
        void LookupId(const AZ::EntityId& id) override;
        bool LookupIsSelected(const AZ::EntityId& id) override;
        AZ::Entity* LookupFindEntity(const AZ::EntityId& id) override;
        AZStd::string LookupEntityName(const AZ::EntityId& id) override;
        AZ::SplinePtr LookupSpline(const AZ::EntityId& id) override;
        AZStd::vector<AZStd::string> LookupAttributeNames(const AZ::EntityId& id) override;
        LmbrCentral::SplineAttribute<float>* LookupAttributeData(const AZ::EntityId& id, const AZStd::string& attribName) override;
        AZ::Transform LookupTransform(const AZ::EntityId& id) override;

        HoudiniEntityContext LookupEntityData(const AZ::EntityId& id) override;

        bool CheckForErrors(bool printErrors = true, bool includeCookingErrors = true) override;
        void LoadAllAssets() override;
        void ReloadAllAssets() override;  // FL[FD-10790] Houdini Digital Asset List Hot Reload
        HoudiniAssetPtr LoadHoudiniDigitalAsset(const AZStd::string& hdaName) override;

        void RemoveNode(const AZStd::string& oldNodeName, IHoudiniNode* node) override;
        void RenameNode(const AZStd::string& nodeName, IHoudiniNode* node) override;
        HoudiniNodePtr CreateNode(const AZStd::string& operatorName, const AZStd::string& nodeName, IHoudiniNode* parent = nullptr ) override;
        void CookNode(HAPI_NodeId node, const AZStd::string& entityName) override;

        AZStd::string GetString(HAPI_StringHandle string_handle) override;
        AZStd::string GetLastHoudiniError() override;
        AZStd::string GetLastHoudiniCookError() override;
            
        AZStd::vector<AZStd::string> GetAvailableAssetNames() override;
        AZStd::vector<HoudiniAssetPtr> GetAvailableAssets() override;
        HoudiniAssetPtr GetAsset(const AZStd::string& assetName) override;

        void SaveDebugFile() override;

        void DeleteAllObjects();

        void Shutdown() 
        {
            AZ_PROFILE_FUNCTION(Editor);
            AZ::TickBus::ClearQueuedEvents();
                        
            if (m_session.type == HAPI_SESSION_INPROCESS) 
            {
                //This seems to sometimes cause cleanup to crash:
                //DeleteAllObjects();

                //Cleanup Houdini session:
                {
                    AZ_PROFILE_SCOPE(Editor, "Cleanup Houdini Session");
                    HAPI_Cleanup(&m_session);
                }

                //CLose Session:
                {
                    AZ_PROFILE_SCOPE(Editor, "Close Houdini Session");
                    HAPI_CloseSession(&m_session);
                }
            }
            else
            {
                HAPI_CloseSession(&m_session);
            }            
            
            //Lock the lookups:
            {
                AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);
                m_lookups.clear();
            }

            m_inputNodeManager->Reset();
            m_assetCache.clear();
            m_nodeCache.clear();
            m_nodeNameCache.clear();
            m_isActive = false;
            m_initialized = false;
        }

        bool StartSession(SessionSettings::ESessionType, const AZStd::string& namedPipe, const AZStd::string& serverHost, AZ::u32 serverPort);
        bool ConnectSession(SessionSettings::ESessionType, const AZStd::string& namedPipe, const AZStd::string& serverHost, AZ::u32 serverPort);


        void Log(const AZStd::string& msg)
        {            
            AZStd::unique_lock<AZStd::mutex> theLock(m_logLock);

            if (msg.empty() || msg[0] == '\n')
            {
                if (m_historyLine.empty() == false)
                {
                    m_history.push(m_historyLine);
                    AZ_Warning("HOULOG", m_printHistory == false, m_historyLine.c_str());

                    if (m_history.size() > 5000)
                    {
                        m_history.pop();
                    }
                }

                m_historyLine = "";
            }            
            else
            {
                m_historyLine += msg;
            }
        }

        /* Accessors*/

        HAPI_NodeId FindNode(HAPI_NodeType networkType, const AZStd::string& path) override;
        
        HoudiniNodePtr GetRootNode() override
        {
            return m_rootNode;
        }
        
        bool IsActive() override
        { 
            return m_isActive && m_globalStateVar->GetIVal() == 1;
        }

        InputNodeManagerPtr GetInputNodeManager() override
        {
            return m_inputNodeManager;
        }

        protected:

            AZStd::unique_ptr<AzFramework::ProcessWatcher> m_houdiniProcessWatcher;

            std::chrono::steady_clock::time_point m_startSyncTime;

            // SessionRequestBus...
            void OpenHoudini() override;
            void StartSession() override;
            void StopSession() override;
            void RestartSession() override;
            void SetViewportSync(int index) override;
            HAPI_Session* GetSessionPtr() override;
            ///

            // SystemTickBus...
            void OnSystemTick() override;
            ///

            // IEditorNotifyListener...
            void OnEditorNotifyEvent(EEditorNotifyEvent event) override;
            ///

            void ComputeSearchPaths();
            void SetupEnvironment();
            void SessionSync();

            bool InitializeHAPISession();
            void ConfigureSession();
    };


    //Houdini Utility functions
    //Taken from MeshCompiler:
    inline static Vec3 CrossProd(const Vec3& a, const Vec3& b)
    {
        Vec3 ret;
        ret.x = a.y * b.z - a.z * b.y;
        ret.y = a.z * b.x - a.x * b.z;
        ret.z = a.x * b.y - a.y * b.x;
        return ret;
    }

    //Taken from MeshCompiler:
    inline static void GetOtherBaseVec(const Vec3& s, Vec3& a, Vec3& b)
    {
        if (fabsf(s.z) > 0.5f)
        {
            a.x = s.z;
            a.y = s.y;
            a.z = -s.x;
        }
        else
        {
            a.x = s.y;
            a.y = -s.x;
            a.z = s.z;
        }

        b = CrossProd(s, a).normalize();
        a = CrossProd(b, s).normalize();
    }

    //Taken from MeshCompiler:
    //Check tangent space and ensure some useful values, fix always according to normal
    /*static void GetTangentsFromNormal(Vec3& normal, Vec3& tangent, Vec3& bitangent)
    {
        if (normal.GetLengthSquared() < 0.1f)
        {
            normal = Vec3(0, 0, 1);
        }
        else if (normal.GetLengthSquared() < 0.9f)
        {
            normal.Normalize();
        }

        //fix case where both are equal
        GetOtherBaseVec(normal, tangent, bitangent);
    }*/
}
