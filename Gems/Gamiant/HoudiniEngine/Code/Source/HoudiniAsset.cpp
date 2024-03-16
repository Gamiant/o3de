/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <HoudiniAsset.h>
#include <HoudiniEngine/HoudiniEngineBus.h>

#include <AzToolsFramework/API/EditorAssetSystemAPI.h>

namespace HoudiniEngine
{

    HoudiniDigitalAssetHandler::HoudiniDigitalAssetHandler()
    {
        Register();
    }

    HoudiniDigitalAssetHandler::~HoudiniDigitalAssetHandler()
    {
        Unregister();
    }

    void HoudiniDigitalAssetHandler::Register()
    {
        const bool assetManagerReady = AZ::Data::AssetManager::IsReady();
        AZ_Error("HoudiniDigitalAssetHandler", assetManagerReady, "Asset manager isn't ready.");
        if (assetManagerReady)
        {
            AZ::Data::AssetManager::Instance().RegisterHandler(this, AZ::AzTypeInfo<HoudiniDigitalAsset>::Uuid());
        }

        AZ::AssetTypeInfoBus::Handler::BusConnect(AZ::AzTypeInfo<HoudiniDigitalAsset>::Uuid());
    }

    void HoudiniDigitalAssetHandler::Unregister()
    {
        AZ::AssetTypeInfoBus::Handler::BusDisconnect();

        if (AZ::Data::AssetManager::IsReady())
        {
            AZ::Data::AssetManager::Instance().UnregisterHandler(this);
        }
    }

    // AZ::AssetTypeInfoBus
    AZ::Data::AssetType HoudiniDigitalAssetHandler::GetAssetType() const
    {
        return AZ::AzTypeInfo<HoudiniDigitalAsset>::Uuid();
    }

    void HoudiniDigitalAssetHandler::GetAssetTypeExtensions(AZStd::vector<AZStd::string>& extensions)
    {
        extensions.push_back(HoudiniDigitalAsset::Extension);
    }

    const char* HoudiniDigitalAssetHandler::GetAssetTypeDisplayName() const
    {
        return "Houdini Digital Asset";
    }

    const char* HoudiniDigitalAssetHandler::GetBrowserIcon() const
    {
        return "Icons/Components/ColliderMesh.svg";
    }

    const char* HoudiniDigitalAssetHandler::GetGroup() const
    {
        return "Houdini";
    }

    AZ::Uuid HoudiniDigitalAssetHandler::GetComponentTypeId() const
    {
        return AZ::Uuid(HOUDINI_ASSET_COMPONENT_GUID);
    }

    bool HoudiniDigitalAssetHandler::CanCreateComponent([[maybe_unused]] const AZ::Data::AssetId& assetId) const
    {
        return true;
    }

    // AZ::Data::AssetHandler
    AZ::Data::AssetPtr HoudiniDigitalAssetHandler::CreateAsset([[maybe_unused]] const AZ::Data::AssetId& id, const AZ::Data::AssetType& type)
    {
        if (type == AZ::AzTypeInfo<HoudiniDigitalAsset>::Uuid())
        {
            return aznew HoudiniDigitalAsset(id);
        }

        AZ_Error("HoudiniDigitalAssetHandler", false, "This handler deals only with Houdini Digital Asset (HDA) type.");
        return nullptr;
    }

    AZ::Data::AssetHandler::LoadResult HoudiniDigitalAssetHandler::LoadAssetData(
        const AZ::Data::Asset<AZ::Data::AssetData>& asset,
        AZStd::shared_ptr<AZ::Data::AssetDataStream>,
        const AZ::Data::AssetFilterCB&)
    {
        HoudiniPtr houdini;
        HoudiniEngineRequestBus::BroadcastResult(houdini, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);

        if (houdini && houdini->IsActive())
        {
            bool foundAsset = false;
            AZStd::string absolutePath;

            AZ::Data::AssetInfo assetInfo;
            AZ::Data::AssetCatalogRequestBus::BroadcastResult(assetInfo, &AZ::Data::AssetCatalogRequestBus::Events::GetAssetInfoById, asset.GetId());
            if (assetInfo.m_assetType != AZ::Data::s_invalidAssetType)
            {
                AzToolsFramework::AssetSystemRequestBus::BroadcastResult(foundAsset, &AzToolsFramework::AssetSystemRequestBus::Events::GetFullSourcePathFromRelativeProductPath, asset.GetHint(), absolutePath);
            }

            HAPI_AssetLibraryId id;
            HAPI_Result result = HAPI_LoadAssetLibraryFromFile(&houdini->GetSession(), absolutePath.c_str(), true, &id);
            if (result != HAPI_RESULT_SUCCESS)
            {
                AZ_Error("Houdini", false, houdini->GetLastHoudiniError().c_str());
                return AssetHandler::LoadResult::Error;
            }

            HoudiniDigitalAsset* hda = asset.GetAs<HoudiniDigitalAsset>();
            hda->m_assetLibraryId = id;

            int assetCount = 0;
            HAPI_GetAvailableAssetCount(&houdini->GetSession(), id, &assetCount);

            if (assetCount > 0)
            {
                AZStd::vector<HAPI_StringHandle> assetNameHandles(assetCount);
                HAPI_GetAvailableAssets(&houdini->GetSession(), id, &assetNameHandles[0], assetCount);

                for (int i = 0; i < assetCount; i++)
                {
                    AZStd::string assetName = houdini->GetString(assetNameHandles[i]);
                    hda->m_assetNames.push_back(assetName);
                }
            }

            AZ_Info("Houdini", AZStd::string::format("HDA Loaded %s\n", assetInfo.m_relativePath.c_str()).c_str());
        }
        else
        {
            // Need to figure out how to do this, I may need to register assets like this
            // so that they're loaded when a Houdini session is established
            return AssetHandler::LoadResult::MoreDataRequired;
        }

        return AssetHandler::LoadResult::LoadComplete;
    }

    void HoudiniDigitalAssetHandler::DestroyAsset(AZ::Data::AssetPtr ptr)
    {
        delete ptr;
    }

    void HoudiniDigitalAssetHandler::GetHandledAssetTypes(AZStd::vector<AZ::Data::AssetType>& assetTypes)
    {
        assetTypes.push_back(AZ::AzTypeInfo<HoudiniDigitalAsset>::Uuid());
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
