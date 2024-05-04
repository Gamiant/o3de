/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <HAPI/HAPI_Common.h>

#include <HoudiniEngine/HoudiniGlobals.h>

#include <AzCore/base.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/IO/FileIO.h>

#include <AzToolsFramework/API/ToolsApplicationAPI.h>

namespace HoudiniEngine
{
    struct HoudiniEngineUtils
    {
    public:
        // Default cook options
        // NOTE: this is originally from HoudiniEngine class
        static HAPI_CookOptions GetDefaultCookOptions();

        // Cook the specified node id
        // if the cook options are null, the default one will be used
        // if bWaitForCompletion is true, this call will be blocking until the cook is finished
        static bool HapiCookNode(const HAPI_NodeId& InNodeId, HAPI_CookOptions* InCookOptions = nullptr, const bool& bWaitForCompletion = false);

        static bool HapiGetParameterDataAsString(const HAPI_NodeId& NodeId, const AZStd::string& ParmName, const AZStd::string& DefaultValue, AZStd::string& OutValue);
        static bool HapiGetAttributeDataAsStringFromInfo(const HAPI_NodeId& InGeoId, const HAPI_PartId& InPartId, const char* InAttribName, HAPI_AttributeInfo& InAttributeInfo, AZStd::vector<AZStd::string>& OutData, const AZ::s32& InStartIndex = 0, const AZ::s32& InCount = -1);
        static bool HapiGetAttributeDataAsFloat(const HAPI_NodeId& InGeoId, const HAPI_PartId& InPartId, const char* InAttribName, HAPI_AttributeInfo& OutAttributeInfo, AZStd::vector<float>& OutData, 
            AZ::s32 InTupleSize = 0, HAPI_AttributeOwner InOwner = HAPI_ATTROWNER_INVALID, const AZ::s32& InStartIndex = 0, const AZ::s32& InCount = -1);
        static bool HapiGetParameterDataAsInteger(const HAPI_NodeId& NodeId, const AZStd::string& ParmName, const AZ::s32& DefaultValue, AZ::s32& OutValue);

        // HAPI : Look for a parameter by name and returns its index. Returns -1 if not found.
        static HAPI_ParmId HapiFindParameterByName(const HAPI_NodeId& InNodeId, const AZStd::string& InParmName, HAPI_ParmInfo& OutFoundParmInfo);

        // HAPI : Look for a parameter by tag and returns its index. Returns -1 if not found.
        static HAPI_ParmId HapiFindParameterByTag(const HAPI_NodeId& InNodeId, const AZStd::string& InParmTag, HAPI_ParmInfo& OutFoundParmInfo);

        static AZStd::string GetLevelPath()
        {

            AZStd::string levelName;
            AzToolsFramework::EditorRequestBus::BroadcastResult(levelName, &AzToolsFramework::EditorRequestBus::Events::GetLevelName);

            return "levels/" + levelName;
        }

        static AZStd::string GetProjectRoot()
        {
            AZStd::string folder = AZStd::string("@projectroot@/");
            char filePath[AZ_MAX_PATH_LEN] = { 0 };
            AZ::IO::FileIOBase::GetInstance()->ResolvePath(folder.c_str(), filePath, AZ_MAX_PATH_LEN);
            folder = AZStd::string(filePath);
            return folder;
        }

        static AZStd::string GetOutputExportFolder()
        {
            const AZStd::string& levelName = GetLevelPath();
            AZStd::string folder = AZStd::string("@projectroot@/") + levelName + AZStd::string("/ExportedGeometry/");
            char filePath[AZ_MAX_PATH_LEN] = { 0 };
            AZ::IO::FileIOBase::GetInstance()->ResolvePath(folder.c_str(), filePath, AZ_MAX_PATH_LEN);
            folder = AZStd::string(filePath);
            return folder;
        }

        static AZStd::string GetOutputFolder()
        {
            const AZStd::string& levelName = GetLevelPath();
            AZStd::string folder = AZStd::string("@projectroot@/") + levelName + AZStd::string("/HoudiniCache/");
            char filePath[AZ_MAX_PATH_LEN] = { 0 };
            AZ::IO::FileIOBase::GetInstance()->ResolvePath(folder.c_str(), filePath, AZ_MAX_PATH_LEN);
            folder = AZStd::string(filePath);
            return folder;
        }

        static AZStd::string GetRelativeOutputFolder()
        {
            const AZStd::string& levelName = GetLevelPath();
            const AZStd::string assetPath = levelName + AZStd::string("/HoudiniCache/");
            return assetPath;
        }

        static const char* GetPathVarDelimiter();

        static AZStd::string GetEnvironmentVariable(const AZStd::string& variableName);
        static bool SetEnvironmentVariable(const AZStd::string& variableName, const AZStd::string& variableValue);

        static AZStd::string GetConnectionError();
        static AZStd::string GetLastError();

        static bool SanitizeHAPIName(AZStd::string& name);

        static void ConvertHAPITransform(const HAPI_Transform& hapiTransform, AZ::Transform& transform);
        static void ConvertHAPIViewport(const HAPI_Viewport& viewport, AZ::Transform& transform);
        static void ConvertO3DEViewport(const AZ::Transform& o3deTransform, HAPI_Viewport& viewport);

        static AZStd::string GetHoudiniResultByCode(int code);

        static bool HapiGetObjectInfos(HAPI_Session* session, const HAPI_NodeId& InNodeId, AZStd::vector<HAPI_ObjectInfo>& objectInfos, AZStd::vector<HAPI_Transform>& objectTransforms);

        static bool ContainsSOPNodes(HAPI_Session* session, const HAPI_NodeId& NodeId);

        static bool IsOBJNodeFullyVisible(HAPI_Session* session, const AZStd::unordered_set<HAPI_NodeId>& allObjectIds, const HAPI_NodeId& rootNodeId, const HAPI_NodeId& childNodeId);

        static bool GatherImmediateOutputGeoInfos(HAPI_Session* session, const HAPI_NodeId& nodeId, const bool useOutputNodes, const bool gatherTemplateNodes, AZStd::vector<HAPI_GeoInfo>& geoInfos, AZStd::unordered_set<HAPI_NodeId>& forceNodesCook);
        static bool CookNode(HAPI_Session* session, const HAPI_NodeId& InNodeId, HAPI_CookOptions* InCookOptions, const bool& bWaitForCompletion);
    };
}
