/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "HoudiniEngineUtils.h"
#include "HoudiniEngineString.h"
#include <HAPI/HAPI.h>
#include <HoudiniEngine/HoudiniApi.h>
#include <HoudiniEngine/HoudiniEngineBus.h>
#include <AzCore/StringFunc/StringFunc.h>

#if defined (AZ_PLATFORM_WINDOWS)
#include <processenv.h>
#endif
#include <HoudiniSettings.h>

namespace HoudiniEngine
{
    HAPI_CookOptions HoudiniEngineUtils::GetDefaultCookOptions()
    {
        // Default CookOptions
        HAPI_CookOptions CookOptions;
        HAPI_CookOptions_Init(&CookOptions);

        CookOptions.curveRefineLOD = 8.0f;
        CookOptions.clearErrorsAndWarnings = false;
        CookOptions.maxVerticesPerPrimitive = 3;
        CookOptions.splitGeosByGroup = false;
        CookOptions.splitGeosByAttribute = false;
        CookOptions.splitAttrSH = 0;
        CookOptions.refineCurveToLinear = true;
        CookOptions.handleBoxPartTypes = false;
        CookOptions.handleSpherePartTypes = false;
        CookOptions.splitPointsByVertexAttributes = false;
        CookOptions.packedPrimInstancingMode = HAPI_PACKEDPRIM_INSTANCING_MODE_FLAT;
        CookOptions.cookTemplatedGeos = true;

        return CookOptions;
    }

    bool HoudiniEngineUtils::HapiCookNode(const HAPI_NodeId& InNodeId, HAPI_CookOptions* InCookOptions, const bool& bWaitForCompletion)
    {
        // Check for an invalid node id
        if (InNodeId < 0)
            return false;

        HoudiniPtr houdini;
        HoudiniEngineRequestBus::BroadcastResult(houdini, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);

        if (houdini == nullptr)
            return false;

        // No Cook Options were specified, use the default one
        if (InCookOptions == nullptr)
        {
            // Use the default cook options
            HAPI_CookOptions CookOptions = GetDefaultCookOptions();
            HAPI_CookNode(&houdini->GetSession(), InNodeId, &CookOptions);
        }
        else
        {
            // Use the provided CookOptions
            HAPI_CookNode(&houdini->GetSession(), InNodeId, InCookOptions);
        }

        // If we don't need to wait for completion, return now
        if (!bWaitForCompletion)
            return true;

        // Wait for the cook to finish
        while (true)
        {
            // Get the current cook status
            int Status = HAPI_STATE_STARTING_COOK;
            HAPI_GetStatus(&houdini->GetSession(), HAPI_STATUS_COOK_STATE, &Status);

            if (Status == HAPI_STATE_READY)
            {
                // The cook has been successful.
                return true;
            }
            else if (Status == HAPI_STATE_READY_WITH_FATAL_ERRORS || Status == HAPI_STATE_READY_WITH_COOK_ERRORS)
            {
                // There was an error while cooking the node.
                //FString CookResultString = FHoudiniEngineUtils::GetCookResult();
                //HOUDINI_LOG_ERROR();
                return false;
            }

            // We want to yield a bit.
            AZStd::this_thread::sleep_for(AZStd::chrono::milliseconds(100));
        }
    }

    bool HoudiniEngineUtils::HapiGetParameterDataAsString(const HAPI_NodeId& NodeId, const AZStd::string& ParmName, const AZStd::string& DefaultValue, AZStd::string& OutValue)
    {
        HoudiniPtr houdini;
        HoudiniEngineRequestBus::BroadcastResult(houdini, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);

        OutValue = DefaultValue;

        if (houdini != nullptr)
        {
            // Try to find the parameter by name
            HAPI_ParmId ParmId = -1;
            HAPI_GetParmIdFromName(&houdini->GetSession(), NodeId, ParmName.c_str(), &ParmId);

            if (ParmId < 0)
                return false;

            // Get the param info...
            HAPI_ParmInfo FoundParamInfo;
            HAPI_ParmInfo_Init(&FoundParamInfo);
            HAPI_GetParmInfo(&houdini->GetSession(), NodeId, ParmId, &FoundParamInfo);

            // .. and value
            HAPI_StringHandle StringHandle;
            HAPI_GetParmStringValues(&houdini->GetSession(), NodeId, false, &StringHandle, FoundParamInfo.stringValuesIndex, 1);

            // Convert the string handle to FString
            return HoudiniEngineString::ToAZString(StringHandle, OutValue);
        }
        
        return false;
    }

    bool HoudiniEngineUtils::HapiGetAttributeDataAsStringFromInfo(const HAPI_NodeId& InGeoId, const HAPI_PartId& InPartId, const char* InAttribName, HAPI_AttributeInfo& InAttributeInfo, AZStd::vector<AZStd::string>& OutData, const AZ::s32& InStartIndex, const AZ::s32& InCount)
    {
        if (!InAttributeInfo.exists)
            return false;

        HoudiniPtr houdini;
        HoudiniEngineRequestBus::BroadcastResult(houdini, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);

        if (houdini != nullptr)
        {
            // Handle partial reading of attributes
            AZ::s32 Start = 0;
            if (InStartIndex > 0 && InStartIndex < InAttributeInfo.count)
                Start = InStartIndex;

            AZ::s32 Count = InAttributeInfo.count;
            if (InCount > 0)
            {
                if ((Start + InCount) <= InAttributeInfo.count)
                    Count = InCount;
                else
                    Count = InAttributeInfo.count - Start;
            }

            // Extract the StringHandles
            AZStd::vector<HAPI_StringHandle> StringHandles;
            StringHandles.resize_no_construct(Count * InAttributeInfo.tupleSize);
            for (AZ::s32 n = 0; n < StringHandles.size(); n++)
                StringHandles[n] = -1;

            HAPI_GetAttributeStringData(&houdini->GetSession(), InGeoId, InPartId, InAttribName, &InAttributeInfo, &StringHandles[0], Start, Count);

            // Set the output data size
            OutData.resize(StringHandles.size());

            // Convert the StringHandles to FString.
            // using a map to minimize the number of HAPI calls
            HoudiniEngineString::SHArrayToStringArray(StringHandles, OutData);

            return true;
        }

        return false;
    }

