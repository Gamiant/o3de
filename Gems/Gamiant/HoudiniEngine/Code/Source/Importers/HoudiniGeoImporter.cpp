/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "HoudiniGeoImporter.h"

#include <AzCore/IO/Path/Path.h>
#include <AzCore/IO/FileIO.h>

#include "HoudiniEngineUtils.h"

namespace HoudiniEngine
{
    bool HoudiniGeoImporter::OpenBGEOFile(const AZStd::string& BGEOFile, HAPI_NodeId& nodeId, bool)
    {
        AZ::IO::Path filePath = BGEOFile;
        if (filePath.empty())
        {
            return false;
        }

        if (!filePath.HasExtension())
        {
            filePath = filePath.ReplaceExtension("bgeo.sc");
        }

        AZStd::string fileName = filePath.Filename().String();
        AZStd::string fileExtension = filePath.Extension().String();

        if (fileExtension.compare("sc") == 0)
        {
            fileExtension = filePath.Extension().String() + "." + fileExtension;
        }

        if (!fileExtension.starts_with("bgeo"))
        {
            AZ_Error("Houdini", false, "GEO Importer: File %s is not a .bgeo or .bgeo.sc file", fileName.c_str());
            return false;
        }

        AZ::IO::FileIOBase* fileIO = AZ::IO::FileIOBase::GetInstance();
        AZ_Assert(fileIO, "No File IO implementation available");

        if (!fileIO->Exists(filePath.c_str()))
        {
            return false;
        }

        nodeId = -1;

        bool isActive = false;
        HoudiniEngineRequestBus::BroadcastResult(isActive, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);

        HAPI_Session* session = nullptr;
        SessionRequestBus::BroadcastResult(session, &SessionRequests::GetSessionPtr);

        if (!isActive || !session)
        {
            AZ_Error("Houdini", false, "No active Houdini session");
            return false;
        }

        HAPI_Result result = HAPI_CreateNode(session, -1, "SOP/file", "bgeo", true, &nodeId);
        if (result != HAPI_RESULT_SUCCESS)
        {
            AZ_Error("Houdini", false, HoudiniEngineUtils::GetHoudiniResultByCode(result).c_str());
            return false;
        }

        HAPI_LoadGeoFromFile(session, nodeId, filePath.c_str());

        return false;
    }

    bool HoudiniGeoImporter::CookFileNode(const HAPI_NodeId& nodeId)
    {

        SessionSettings* settings = nullptr;
        SettingsBus::BroadcastResult(settings, &SettingsBus::Events::GetSessionSettings);
        AZ_Assert(settings, "Settings cannot be null");

        HAPI_CookOptions cookOptions = settings->GetDefaultCookOptions();

        HAPI_Session* session = nullptr;
        SessionRequestBus::BroadcastResult(session, &SessionRequests::GetSessionPtr);
        if (!session)
        {
            AZ_Error("Houdini", false, "No active Houdini session");
            return false;
        }

        HAPI_Result result = HAPI_CookNode(session, nodeId, &cookOptions);
        if (result != HAPI_RESULT_SUCCESS)
        {
            AZ_Error("Houdini", false, HoudiniEngineUtils::GetHoudiniResultByCode(result).c_str());
            return false;
        }

        // Wait for the cook to finish
        int status = HAPI_STATE_MAX_READY_STATE + 1;
        while (status > HAPI_STATE_MAX_READY_STATE)
        {
            result = HAPI_GetStatus(session, HAPI_STATUS_COOK_STATE, &status);
            if (result != HAPI_RESULT_SUCCESS)
            {
                AZ_Error("Houdini", false, HoudiniEngineUtils::GetHoudiniResultByCode(result).c_str());
                return false;
            }

            AZ_Info("Houdini", "Still Cooking, current status: %s", HoudiniEngineUtils::GetStatusString(HAPI_STATUS_COOK_STATE, HAPI_STATUSVERBOSITY_ERRORS).c_str());

            if (status > HAPI_STATE_MAX_READY_STATE)
            {
                AZStd::this_thread::sleep_for(AZStd::chrono::milliseconds(500));
            }
        }

        if (status != HAPI_STATE_READY)
        {
            // There was some cook errors
            AZ_Warning("Houdini", false, "Finished Cooking with errors!");
            return false;
        }

        AZ_Info("Houdini", "Finished Cooking!");

        return false;
    }

    bool HoudiniGeoImporter::CloseBGEOFile(const HAPI_NodeId& /*InNodeId*/)
    {
        return false;
    }

    bool HoudiniGeoImporter::BuildAllOutputsForNode(const HAPI_NodeId& InNodeId, AZ::EntityId outer, AZStd::vector<HoudiniOutput*>& InOldOutputs, AZStd::vector<HoudiniOutput*>& OutNewOutputs, bool bInAddOutputsToRootSet, bool bInUseOutputNodes)
    {
        AZ_UNUSED(InNodeId);
        AZ_UNUSED(outer);
        AZ_UNUSED(InOldOutputs);
        AZ_UNUSED(OutNewOutputs);
        AZ_UNUSED(bInAddOutputsToRootSet);
        AZ_UNUSED(bInUseOutputNodes);
        //bool bOutputTemplateGeos = false;

        //// Gather output nodes for the HAC
        //AZStd::vector<int> OutputNodes;
        //HoudiniEngineUtils::GatherAllAssetOutputs(InNodeId, bInUseOutputNodes, bOutputTemplateGeos, OutputNodes);

        //AZStd::unordered_map<HAPI_NodeId, int> OutputNodeCookCount;
        //if (!FHoudiniOutputTranslator::BuildAllOutputs(InNodeId, InOuter, OutputNodes, OutputNodeCookCount, InOldOutputs, OutNewOutputs, bOutputTemplateGeos, bInUseOutputNodes))
        //{
        //    // Couldn't create the package
        //    HOUDINI_LOG_ERROR(TEXT("Houdini GEO Importer: Failed to process the File SOP's outputs!"));
        //    return false;
        //}

        //if (bInAddOutputsToRootSet)
        //{
        //    // Add the output objects to the RootSet to prevent them from being GCed
        //    for (auto& Out : OutNewOutputs)
        //        Out->AddToRoot();
        //}

        return true;
    }
}
