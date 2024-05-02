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
}