    bool HoudiniEngineUtils::HapiGetAttributeDataAsFloat(const HAPI_NodeId& InGeoId, const HAPI_PartId& InPartId, const char* InAttribName, HAPI_AttributeInfo& OutAttributeInfo, AZStd::vector<float>& OutData, AZ::s32 InTupleSize, HAPI_AttributeOwner InOwner, const AZ::s32& InStartIndex, const AZ::s32& InCount)
    {
        OutAttributeInfo.exists = false;

        // Reset container size.
        OutData.clear();
        
        HoudiniPtr houdini;
        HoudiniEngineRequestBus::BroadcastResult(houdini, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);

        if (houdini != nullptr)
        {
            AZ::s32 OriginalTupleSize = InTupleSize;

            HAPI_AttributeInfo AttributeInfo;
            HAPI_AttributeInfo_Init(&AttributeInfo);
            //HAPI_AttributeInfo AttributeInfo = HAPI_AttributeInfo_Create();
            if (InOwner == HAPI_ATTROWNER_INVALID)
            {
                for (AZ::s32 AttrIdx = 0; AttrIdx < HAPI_ATTROWNER_MAX; ++AttrIdx)
                {
                    HAPI_GetAttributeInfo(&houdini->GetSession(), InGeoId, InPartId, InAttribName, (HAPI_AttributeOwner)AttrIdx, &AttributeInfo);

                    if (AttributeInfo.exists)
                        break;
                }
            }
            else
            {
                HAPI_GetAttributeInfo(&houdini->GetSession(), InGeoId, InPartId, InAttribName, InOwner, &AttributeInfo);
            }

            if (!AttributeInfo.exists)
                return false;

            if (OriginalTupleSize > 0)
                AttributeInfo.tupleSize = OriginalTupleSize;

            // Store the retrieved attribute information.
            OutAttributeInfo = AttributeInfo;

            // Handle partial reading of attributes
            AZ::s32 Start = 0;
            if (InStartIndex > 0 && InStartIndex < AttributeInfo.count)
                Start = InStartIndex;

            AZ::s32 Count = AttributeInfo.count;
            if (InCount > 0)
            {
                if ((Start + InCount) <= AttributeInfo.count)
                    Count = InCount;
                else
                    Count = AttributeInfo.count - Start;
            }

            if (AttributeInfo.storage == HAPI_STORAGETYPE_FLOAT)
            {
                // Allocate sufficient buffer for data.
                OutData.resize(Count * AttributeInfo.tupleSize);

                // Fetch the values
                HAPI_GetAttributeFloatData(&houdini->GetSession(), InGeoId, InPartId, InAttribName, &AttributeInfo, -1, &OutData[0], Start, Count);

                return true;
            }
            else if (AttributeInfo.storage == HAPI_STORAGETYPE_INT)
            {
                // Expected Float, found an int, try to convert the attribute

                // Allocate sufficient buffer for data.
                AZStd::vector<AZ::s32> IntData;
                IntData.resize(Count * AttributeInfo.tupleSize);

                // Fetch the values
                if (HAPI_RESULT_SUCCESS == HAPI_GetAttributeIntData(&houdini->GetSession(), InGeoId, InPartId, InAttribName, &AttributeInfo, -1, &IntData[0], Start, Count))
                {
                    OutData.resize(IntData.size());
                    for (AZ::s32 Idx = 0; Idx < IntData.size(); Idx++)
                    {
                        OutData[Idx] = (float)IntData[Idx];
                    }

                    //HOUDINI_LOG_MESSAGE(TEXT("Attribute %s was expected to be a float attribute, its value had to be converted from integer."), *FString(InAttribName));
                    return true;
                }
            }
            else if (AttributeInfo.storage == HAPI_STORAGETYPE_STRING)
            {
                // Expected Float, found a string, try to convert the attribute
                AZStd::vector<AZStd::string> StringData;
                if (HoudiniEngineUtils::HapiGetAttributeDataAsStringFromInfo(
                    InGeoId, InPartId, InAttribName,
                    AttributeInfo, StringData,
                    Start, Count))
                {
                    bool bConversionError = false;
                    OutData.resize(StringData.size());
                    for (AZ::s32 Idx = 0; Idx < StringData.size(); Idx++)
                    {
                        if (!AZ::StringFunc::LooksLikeFloat(StringData[Idx].data(), &OutData[Idx]))
                            bConversionError = true;
                    }

                    if (!bConversionError)
                    {
                        //HOUDINI_LOG_MESSAGE(TEXT("Attribute %s was expected to be a float attribute, its value had to be converted from string."), *FString(InAttribName));
                        return true;
                    }
                }
            }
        }

        //HOUDINI_LOG_WARNING(TEXT("Found attribute %s, but it was expected to be a float attribute and is of an invalid type."), *FString(InAttribName));
        return false;
    }

    bool HoudiniEngineUtils::HapiGetParameterDataAsInteger(const HAPI_NodeId& NodeId, const AZStd::string& ParmName, const AZ::s32& DefaultValue, AZ::s32& OutValue)
    {
        HoudiniPtr houdini;
        HoudiniEngineRequestBus::BroadcastResult(houdini, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);

        OutValue = DefaultValue;

        if (houdini != nullptr)
        {
            // Try to find the parameter by its name
            HAPI_ParmId ParmId = -1;
            HAPI_GetParmIdFromName(&houdini->GetSession(), NodeId, ParmName.c_str(), &ParmId);

            if (ParmId < 0)
                return false;

            // Get the param info...
            HAPI_ParmInfo FoundParmInfo;
            HAPI_ParmInfo_Init(&FoundParmInfo);
            HAPI_GetParmInfo(&houdini->GetSession(), NodeId, ParmId, &FoundParmInfo);

            // .. and value
            AZ::s32 Value = DefaultValue;
            HAPI_GetParmIntValues(&houdini->GetSession(), NodeId, &Value, FoundParmInfo.intValuesIndex, 1);

            OutValue = Value;

            return true;
        }

        return false;
    }

