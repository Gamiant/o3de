/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

namespace HoudiniEngine
{

#pragma once

#if REFERENCE
#include "HAPI/HAPI_Common.h"
    struct HoudiniOutputTranslator
    {
        // 
        static bool UpdateOutputs(HoudiniAssetComponent* HAC, const bool& bInForceUpdate, bool& bOutHasHoudiniStaticMeshOutput);

        //
        static bool BuildStaticMeshesOnHoudiniProxyMeshOutputs(UHoudiniAssetComponent* HAC, bool bInDestroyProxies = false);

        //
        static bool UpdateLoadedOutputs(UHoudiniAssetComponent* HAC);

        //
        static bool UploadChangedEditableOutput(
            UHoudiniAssetComponent* HAC,
            const bool& bInForceUpdate);
        //
        static bool BuildAllOutputs(
            const HAPI_NodeId& AssetId,
            UObject* InOuterObject,
            const TArray<HAPI_NodeId>& OutputNodes,
            const TMap<HAPI_NodeId, int32>& OutputNodeCookCounts,
            TArray<UHoudiniOutput*>& InOldOutputs,
            TArray<UHoudiniOutput*>& OutNewOutputs,
            const bool& InOutputTemplatedGeos,
            const bool& InUseOutputNodes);

        static bool UpdateChangedOutputs(
            UHoudiniAssetComponent* HAC);

        // Helpers functions used to convert HAPI types
        static EHoudiniGeoType ConvertHapiGeoType(const HAPI_GeoType& InType);
        static EHoudiniPartType ConvertHapiPartType(const HAPI_PartType& InType);
        static EHoudiniCurveType ConvertHapiCurveType(const HAPI_CurveType& InType);

        // Helper functions used to cache HAPI infos
        static void CacheObjectInfo(const HAPI_ObjectInfo& InObjInfo, FHoudiniObjectInfo& OutObjInfoCache);
        static void CacheGeoInfo(const HAPI_GeoInfo& InGeoInfo, FHoudiniGeoInfo& OutGeoInfoCache);
        static void CachePartInfo(const HAPI_PartInfo& InPartInfo, FHoudiniPartInfo& OutPartInfoCache);
        static void CacheVolumeInfo(const HAPI_VolumeInfo& InVolumeInfo, FHoudiniVolumeInfo& OutVolumeInfoCache);
        static void CacheCurveInfo(const HAPI_CurveInfo& InCurveInfo, FHoudiniCurveInfo& OutCurveInfoCache);

        /**
         * Helper to clear the outputs of the houdini asset component
         *
         * Some outputs (such as landscapes) need "deferred clearing". This means that
         * these outputs should only be destroyed AFTER the new outputs have been processed.
         *
         * @param   InHAC	All outputs for this Houdini Asset Component will be cleared.
         * @param   OutputsPendingClear	Any outputs that is "pending" clear. These outputs should typically be cleared AFTER the new outputs have been fully processed.
         * @param   bForceClearAll	Setting this flag will force outputs to be cleared here and not take into account outputs requested a deferred clear.
         */
        static void ClearAndRemoveOutputs(UHoudiniAssetComponent* InHAC, TArray<UHoudiniOutput*>& OutputsPendingClear, bool bForceClearAll = false);
        // Helper to clear an individual UHoudiniOutput
        static void ClearOutput(UHoudiniOutput* Output);

        static bool GetCustomPartNameFromAttribute(const HAPI_NodeId& NodeId, const HAPI_PartId& PartId, FString& OutCustomPartName);
    };
#endif
}
