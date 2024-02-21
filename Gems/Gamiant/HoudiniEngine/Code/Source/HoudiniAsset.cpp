/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <HoudiniCommon.h>

namespace HoudiniEngine
{

    HoudiniDigitalAssetHandler::HoudiniDigitalAssetHandler()
        : AzFramework::GenericAssetHandler<HoudiniDigitalAsset>(
            HoudiniDigitalAsset::DisplayName,
            HoudiniDigitalAsset::Group,
            HoudiniDigitalAsset::Extension)
    {
    }

    AZ::Data::AssetHandler::LoadResult HoudiniDigitalAssetHandler::LoadAssetData(
        const AZ::Data::Asset<AZ::Data::AssetData>& asset,
        AZStd::shared_ptr<AZ::Data::AssetDataStream> stream,
        [[maybe_unused]] const AZ::Data::AssetFilterCB& assetLoadFilterCB)
    {
        HoudiniDigitalAsset* assetData = asset.GetAs<HoudiniDigitalAsset>();
        if (assetData && stream->GetLength() > 0)
        {
            assetData->m_data.resize(stream->GetLength());
            stream->Read(stream->GetLength(), assetData->m_data.data());

            return AZ::Data::AssetHandler::LoadResult::LoadComplete;
        }

        return AZ::Data::AssetHandler::LoadResult::Error;
    }


    HoudiniAsset::HoudiniAsset(IHoudini* hou, HAPI_AssetLibraryId id, AZStd::string hdaFile, AZStd::string hdaName) :
        m_hou(hou)
        , m_id(id)
        , m_hdaFile(hdaFile)
        , m_hdaName(hdaName)
    {
        m_session = const_cast<HAPI_Session*>(&hou->GetSession());
        m_assetCount = 0;

        if (m_id >= 0)
        {
            HAPI_GetAvailableAssetCount(m_session, m_id, &m_assetCount);

            if (m_assetCount > 0)
            {
                AZStd::vector<HAPI_StringHandle> assetNameHandles(m_assetCount);
                HAPI_GetAvailableAssets(m_session, m_id, &assetNameHandles[0], m_assetCount);

                for (int i = 0; i < m_assetCount; i++)
                {
                    AZStd::string assetName = hou->GetString(assetNameHandles[i]);
                    m_assets.push_back(assetName);
                }
            }
        }
    }
}// Namespace HoudiniEngine