    HAPI_ParmId HoudiniEngineUtils::HapiFindParameterByName(const HAPI_NodeId& InNodeId, const AZStd::string& InParmName, HAPI_ParmInfo& OutFoundParmInfo)
    {
        HoudiniPtr houdini;
        HoudiniEngineRequestBus::BroadcastResult(houdini, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);
        if (houdini == nullptr)
            return -1;

        // Try to find the parameter by its name
        HAPI_ParmId ParmId = -1;
        HAPI_GetParmIdFromName(&houdini->GetSession(), InNodeId, InParmName.c_str(), &ParmId);

        if (ParmId < 0)
            return -1;

        HAPI_ParmInfo_Init(&OutFoundParmInfo);
        HAPI_GetParmInfo(&houdini->GetSession(), InNodeId, ParmId, &OutFoundParmInfo);

        return ParmId;
    }

    HAPI_ParmId HoudiniEngineUtils::HapiFindParameterByTag(const HAPI_NodeId& InNodeId, const AZStd::string& InParmTag, HAPI_ParmInfo& OutFoundParmInfo)
    {
        HoudiniPtr houdini;
        HoudiniEngineRequestBus::BroadcastResult(houdini, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);
        if (houdini == nullptr)
            return -1;

        // Try to find the parameter by its tag
        HAPI_ParmId ParmId = -1;
        HAPI_GetParmWithTag(&houdini->GetSession(), InNodeId, InParmTag.c_str(), &ParmId);

        if (ParmId < 0)
            return -1;

        HAPI_ParmInfo_Init(&OutFoundParmInfo);
        HAPI_GetParmInfo(&houdini->GetSession(), InNodeId, ParmId, &OutFoundParmInfo);

        return ParmId;
    }

    const char* HoudiniEngineUtils::GetPathVarDelimiter()
    {
#if defined(_WIN32)
        return ";";
#else
        return ":";
#endif
    }

    AZStd::string HoudiniEngineUtils::GetEnvironmentVariable(const AZStd::string& variableName) {
#ifdef _WIN32
        DWORD bufferSize = GetEnvironmentVariableA(variableName.c_str(), nullptr, 0);

        if (bufferSize == 0)
        {
            // Failed to get the size
            return "";
        }

        AZStd::string buffer(bufferSize, '\0');
        DWORD result = GetEnvironmentVariableA(variableName.c_str(), &buffer[0], bufferSize);

        if (result == 0 || result > bufferSize)
        {
            // Failed to get the environment variable value
            return "";
        }

        buffer.resize(result - 1);  // Exclude the null terminator
        return buffer;

#else
        // Unix-based systems implementation
        char* value = getenv(variableName.c_str());

        if (value == nullptr)
        {
            // Environment variable not found
            return "";
        }

        return value;
#endif
    }

    bool HoudiniEngineUtils::SetEnvironmentVariable(const AZStd::string& variableName, const AZStd::string& variableValue)
    {
#if defined(_WIN32)
        return SetEnvironmentVariableA(variableName.c_str(), variableValue.c_str()) != 0;
#else
        return setenv(variableName.c_str(), variableValue.c_str(), 1) == 0;
#endif
    }

    AZStd::string HoudiniEngineUtils::GetConnectionError()
    {
        int errorLength = 0;
        HAPI_GetConnectionErrorLength(&errorLength);

        if (errorLength > 0)
        {
            char* error = new char[errorLength];
            HAPI_GetConnectionError(error, errorLength, true);

            return error;
        }

        return {};
    }

    AZStd::string HoudiniEngineUtils::GetLastError()
    {
        int buffer_length = 0;
        HAPI_GetStatusStringBufLength(nullptr, HAPI_STATUS_CALL_RESULT, HAPI_STATUSVERBOSITY_ALL, &buffer_length);
        if (buffer_length > 0)
        {
            char* buf = new char[buffer_length];
            HAPI_GetStatusString(nullptr, HAPI_STATUS_CALL_RESULT, buf, buffer_length);
            AZStd::string result(buf);
            delete[] buf;
            return result;
        }
        return {};
    }

    bool HoudiniEngineUtils::SanitizeHAPIName(AZStd::string& name)
    {
        bool changed = false;

        char startingChar = name.c_str()[0];
        if (isdigit(startingChar))
        {
            name.insert(0, '_');
            changed = true;
        }

        if (AZ::StringFunc::Replace(name, " ", "_"))
        {
            changed = true;
        }

        return changed;
    }

    void HoudiniEngineUtils::ConvertHAPITransform(const HAPI_Transform& hapiTransform, AZ::Transform& transform)
    {
        AZ::Quaternion rotation = AZ::Quaternion(
            hapiTransform.rotationQuaternion[0],
            hapiTransform.rotationQuaternion[2],
            hapiTransform.rotationQuaternion[1],
            -hapiTransform.rotationQuaternion[3]);

        AZ::Vector3 translation = AZ::Vector3(
            hapiTransform.position[0],
            hapiTransform.position[2],
            hapiTransform.position[1]
        );

        translation *= Globals::ScaleFactorTranslation;

        transform.SetRotation(rotation);
        transform.SetTranslation(translation);
        transform.SetUniformScale(*hapiTransform.scale);
    }

