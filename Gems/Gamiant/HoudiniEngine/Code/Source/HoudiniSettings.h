/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/std/string/string.h>
#include <AzCore/EBus/Event.h>
#include <AzCore/Settings/SettingsRegistry.h>
#include <AzCore/Settings/SettingsRegistryMergeUtils.h>
#include <AzCore/EBus/EBus.h>

namespace HoudiniEngine
{
    enum class SessionType
    {
        None,
        Socket,
        NamedPipe
    };


    constexpr AZStd::string_view ServerHost = "/Houdini/Settings/Session/ServerHost";
    constexpr AZStd::string_view ServerPort = "/Houdini/Settings/Session/ServerPort";
    constexpr AZStd::string_view NamedPipe = "/Houdini/Settings/Session/NamedPipe";
    constexpr AZStd::string_view SessionType = "/Houdini/Settings/Session/SessionType";

    constexpr AZStd::string_view AutoClose = "/Houdini/Settings/Session/AutoClose";
    constexpr AZStd::string_view AutoCloseTimeOut = "/Houdini/Settings/Session/AutoCloseTimeOut";

    constexpr AZStd::string_view OTLPath = "/Houdini/Settings/Path/OTL";

    constexpr AZStd::string_view AutomaticServer = "/Houdini/Settings/Session/AutomaticServer";
    constexpr AZStd::string_view AutomaticServerTimeout = "/Houdini/Settings/Session/AutomaticServerTimeout";
    constexpr AZStd::string_view SyncWithHoudiniCook = "/Houdini/Settings/Session/SyncWithHoudiniCook";
    constexpr AZStd::string_view CookUsingHoudiniTime = "/Houdini/Settings/Session/CookUsingHoudiniTime";
    constexpr AZStd::string_view SyncViewport = "/Houdini/Settings/Session/SyncViewport";
    constexpr AZStd::string_view SyncHoudiniViewport = "/Houdini/Settings/Session/SyncHoudiniViewport";
    constexpr AZStd::string_view SyncO3DEViewport = "/Houdini/Settings/Session/SyncO3DEViewport";

    constexpr AZStd::string_view SearchPath = "/Houdini/Settings/Tools/SearchPath";
    constexpr AZStd::string_view UseCustomHoudiniLocation = "/Houdini/Settings/Location/UseCustomHoudiniLocation";
    constexpr AZStd::string_view CustomHoudiniLocation = "/Houdini/Settings/Location/CustomHoudiniLocation";
    constexpr AZStd::string_view HoudiniExecutable = "/Houdini/Settings/Location/HoudiniExecutable";
    constexpr AZStd::string_view CustomHoudiniHomeLocation = "/Houdini/Settings/Location/CustomHoudiniHomeLocation";

    // HAPI_Initialize
    constexpr AZStd::string_view CookingThreadStack = "/Houdini/Settings/Initialize/CookingThreadStack";
    constexpr AZStd::string_view HoudiniEnvironmentFiles = "/Houdini/Settings/Initialize/HoudiniEnvironmentFiles";
    constexpr AZStd::string_view OtlSearchPath = "/Houdini/Settings/Initialize/OtlSearchPath";
    constexpr AZStd::string_view DsoSearchPath = "/Houdini/Settings/Initialize/DsoSearchPath";
    constexpr AZStd::string_view ImageDsoSearchPath = "/Houdini/Settings/Initialize/ImageDsoSearchPath";
    constexpr AZStd::string_view AudioDsoSearchPath = "/Houdini/Settings/Initialize/AudioDsoSearchPath";

    using BoolPropertyEvent = AZ::Event<bool>;
    using StringPropertyEvent = AZ::Event<AZStd::string>;
    using FloatPropertyEvent = AZ::Event<double>;
    using U64PropertyEvent = AZ::Event<AZ::u64>;

    class SessionSettings
    {
    public:

        enum class ESessionType : AZ::u8
        {
            TCPSocket,
            NamedPipe
        };

        SessionSettings();

        void SetServerHost(const AZStd::string& host);
        AZStd::string GetServerHost() const;
        //void SetServerHostChangedEvent(StringPropertyEvent::Handler& handler);

        void SetServerPort(AZ::u64 port);
        AZ::u32 GetServerPort() const;

        void SetNamedPipe(const AZStd::string& pipe);
        AZStd::string GetNamedPipe() const;

        void SetSessionType(AZ::u64 sessionType);
        ESessionType GetSessionType() const;

        void SetOTLPath(const AZStd::string& otlPath);
        AZStd::string GetOTLPath() const;

        void SetAutoClose(bool autoClose);
        bool GetAutoClose() const;

        void SetAutoCloseTimeOut(float timeOut);
        float GetAutoCloseTimeOut() const;

        void SetCookUsingHoudiniTime(bool cookUsingHoudiniTime);
        bool GetCookUsingHoudiniTime() const;

        void SetSyncViewports(bool sync);
        bool GetSyncViewports() const;

    protected:

        StringPropertyEvent m_serverHost;
        AZ::SettingsRegistryInterface::NotifyEventHandler m_serverHostHandler;

        StringPropertyEvent m_namedPipe;
        AZ::SettingsRegistryInterface::NotifyEventHandler m_namedPipeHandler;

        U64PropertyEvent m_serverPort;
        AZ::SettingsRegistryInterface::NotifyEventHandler m_serverPortHandler;

        U64PropertyEvent m_sessionType;
        AZ::SettingsRegistryInterface::NotifyEventHandler m_sessionTypeHandler;

        StringPropertyEvent m_otlPath;
        AZ::SettingsRegistryInterface::NotifyEventHandler m_otlPathHandler;

        BoolPropertyEvent m_autoClose;
        AZ::SettingsRegistryInterface::NotifyEventHandler m_autoCloseHandler;

        FloatPropertyEvent m_autoCloseTimeOut;
        AZ::SettingsRegistryInterface::NotifyEventHandler m_autoCloseTimeOutHandler;

        BoolPropertyEvent m_cookUsingHoudiniTime;
        AZ::SettingsRegistryInterface::NotifyEventHandler m_cookUsingHoudiniTimeHandler;

        BoolPropertyEvent m_syncViewports;
        AZ::SettingsRegistryInterface::NotifyEventHandler m_syncViewportsHandler;

    };

    struct SettingsBusRequests : public AZ::EBusTraits
    {
        virtual SessionSettings* GetSessionSettings() = 0;
    };
    using SettingsBus = AZ::EBus<SettingsBusRequests>;

}
