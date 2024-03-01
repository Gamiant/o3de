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
#include <HoudiniEngine/HoudiniEngineBus.h>

namespace HoudiniEngine
{
    constexpr AZStd::string_view ServerHost = "/Houdini/Settings/Session/ServerHost";
    constexpr AZStd::string_view ServerPort = "/Houdini/Settings/Session/ServerPort";
    constexpr AZStd::string_view NamedPipe = "/Houdini/Settings/Session/NamedPipe";
    constexpr AZStd::string_view SessionType = "/Houdini/Settings/Session/SessionType";
    constexpr AZStd::string_view SyncHoudiniViewport = "/Houdini/Settings/Session/SyncHoudiniViewport";
    constexpr AZStd::string_view SyncO3DEViewport = "/Houdini/Settings/Session/SyncO3DEViewport";

    constexpr AZStd::string_view AutoClose = "/Houdini/Settings/Session/AutoClose";
    constexpr AZStd::string_view AutoCloseTimeOut = "/Houdini/Settings/Session/AutoCloseTimeOut";

    constexpr AZStd::string_view OTLPath = "/Houdini/Settings/Path/OTL";

    constexpr AZStd::string_view AutomaticServer = "/Houdini/Settings/Session/AutomaticServer";
    constexpr AZStd::string_view AutomaticServerTimeout = "/Houdini/Settings/Session/AutomaticServerTimeout";
    constexpr AZStd::string_view SyncWithHoudiniCook = "/Houdini/Settings/Session/SyncWithHoudiniCook";
    constexpr AZStd::string_view CookUsingHoudiniTime = "/Houdini/Settings/Session/CookUsingHoudiniTime";

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

    // Node Sync Settings
    constexpr AZStd::string_view KeepWorldTransform = "/Houdini/Settings/NodeSync/KeepWorldTransform";
    constexpr AZStd::string_view PackGeometryBeforeMerge = "/Houdini/Settings/NodeSync/PackGeometryBeforeMerge";
    constexpr AZStd::string_view ExportInputAsReferences = "/Houdini/Settings/NodeSync/ExportInputAsReferences";
    constexpr AZStd::string_view ExportLODs = "/Houdini/Settings/NodeSync/ExportLODs";
    constexpr AZStd::string_view ExportSockets = "/Houdini/Settings/NodeSync/ExportSockets";
    constexpr AZStd::string_view ExportColliders = "/Houdini/Settings/NodeSync/ExportColliders";
    constexpr AZStd::string_view ExportMaterialParams = "/Houdini/Settings/NodeSync/ExportMaterialParams";
    constexpr AZStd::string_view MergeSplineMeshComponents = "/Houdini/Settings/NodeSync/MergeSplineMeshComponents";
    constexpr AZStd::string_view ExportLevelInstanceContent = "/Houdini/Settings/NodeSync/ExportLevelInstanceContent";
    constexpr AZStd::string_view DirectlyConnectHDAsAsInHoudini = "/Houdini/Settings/NodeSync/DirectlyConnectHDAsAsInHoudini";
    constexpr AZStd::string_view UseLegacyInputCurves = "/Houdini/Settings/NodeSync/UseLegacyInputCurves";
    constexpr AZStd::string_view SplineResolution = "/Houdini/Settings/NodeSync/SplineResolution";
    constexpr AZStd::string_view SendNodePath = "/Houdini/Settings/NodeSync/SendNodePath";

    class SessionSettings
    {
    public:

        enum class ESessionType : AZ::u8
        {
            TCPSocket,
            NamedPipe
        };

        enum class EViewportSync : AZ::u8
        {
            Disabled,
            HoudiniToO3DE,
            O3DEToHoudini,
            Both
        };

        SessionSettings() = default;

        void SetServerHost(const AZStd::string& host);
        AZStd::string GetServerHost() const;

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

        void SetSyncO3DEViewport(bool sync);
        bool GetSyncO3DEViewport() const;

        void SetSyncHoudiniViewport(bool sync);
        bool GetSyncHoudiniViewport() const;


        // Node Sync Settings
        void SetKeepWorldTransform(bool keepWorldTransform);
        bool GetKeepWorldTransform() const;

        void SetPackGeometryBeforeMerge(bool pack);
        bool GetPackGeometryBeforeMerge() const;

        void SetExportInputAsReferences(bool exportValue);
        bool GetExportInputAsReferences() const;

        void SetExportLODs(bool exportValue);
        bool GetExportLODs() const;

        void SetExportSockets(bool exportValue);
        bool GetExportSockets() const;

        void SetExportColliders(bool exportValue);
        bool GetExportColliders() const;

        void SetExportMaterialParams(bool exportValue);
        bool GetExportMaterialParams() const;

        void SetMergeSplineMeshComponents(bool merge);
        bool GetMergeSplineMeshComponents() const;

        void SetExportLevelInstanceContent(bool exportValue);
        bool GetExportLevelInstanceContent() const;

        void SetDirectlyConnectHDAsInHoudini(bool directlyConnect);
        bool GetDirectlyConnectHDAsInHoudini() const;

        void SetUseLegacyInputCurves(bool useLegacyInputCurves);
        bool GetUseLegacyInputCurves() const;

        void SetSplineResolution(float splineResolution);
        float GetSplineResolution() const;

        void SetSendNodePath(const AZStd::string& path);
        AZStd::string GetSendNodePath() const;

    };

    struct SettingsBusRequests : public AZ::EBusTraits
    {
        virtual SessionSettings* GetSessionSettings() = 0;
    };
    using SettingsBus = AZ::EBus<SettingsBusRequests>;

    struct SessionNotifications : public AZ::EBusTraits
    {
        virtual void OnSessionStatusChange(SessionRequests::ESessionStatus sessionStatus) { AZ_UNUSED(sessionStatus); }

        virtual void OnSendToHoudini() {}
        virtual void OnFetchFromHoudini() {}

        virtual void OnViewportSyncChange(SessionSettings::EViewportSync) {}
    };
    using SessionNotificationBus = AZ::EBus<SessionNotifications>;




}