    void HoudiniEngineUtils::ConvertHAPIViewport(const HAPI_Viewport& viewport, AZ::Transform& transform)
    {
        AZ::Quaternion rotation = AZ::Quaternion(
            viewport.rotationQuaternion[0],
            viewport.rotationQuaternion[2],
            viewport.rotationQuaternion[1],
            -viewport.rotationQuaternion[3]);

        // Rotate to transform from right handed
        rotation = rotation * AZ::Quaternion::CreateFromEulerAnglesDegrees(AZ::Vector3(0.f, 0.f, -180.f));

        AZ::Vector3 translation = AZ::Vector3(
            viewport.position[0],
            viewport.position[2],
            -viewport.position[1]
        );

        translation *= (-viewport.offset * Globals::ScaleFactorTranslation);

        transform.SetRotation(rotation);
        transform.SetTranslation(translation);

    }

    void HoudiniEngineUtils::ConvertO3DEViewport(const AZ::Transform& o3deTransform, HAPI_Viewport& viewport)
    {
        AZ::Quaternion rotation = o3deTransform.GetRotation();
        rotation = rotation * AZ::Quaternion::CreateFromEulerAnglesDegrees(AZ::Vector3(0.f, 0.f, -180.f));

        viewport.rotationQuaternion[0] = rotation.GetX();
        viewport.rotationQuaternion[1] = rotation.GetZ();
        viewport.rotationQuaternion[2] = rotation.GetY();
        viewport.rotationQuaternion[3] = -rotation.GetW();

        const auto& cameraPosition = o3deTransform.GetTranslation();
        //cameraPosition *= (viewport.offset * Globals::ScaleFactorTranslation);

        viewport.position[0] = cameraPosition.GetX();
        viewport.position[1] = cameraPosition.GetZ();
        viewport.position[2] = -cameraPosition.GetY();


        // TODO-GMT: figure out the offset when we're using orbit 
        viewport.offset = 10.f; 
    }

    AZStd::string HoudiniEngineUtils::GetHoudiniResultByCode(int code)
    {
        static AZStd::map<HAPI_Result, AZStd::string> codeMapping = {
            { HAPI_RESULT_SUCCESS, "HAPI_RESULT_SUCCESS" },
            { HAPI_RESULT_FAILURE, "HAPI_RESULT_FAILURE" },
            { HAPI_RESULT_ALREADY_INITIALIZED, "HAPI_RESULT_ALREADY_INITIALIZED" },
            { HAPI_RESULT_NOT_INITIALIZED, "HAPI_RESULT_NOT_INITIALIZED" },
            { HAPI_RESULT_CANT_LOADFILE, "HAPI_RESULT_CANT_LOADFILE" },
            { HAPI_RESULT_PARM_SET_FAILED, "HAPI_RESULT_PARM_SET_FAILED" },
            { HAPI_RESULT_INVALID_ARGUMENT, "HAPI_RESULT_INVALID_ARGUMENT" },
            { HAPI_RESULT_CANT_LOAD_GEO, "HAPI_RESULT_CANT_LOAD_GEO" },
            { HAPI_RESULT_CANT_GENERATE_PRESET, "HAPI_RESULT_CANT_GENERATE_PRESET" },
            { HAPI_RESULT_CANT_LOAD_PRESET, "HAPI_RESULT_CANT_LOAD_PRESET" },
            { HAPI_RESULT_ASSET_DEF_ALREADY_LOADED, "HAPI_RESULT_ASSET_DEF_ALREADY_LOADED" },

            { HAPI_RESULT_NO_LICENSE_FOUND, "HAPI_RESULT_NO_LICENSE_FOUND" },
            { HAPI_RESULT_DISALLOWED_NC_LICENSE_FOUND, "HAPI_RESULT_DISALLOWED_NC_LICENSE_FOUND" },
            { HAPI_RESULT_DISALLOWED_NC_ASSET_WITH_C_LICENSE, "HAPI_RESULT_DISALLOWED_NC_ASSET_WITH_C_LICENSE" },
            { HAPI_RESULT_DISALLOWED_NC_ASSET_WITH_LC_LICENSE, "HAPI_RESULT_DISALLOWED_NC_ASSET_WITH_LC_LICENSE" },
            { HAPI_RESULT_DISALLOWED_LC_ASSET_WITH_C_LICENSE, "HAPI_RESULT_DISALLOWED_LC_ASSET_WITH_C_LICENSE" },
            { HAPI_RESULT_DISALLOWED_HENGINEINDIE_W_3PARTY_PLUGIN, "HAPI_RESULT_DISALLOWED_HENGINEINDIE_W_3PARTY_PLUGIN" },

            { HAPI_RESULT_ASSET_INVALID, "HAPI_RESULT_ASSET_INVALID" },
            { HAPI_RESULT_NODE_INVALID, "HAPI_RESULT_NODE_INVALID" },

            { HAPI_RESULT_USER_INTERRUPTED, "HAPI_RESULT_USER_INTERRUPTED" },

            { HAPI_RESULT_INVALID_SESSION, "HAPI_RESULT_INVALID_SESSION" }
        };

        auto it = codeMapping.find((HAPI_Result)code);
        if (it != codeMapping.end())
        {
            return it->second;
        }

        return {};
    }

