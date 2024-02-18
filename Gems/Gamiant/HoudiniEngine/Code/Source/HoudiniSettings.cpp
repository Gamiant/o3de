/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "HoudiniSettings.h"
#include <Viewport/ViewportSettings.h>

namespace HoudiniEngine
{
    SessionSettings::SessionSettings()
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            using AZ::SettingsRegistryMergeUtils::IsPathAncestorDescendantOrEqual;

            m_serverHostHandler = registry->RegisterNotifier(
                [this](const AZ::SettingsRegistryInterface::NotifyEventArgs& notifyEventArgs)
                {
                    if (IsPathAncestorDescendantOrEqual(ServerHost, notifyEventArgs.m_jsonKeyPath))
                    {
                        m_serverHost.Signal(GetServerHost());
                    }
                }
            );

            m_serverPortHandler = registry->RegisterNotifier(
                [this](const AZ::SettingsRegistryInterface::NotifyEventArgs& notifyEventArgs)
                {
                    if (IsPathAncestorDescendantOrEqual(ServerPort, notifyEventArgs.m_jsonKeyPath))
                    {
                        m_serverPort.Signal(GetServerPort());
                    }
                }
            );
        }
    }

    void SessionSettings::SetServerHost(const AZStd::string& host)
    {
        AzToolsFramework::SetRegistry(ServerHost, host);
    }

    AZStd::string SessionSettings::GetServerHost() const
    {
        return AzToolsFramework::GetRegistry<AZStd::string>(ServerHost, "localhost");
    }

    void SessionSettings::SetNamedPipe(const AZStd::string& pipe)
    {
        AzToolsFramework::SetRegistry(NamedPipe, pipe);
    }

    AZStd::string SessionSettings::GetNamedPipe() const
    {
        return AzToolsFramework::GetRegistry<AZStd::string>(NamedPipe, "o3de_houdini");
    }

    void SessionSettings::SetServerPort(AZ::u64 port)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(ServerPort, port);
        }
    }

    AZ::u32 SessionSettings::GetServerPort() const
    {
        AZ::u32 port = aznumeric_cast<AZ::u32>(AzToolsFramework::GetRegistry<AZ::u64>(ServerPort, 9090));
        return port;
    }

    void SessionSettings::SetSessionType(AZ::u64 sessionType)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(SessionType, sessionType);
        }
    }

    SessionSettings::ESessionType SessionSettings::GetSessionType() const
    {
        ESessionType sessionType;
        sessionType = static_cast<ESessionType>(AzToolsFramework::GetRegistry<AZ::u64>(SessionType, 0));
        return sessionType;
    }

    void SessionSettings::SetOTLPath(const AZStd::string& otlPath)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(OTLPath, otlPath);
        }
    }

    AZStd::string SessionSettings::GetOTLPath() const
    {
        return AzToolsFramework::GetRegistry<AZStd::string>(NamedPipe, "@projectroot@/assets/hda");
    }

    void SessionSettings::SetAutoClose(bool autoClose)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(AutoClose, autoClose);
        }
    }

    bool SessionSettings::GetAutoClose() const
    {
        return AzToolsFramework::GetRegistry<bool>(AutoClose, true);
    }

    void SessionSettings::SetAutoCloseTimeOut(float timeOut)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(AutoCloseTimeOut, timeOut);
        }
    }

    float SessionSettings::GetAutoCloseTimeOut() const
    {
        return AzToolsFramework::GetRegistry<double>(AutoCloseTimeOut, 30000.0);
    }

    void SessionSettings::SetCookUsingHoudiniTime(bool cookUsingHoudiniTime)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(CookUsingHoudiniTime, cookUsingHoudiniTime);
        }
    }

    bool SessionSettings::GetCookUsingHoudiniTime() const
    {
        return AzToolsFramework::GetRegistry<bool>(CookUsingHoudiniTime, true);
    }

    void SessionSettings::SetSyncViewports(bool sync)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(SyncViewport, sync);
        }
    }

    bool SessionSettings::GetSyncViewports() const
    {
        return AzToolsFramework::GetRegistry<bool>(SyncViewport, true);
    }


}
