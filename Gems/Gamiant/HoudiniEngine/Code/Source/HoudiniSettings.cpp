/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "HoudiniSettings.h"
#include <HoudiniEngine/HoudiniGlobals.h>

#include <Viewport/ViewportSettings.h>

#include <AzCore/Settings/SettingsRegistry.h>
#include <AzToolsFramework/API/SettingsRegistryUtils.h>

namespace HoudiniEngine
{
    void SessionSettings::SetServerHost(const AZStd::string& host)
    {
        AzToolsFramework::SetRegistry(ServerHost, host);
    }

    AZStd::string SessionSettings::GetServerHost() const
    {
        return AzToolsFramework::GetRegistry<AZStd::string>(ServerHost, Globals::SessionServer);
    }

    void SessionSettings::SetNamedPipe(const AZStd::string& pipe)
    {
        AzToolsFramework::SetRegistry(NamedPipe, pipe);
    }

    AZStd::string SessionSettings::GetNamedPipe() const
    {
        return AzToolsFramework::GetRegistry<AZStd::string>(NamedPipe, Globals::SessionNamedPipe);
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
        return aznumeric_cast<AZ::u32>(AzToolsFramework::GetRegistry<AZ::u64>(ServerPort, Globals::SessionPort));
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
        return static_cast<ESessionType>(AzToolsFramework::GetRegistry<AZ::u64>(SessionType, 0));
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
        return AzToolsFramework::GetRegistry<AZStd::string>(OTLPath, "@projectroot@/assets/hda");
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

    void SessionSettings::SetSyncO3DEViewport(bool sync)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(SyncO3DEViewport, sync);
        }
    }

    bool SessionSettings::GetSyncO3DEViewport() const
    {
        return AzToolsFramework::GetRegistry<bool>(SyncO3DEViewport, true);
    }

    void SessionSettings::SetSyncHoudiniViewport(bool sync)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(SyncHoudiniViewport, sync);
        }
    }

    bool SessionSettings::GetSyncHoudiniViewport() const
    {
        return AzToolsFramework::GetRegistry<bool>(SyncHoudiniViewport, true);
    }

    // SessionSettings ////////////////////////////////////////////////////////////////

    void SessionSettings::SetKeepWorldTransform(bool keepWorldTransform)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(KeepWorldTransform, keepWorldTransform);
        }
    }

    bool SessionSettings::GetKeepWorldTransform() const
    {
        return AzToolsFramework::GetRegistry<bool>(KeepWorldTransform, true);
    }

    void SessionSettings::SetPackGeometryBeforeMerge(bool pack)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(PackGeometryBeforeMerge, pack);
        }
    }

    bool SessionSettings::GetPackGeometryBeforeMerge() const
    {
        return AzToolsFramework::GetRegistry<bool>(PackGeometryBeforeMerge, false);
    }

    void SessionSettings::SetExportInputAsReferences(bool exportValue)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(ExportInputAsReferences, exportValue);
        }
    }

    bool SessionSettings::GetExportInputAsReferences() const
    {
        return AzToolsFramework::GetRegistry<bool>(ExportInputAsReferences, false);
    }

    void SessionSettings::SetExportLODs(bool exportValue)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(ExportLODs, exportValue);
        }
    }

    bool SessionSettings::GetExportLODs() const
    {
        return AzToolsFramework::GetRegistry<bool>(ExportLODs, false);
    }

    void SessionSettings::SetExportSockets(bool exportValue)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(ExportSockets, exportValue);
        }
    }

    bool SessionSettings::GetExportSockets() const
    {
        return AzToolsFramework::GetRegistry<bool>(ExportSockets, false);
    }

    void SessionSettings::SetExportColliders(bool exportValue)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(ExportColliders, exportValue);
        }
    }

    bool SessionSettings::GetExportColliders() const
    {
        return AzToolsFramework::GetRegistry<bool>(ExportColliders, false);
    }

    void SessionSettings::SetExportMaterialParams(bool exportValue)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(ExportMaterialParams, exportValue);
        }
    }

    bool SessionSettings::GetExportMaterialParams() const
    {
        return AzToolsFramework::GetRegistry<bool>(ExportMaterialParams, false);
    }

    void SessionSettings::SetMergeSplineMeshComponents(bool merge)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(MergeSplineMeshComponents, merge);
        }
    }

    bool SessionSettings::GetMergeSplineMeshComponents() const
    {
        return AzToolsFramework::GetRegistry<bool>(MergeSplineMeshComponents, true);
    }

    void SessionSettings::SetExportLevelInstanceContent(bool exportValue)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(ExportLevelInstanceContent, exportValue);
        }
    }

    bool SessionSettings::GetExportLevelInstanceContent() const
    {
        return AzToolsFramework::GetRegistry<bool>(ExportLevelInstanceContent, true);
    }

    void SessionSettings::SetDirectlyConnectHDAsInHoudini(bool directlyConnect)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(DirectlyConnectHDAsAsInHoudini, directlyConnect);
        }
    }

    bool SessionSettings::GetDirectlyConnectHDAsInHoudini() const
    {
        return AzToolsFramework::GetRegistry<bool>(DirectlyConnectHDAsAsInHoudini, true);
    }

    void SessionSettings::SetUseLegacyInputCurves(bool useLegacyInputCurves)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(UseLegacyInputCurves, useLegacyInputCurves);
        }
    }

    bool SessionSettings::GetUseLegacyInputCurves() const
    {
        return AzToolsFramework::GetRegistry<bool>(UseLegacyInputCurves, true);
    }

    void SessionSettings::SetSplineResolution(float splineResolution)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(SplineResolution, splineResolution);
        }
    }

    float SessionSettings::GetSplineResolution() const
    {
        return AzToolsFramework::GetRegistry<double>(SplineResolution, 50.0);
    }

    void SessionSettings::SetSendNodePath(const AZStd::string& path)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(SendNodePath, path);
        }
    }

    AZStd::string SessionSettings::GetSendNodePath() const
    {
        return AzToolsFramework::GetRegistry<AZStd::string>(SendNodePath, "/obj/O3DEContent");
    }

    void SessionSettings::SetFetchPath(const AZStd::string& path)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(SendNodePath, path);
        }
    }

    AZStd::string SessionSettings::GetFetchPath() const
    {
        return AzToolsFramework::GetRegistry<AZStd::string>(SendNodePath, "/Assets/obj");
    }

    void SessionSettings::SetDefaultAssetName(const AZStd::string& path)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(DefaultAssetName, path);
        }
    }

    AZStd::string SessionSettings::GetDefaultAssetName() const
    {
        return AzToolsFramework::GetRegistry<AZStd::string>(SendNodePath, "MyAsset");
    }

    void SessionSettings::SetUseOutputNodes(bool useOutputNodes)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(UseOutputNodes, useOutputNodes);
        }
    }

    bool SessionSettings::GetUseOutputNodes() const
    {
        return AzToolsFramework::GetRegistry<bool>(UseOutputNodes, true);
    }

    void SessionSettings::SetFetchToWorld(bool fetchToWorld)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(FetchToWorld, fetchToWorld);
        }
    }

    bool SessionSettings::GetFetchToWorld() const
    {
        return AzToolsFramework::GetRegistry<bool>(FetchToWorld, false);
    }

    void SessionSettings::SetUpdatePeriod(float updatePeriod)
    {
        if (auto* registry = AZ::SettingsRegistry::Get())
        {
            registry->Set(UpdatePeriod, updatePeriod);
        }
    }

    float SessionSettings::GetUpdatePeriod() const
    {
        return AzToolsFramework::GetRegistry<double>(UpdatePeriod, 0.5);
    }

    const HAPI_CookOptions SessionSettings::GetDefaultCookOptions()
    {
        // Default CookOptions
        HAPI_CookOptions cookOptions;
        HAPI_CookOptions_Init(&cookOptions);

        cookOptions.curveRefineLOD = aznumeric_cast<float>(AzToolsFramework::GetRegistry<double>(CurveRefineLOD, 8.f));
        cookOptions.clearErrorsAndWarnings = AzToolsFramework::GetRegistry<bool>(ClearErrorsAndWarnings, false);
        cookOptions.maxVerticesPerPrimitive = AzToolsFramework::GetRegistry<AZ::u64>(MaxVerticesPerPrimitive, 3);
        cookOptions.splitGeosByGroup = AzToolsFramework::GetRegistry<bool>(SplitGeosByGroup, false);
        cookOptions.splitGeosByAttribute = AzToolsFramework::GetRegistry<bool>(SplitGeosByAttribute, false);
        cookOptions.splitAttrSH = AzToolsFramework::GetRegistry<AZ::u64>(SplitAttrSH, 0);
        cookOptions.refineCurveToLinear = AzToolsFramework::GetRegistry<bool>(RefineCurveToLinear, true);
        cookOptions.handleBoxPartTypes = AzToolsFramework::GetRegistry<bool>(HandleBoxPartTypes, false);
        cookOptions.handleSpherePartTypes = AzToolsFramework::GetRegistry<bool>(HandleSpherePartTypes, false);
        cookOptions.splitPointsByVertexAttributes = AzToolsFramework::GetRegistry<bool>(SplitPointsByVertexAttributes, false);
        cookOptions.packedPrimInstancingMode = static_cast<HAPI_PackedPrimInstancingMode>(AzToolsFramework::GetRegistry<AZ::u64>(PackedPrimInstancingMode, HAPI_PACKEDPRIM_INSTANCING_MODE_FLAT));
        cookOptions.cookTemplatedGeos = AzToolsFramework::GetRegistry<bool>(CookTemplatedGeos, false);

        return cookOptions;
    }
}