    bool HoudiniEngineUtils::HapiGetObjectInfos(HAPI_Session* session, const HAPI_NodeId& nodeId, AZStd::vector<HAPI_ObjectInfo>& objectInfos, AZStd::vector<HAPI_Transform>& objectTransforms)
    {
        HAPI_NodeInfo nodeInfo;
        HAPI_NodeInfo_Init(&nodeInfo);
        HAPI_Result result = HAPI_GetNodeInfo(session, nodeId, &nodeInfo);
        if (result != HAPI_RESULT_SUCCESS)
        {
            AZ_Error("Houdini", false, HoudiniEngineUtils::GetHoudiniResultByCode(result).c_str());
            return false;
        }

        int objectCount = 0;
        if (nodeInfo.type == HAPI_NODETYPE_SOP)
        {
            objectCount = 1;
            objectInfos.reserve(1);
            HAPI_ObjectInfo_Init(&objectInfos[0]);

            result = HAPI_GetObjectInfo(session, nodeId, &objectInfos[0]);
            if (result != HAPI_RESULT_SUCCESS)
            {
                AZ_Error("Houdini", false, HoudiniEngineUtils::GetHoudiniResultByCode(result).c_str());
                return false;
            }

            // Use the identity transform
            objectTransforms.reserve(1);
            HAPI_Transform_Init(&objectTransforms[0]);

            objectTransforms[0].rotationQuaternion[3] = 1.0f;
            objectTransforms[0].scale[0] = 1.0f;
            objectTransforms[0].scale[1] = 1.0f;
            objectTransforms[0].scale[2] = 1.0f;
            objectTransforms[0].rstOrder = HAPI_SRT;
        }
        else if (nodeInfo.type == HAPI_NODETYPE_OBJ)
        {
            result = HAPI_ComposeObjectList(session, nodeId, nullptr, &objectCount);
            if (result != HAPI_RESULT_SUCCESS)
            {
                AZ_Error("Houdini", false, HoudiniEngineUtils::GetHoudiniResultByCode(result).c_str());
                return false;
            }

            if (objectCount <= 0)
            {
                // This asset is an OBJ that has no object as children, use the object itself
                objectCount = 1;
                objectInfos.reserve(1);
                HAPI_ObjectInfo_Init(&objectInfos[0]);

                result = HAPI_GetObjectInfo(session, nodeId, &objectInfos[0]);
                if (result != HAPI_RESULT_SUCCESS)
                {
                    AZ_Error("Houdini", false, HoudiniEngineUtils::GetHoudiniResultByCode(result).c_str());
                    return false;
                }

                // Use the identity transform
                objectTransforms.reserve(1);
                HAPI_Transform_Init(&objectTransforms[0]);

                objectTransforms[0].rotationQuaternion[3] = 1.0f;
                objectTransforms[0].scale[0] = 1.0f;
                objectTransforms[0].scale[1] = 1.0f;
                objectTransforms[0].scale[2] = 1.0f;
                objectTransforms[0].rstOrder = HAPI_SRT;
            }
            else
            {
                // This OBJ has children
                // See if we should add ourself by looking for immediate display SOP 
                int immediateSOP = 0;
                result = HAPI_ComposeChildNodeList(session, nodeInfo.id, HAPI_NODETYPE_SOP, HAPI_NODEFLAGS_DISPLAY, false, &immediateSOP);
                if (result != HAPI_RESULT_SUCCESS)
                {
                    AZ_Error("Houdini", false, HoudiniEngineUtils::GetHoudiniResultByCode(result).c_str());
                    return false;
                }

                result = HAPI_ComposeObjectList(session, nodeId, nullptr, &objectCount);
                if (result != HAPI_RESULT_SUCCESS)
                {
                    AZ_Error("Houdini", false, HoudiniEngineUtils::GetHoudiniResultByCode(result).c_str());
                    return false;
                }

                bool addSelf = immediateSOP > 0;
                int count = addSelf ? objectCount + 1 : objectCount;
                objectInfos.reserve(count);
                objectTransforms.reserve(count);

                for (int i = 0; i < objectInfos.size(); ++i)
                {
                    HAPI_ObjectInfo_Init(&objectInfos[i]);
                    HAPI_Transform_Init(&objectTransforms[i]);
                }

                // Get our object info in 0 if needed
                if (addSelf)
                {
                    result = HAPI_GetObjectInfo(session, nodeId, &objectInfos[0]);
                    if (result != HAPI_RESULT_SUCCESS)
                    {
                        AZ_Error("Houdini", false, HoudiniEngineUtils::GetHoudiniResultByCode(result).c_str());
                        return false;
                    }

                    // Use the identity transform
                    objectTransforms[0].rotationQuaternion[3] = 1.0f;
                    objectTransforms[0].scale[0] = 1.0f;
                    objectTransforms[0].scale[1] = 1.0f;
                    objectTransforms[0].scale[2] = 1.0f;
                    objectTransforms[0].rstOrder = HAPI_SRT;
                }

                // Get the other object infos
                result = HAPI_GetComposedObjectList(session, nodeId, &objectInfos[addSelf ? 1 : 0], 0, objectCount);
                if (result != HAPI_RESULT_SUCCESS)
                {
                    AZ_Error("Houdini", false, HoudiniEngineUtils::GetHoudiniResultByCode(result).c_str());
                    return false;
                }

                result = HAPI_GetComposedObjectTransforms(session, nodeId, HAPI_SRT, &objectTransforms[addSelf ? 1 : 0], 0, objectCount);
                if (result != HAPI_RESULT_SUCCESS)
                {
                    AZ_Error("Houdini", false, HoudiniEngineUtils::GetHoudiniResultByCode(result).c_str());
                    return false;
                }
            }
        }
        else
        {
            return false;
        }

        return true;
    }

    bool HoudiniEngineUtils::ContainsSOPNodes(HAPI_Session* session, const HAPI_NodeId& nodeId)
    {
        int childCount = 0;

        HOUDINI_CHECK_ERROR_RETURN(HAPI_ComposeChildNodeList(session, nodeId, HAPI_NODETYPE_SOP, HAPI_NODEFLAGS_ANY, false, &childCount), false);

        return childCount > 0;
    }

    bool HoudiniEngineUtils::IsOBJNodeFullyVisible(HAPI_Session* session, const AZStd::unordered_set<HAPI_NodeId>& allObjectIds, const HAPI_NodeId& rootNodeId, const HAPI_NodeId& childNodeId)
    {
        // Walk up the hierarchy from child to root.
        // If any node in that hierarchy is not in the "AllObjectIds" set, the OBJ node is considered to
        // be hidden.

        if (childNodeId == rootNodeId)
            return true;

        HAPI_NodeId currentChildNodeId = childNodeId;

        HAPI_ObjectInfo childObjInfo;
        HAPI_NodeInfo childNodeInfo;

        HAPI_ObjectInfo_Init(&childObjInfo);
        HAPI_NodeInfo_Init(&childNodeInfo);

        do
        {
            if (HAPI_RESULT_SUCCESS != HAPI_GetObjectInfo(session, childNodeId, &childObjInfo))
            {
                // If can't get info for this object, we can't say whether it's visible (or not).
                return false;
            }

            if (!childObjInfo.isVisible || childObjInfo.nodeId < 0)
            {
                // We have an object in the chain that is not visible. Return false immediately!
                return false;
            }

            if (childNodeId != childNodeId)
            {
                // Only perform this check for 'parents' of the incoming child node
                if (!allObjectIds.contains(currentChildNodeId))
                {
                    // There is a non-object node in the hierarchy between the child and asset root, rendering the
                    // child object node invisible.
                    return false;
                }
            }

            if (HAPI_RESULT_SUCCESS != HAPI_GetNodeInfo(session, currentChildNodeId, &childNodeInfo))
            {
                // Could not retrieve node info.
                return false;
            }

            // Go up the hierarchy.
            currentChildNodeId = childNodeInfo.parentId;

        } while (currentChildNodeId != rootNodeId && currentChildNodeId >= 0);

        // We have traversed the whole hierarchy up to the root and nothing indicated that
        // we _shouldn't_ be visible.
        return true;
    }

    bool HoudiniEngineUtils::GatherImmediateOutputGeoInfos(HAPI_Session* session, const HAPI_NodeId& nodeId, const bool useOutputNodes, const bool gatherTemplateNodes, AZStd::vector<HAPI_GeoInfo>& geoInfos, AZStd::unordered_set<HAPI_NodeId>& forceNodesCook)
    {
        AZStd::unordered_set<HAPI_NodeId> gatheredNodeIds;

        // NOTE: This function assumes that the incoming node is a Geometry container that contains immediate outputs / display nodes / template nodes.

        // First we look for (immediate) output nodes (if bUseOutputNodes have been enabled). If we didn't find an output node, we'll look for a display node.

        bool hasOutputs = false;
        if (useOutputNodes)
        {
            int numOutputs = -1;
            HAPI_GetOutputGeoCount(session, nodeId, &numOutputs);

            if (numOutputs > 0)
            {
                hasOutputs = true;

                // Extract GeoInfo from the immediate output nodes.
                AZStd::vector<HAPI_GeoInfo> outputGeoInfos;

                outputGeoInfos.reserve(numOutputs);

                if (HAPI_RESULT_SUCCESS == HAPI_GetOutputGeoInfos(session, nodeId, outputGeoInfos.data(), numOutputs))
                {
                    // Gather all the output nodes
                    for (HAPI_GeoInfo& OutputGeoInfo : outputGeoInfos)
                    {
                        // This geo should be output. Be sure to ignore any template flags. 
                        OutputGeoInfo.isTemplated = false;
                        geoInfos.push_back(OutputGeoInfo);
                        gatheredNodeIds.insert(OutputGeoInfo.nodeId);
                        forceNodesCook.insert(OutputGeoInfo.nodeId); // Ensure this output node gets cooked
                    }
                }
            }
        }

        if (!hasOutputs)
        {
            // If we didn't get any output data, pull our output data directly from the Display node.

            // Look for display nodes.
            int displayNodeCount = 0;
            if (HAPI_RESULT_SUCCESS == HAPI_ComposeChildNodeList(session, nodeId, HAPI_NODETYPE_SOP, HAPI_NODEFLAGS_DISPLAY, false, &displayNodeCount))
            {
                if (displayNodeCount > 0)
                {
                    // Retrieve all the display node ids
                    AZStd::vector<HAPI_NodeId> displayNodeIds;
                    displayNodeIds.reserve(displayNodeCount);
                    if (HAPI_RESULT_SUCCESS == HAPI_GetComposedChildNodeList(session, nodeId, displayNodeIds.data(), displayNodeCount))
                    {
                        HAPI_GeoInfo geoInfo;
                        HAPI_GeoInfo_Init(&geoInfo);
                        // Retrieve the Geo Infos for each display node
                        for (const HAPI_NodeId& displayNodeId : displayNodeIds)
                        {
                            if (gatheredNodeIds.contains(displayNodeId))
                                continue; // This node has already been gathered from this subnet.

                            if (HAPI_RESULT_SUCCESS == HAPI_GetGeoInfo(session, displayNodeId, &geoInfo))
                            {
                                // This geo should be output. Be sure to ignore any templated flags.
                                geoInfo.isTemplated = false;
                                geoInfos.push_back(geoInfo);
                                gatheredNodeIds.insert(displayNodeId);
                                // If this node doesn't have a part_id count, ensure it gets cooked.
                                forceNodesCook.insert(displayNodeId);
                            }
                        }
                    }
                }
            }
        }

        // Gather templated nodes.
        if (gatherTemplateNodes)
        {
            int numTemplateNodes = 0;

            // Gather all template nodes
            if (HAPI_RESULT_SUCCESS == HAPI_ComposeChildNodeList(session, nodeId,HAPI_NODETYPE_SOP, HAPI_NODEFLAGS_TEMPLATED, false, &numTemplateNodes))
            {
                AZStd::vector<HAPI_NodeId> templateNodeIds;

                templateNodeIds.reserve(numTemplateNodes);
                if (HAPI_RESULT_SUCCESS == HAPI_GetComposedChildNodeList(session, nodeId,templateNodeIds.data(), numTemplateNodes))
                {
                    for (const HAPI_NodeId& templateNodeId : templateNodeIds)
                    {
                        if (gatheredNodeIds.contains(templateNodeId))
                        {
                            continue; // This geometry has already been gathered.
                        }

                        HAPI_GeoInfo geoInfo;
                        HAPI_GeoInfo_Init(&geoInfo);
                        HAPI_GetGeoInfo(session, templateNodeId, &geoInfo);

                        // For some reason the return type is always "HAPI_RESULT_INVALID_ARGUMENT", so we
                        // just check the GeoInfo.type manually.
                        if (geoInfo.type != HAPI_GEOTYPE_INVALID)
                        {
                            // Add this template node to the gathered outputs.
                            gatheredNodeIds.insert(templateNodeId);
                            geoInfos.push_back(geoInfo);
                            // If this node doesn't have a part_id count, ensure it gets cooked.
                            forceNodesCook.insert(templateNodeId);
                        }
                    }
                }
            }
        }
        return true;
    }


    bool HoudiniEngineUtils::CookNode(HAPI_Session* session, const HAPI_NodeId& nodeId, HAPI_CookOptions* cookOptions, const bool& waitForCompletion)
    {
        // Check for an invalid node id
        if (nodeId < 0)
            return false;

        // No Cook Options were specified, use the default one
        if (cookOptions == nullptr)
        {
            SessionSettings* settings = nullptr;
            SettingsBus::BroadcastResult(settings, &SettingsBus::Events::GetSessionSettings);
            AZ_Assert(settings, "Settings cannot be null");

            // Use the default cook options
            HAPI_CookOptions options = settings->GetDefaultCookOptions();
            HOUDINI_CHECK_ERROR_RETURN(HAPI_CookNode(session, nodeId, cookOptions), false);
        }
        else
        {
            // Use the provided CookOptions
            HOUDINI_CHECK_ERROR_RETURN(HAPI_CookNode(session, nodeId, cookOptions), false);
        }

        // If we don't need to wait for completion, return now
        if (!waitForCompletion)
        {
            return true;
        }

        // Wait for the cook to finish
        HAPI_Result result = HAPI_RESULT_SUCCESS;
        while (true)
        {
            // Get the current cook status
            int status = HAPI_STATE_STARTING_COOK;

            result = HAPI_GetStatus(session, HAPI_STATUS_COOK_STATE, &status);
            if (status == HAPI_STATE_READY)
            {
                return true;
            }
            else
            if (status == HAPI_STATE_READY_WITH_FATAL_ERRORS || status == HAPI_STATE_READY_WITH_COOK_ERRORS)
            {
                // TODO-GMT: GetCookResults
                AZ_Error("Houdini", false, "There was an error while cooking the node");
                return false;
            }

            AZStd::this_thread::sleep_for(AZStd::chrono::milliseconds(100));

        }
    }

    const AZStd::string HoudiniEngineUtils::GetStatusString(HAPI_StatusType status_type, HAPI_StatusVerbosity verbosity)
    {
        HAPI_Session* session = nullptr;
        SessionRequestBus::BroadcastResult(session, &SessionRequests::GetSessionPtr);
        if (!session)
        {
            return "No active Houdini session";
        }

        int statusBufferLength = 0;
        HAPI_Result result = HAPI_GetStatusStringBufLength(session, status_type, verbosity, &statusBufferLength);

        if (result == HAPI_RESULT_INVALID_SESSION)
        {
            // Let FHoudiniEngine know that the sesion is now invalid to "Stop" the invalid session
            // and clean things up
            // OnSessionLost();
        }

        if (statusBufferLength > 0)
        {
            AZStd::vector< char > statusStringBuffer;
            statusStringBuffer.resize_no_construct(statusBufferLength);
            HAPI_GetStatusString(session, status_type, &statusStringBuffer[0], statusBufferLength);

            return &statusStringBuffer[0];
        }

        return {};
    }

    bool HoudiniEngineUtils::GatherAllAssetOutputs(HAPI_Session* session, const HAPI_NodeId& assetId, const bool bUseOutputNodes, const bool outputTemplatedGeos, AZStd::vector<HAPI_NodeId>& outputNodes)
    {
        outputNodes.clear();

        // Ensure the asset has a valid node ID
        if (assetId < 0)
        {
            return false;
        }

        // Get the AssetInfo
        HAPI_AssetInfo assetInfo;
        HAPI_AssetInfo_Init(&assetInfo);
        bool assetInfoResult = HAPI_RESULT_SUCCESS == HAPI_GetAssetInfo(session, assetId, &assetInfo);

        // Get the Asset NodeInfo
        HAPI_NodeInfo assetNodeInfo;
        HAPI_NodeInfo_Init(&assetNodeInfo);
        HOUDINI_CHECK_ERROR_RETURN(HAPI_GetNodeInfo(session, assetId, &assetNodeInfo), false);

        AZStd::string currentAssetName;
        {
            HoudiniEngineString hapiSTR(assetInfoResult ? assetInfo.nameSH : assetNodeInfo.nameSH);
            hapiSTR.ToAZString(currentAssetName);
        }

        //// In certain cases, such as PDG output processing we might end up with a SOP node instead of a
        //// container. In that case, don't try to run child queries on this node. They will fail.
        const bool assetHasChildren = !(assetNodeInfo.type == HAPI_NODETYPE_SOP && assetNodeInfo.childNodeCount == 0);

        //// Retrieve information about each object contained within our asset.
        AZStd::vector<HAPI_ObjectInfo> objectInfos;
        AZStd::vector<HAPI_Transform> objectTransforms;
        if (!HoudiniEngineUtils::HapiGetObjectInfos(session, assetId, objectInfos, objectTransforms))
        {
            return false;
        }

        // Find the editable nodes in the asset.
        AZStd::vector<HAPI_GeoInfo> editableGeoInfos;
        int editableNodeCount = 0;
        if (assetHasChildren)
        {
            HOUDINI_CHECK_ERROR(HAPI_ComposeChildNodeList(session, assetId, HAPI_NODETYPE_SOP, HAPI_NODEFLAGS_EDITABLE, true, &editableNodeCount));
        }

        // All editable nodes will be output, regardless of whether the subnet is considered visible or not.
        if (editableNodeCount > 0)
        {
            AZStd::vector<HAPI_NodeId> editableNodeIds;
            editableNodeIds.resize_no_construct(editableNodeCount);
            HOUDINI_CHECK_ERROR(HAPI_GetComposedChildNodeList(session, assetId, editableNodeIds.data(), editableNodeCount));

            for (int i = 0; i < editableNodeCount; ++i)
            {
                HAPI_GeoInfo currentEditableGeoInfo;
                HAPI_GeoInfo_Init(&currentEditableGeoInfo);
                HOUDINI_CHECK_ERROR(HAPI_GetGeoInfo(session, editableNodeIds[i], &currentEditableGeoInfo));

                // TODO: Check whether this display geo is actually being output
                //       Just because this is a display node doesn't mean that it will be output (it
                //       might be in a hidden subnet)

                // Do not process the main display geo twice!
                if (currentEditableGeoInfo.isDisplayGeo)
                {
                    continue;
                }

                // We only handle editable curves for now
                if (currentEditableGeoInfo.type != HAPI_GEOTYPE_CURVE)
                {
                    continue;
                }

                // Add this geo to the geo info array
                editableGeoInfos.push_back(currentEditableGeoInfo);
            }
        }

        const bool isSopAsset = assetInfoResult ? (assetInfo.nodeId != assetInfo.objectNodeId) : assetNodeInfo.type == HAPI_NODETYPE_SOP;
        bool useOutputFromSubnets = true;
        if (assetHasChildren)
        {
            if (HoudiniEngineUtils::ContainsSOPNodes(session, assetInfoResult ? assetInfo.nodeId : assetNodeInfo.id))
            {
                // This HDA contains immediate SOP nodes. Don't look for subnets to output.
                useOutputFromSubnets = false;
            }
            else
            {
                // Assume we're using a subnet-based HDA
                useOutputFromSubnets = true;
            }
        }
        else
        {
            // This asset doesn't have any children. Don't try to find subnets.
            useOutputFromSubnets = false;
        }

        // Before we can perform visibility checks on the Object nodes, we have
        // to build a set of all the Object node ids. The 'AllObjectIds' act
        // as a visibility filter. If an Object node is not present in this
        // list, the content of that node will not be displayed (display / output / templated nodes).
        // NOTE that if the HDA contains immediate SOP nodes we will ignore
        // all subnets and only use the data outputs directly from the HDA.

        AZStd::unordered_set<HAPI_NodeId> allObjectIds;
        if (useOutputFromSubnets)
        {
            int numObjSubnets = 0;
            AZStd::vector<HAPI_NodeId> objectIds;
            HOUDINI_CHECK_ERROR_RETURN(HAPI_ComposeChildNodeList(session, assetId, HAPI_NODETYPE_OBJ, HAPI_NODEFLAGS_OBJ_SUBNET, true, &numObjSubnets), false);

            objectIds.resize_no_construct(numObjSubnets);
            HOUDINI_CHECK_ERROR_RETURN(HAPI_GetComposedChildNodeList(session, assetId, objectIds.data(), numObjSubnets), false);
            allObjectIds.insert_range(objectIds);
        }
        else
        {
            allObjectIds.insert(assetInfo.objectNodeId);
        }

        // Iterate through all objects to determine visibility and
        // gather output nodes that needs to be cooked.
        for (int i = 0; i < objectInfos.size(); ++i)
        {
            // Retrieve the object info
            const HAPI_ObjectInfo& currentHapiObjectInfo = objectInfos[i];

            // Determine whether this object node is fully visible.
            bool objectIsVisible = false;
            HAPI_NodeId gatherOutputsNodeId = -1; // Outputs will be gathered from this node.
            if (!assetHasChildren)
            {
                // If the asset doesn't have children, we have to gather outputs from the asset's parent in order to output
                // this asset node
                objectIsVisible = true;
                gatherOutputsNodeId = assetNodeInfo.parentId;
            }
            else if (isSopAsset && currentHapiObjectInfo.nodeId == assetInfo.objectNodeId)
            {
                // When dealing with a SOP asset, be sure to gather outputs from the SOP node, not the
                // outer object node.
                objectIsVisible = true;
                gatherOutputsNodeId = assetInfo.nodeId;
            }
            else
            {
                objectIsVisible = HoudiniEngineUtils::IsOBJNodeFullyVisible(session, allObjectIds, assetId, currentHapiObjectInfo.nodeId);
                gatherOutputsNodeId = currentHapiObjectInfo.nodeId;
            }

            // Build an array of the geos we'll need to process
            // In most case, it will only be the display geo, 
            // but we may also want to process editable geos as well
            AZStd::vector<HAPI_GeoInfo> geoInfos;

            // These node ids may need to be cooked in order to extract part counts.
            AZStd::unordered_set<HAPI_NodeId> forceNodesToCook;

            // Append the initial set of editable geo infos here
            // then clear the editable geo infos array since we
            // only want to process them once.
            geoInfos.append_range(editableGeoInfos);
            editableGeoInfos.clear();

            if (objectIsVisible)
            {
                // NOTE: The HAPI_GetDisplayGeoInfo will not always return the expected Geometry subnet's
                //     Display flag geometry. If the Geometry subnet contains an Object subnet somewhere, the
                //     GetDisplayGeoInfo will sometimes fetch the display SOP from within the subnet which is
                //     not what we want.

                // Resolve and gather outputs (display / output / template nodes) from the GatherOutputsNodeId.
                HoudiniEngineUtils::GatherImmediateOutputGeoInfos(session, gatherOutputsNodeId, bUseOutputNodes, outputTemplatedGeos, geoInfos, forceNodesToCook);

            } // if (bObjectIsVisible)

            for (const HAPI_NodeId& nodeId : forceNodesToCook)
            {
                if (AZStd::find(outputNodes.begin(), outputNodes.end(), nodeId) == outputNodes.end())
                {
                    outputNodes.push_back(nodeId);
                }
            }
        }
        return true;
    }


}

