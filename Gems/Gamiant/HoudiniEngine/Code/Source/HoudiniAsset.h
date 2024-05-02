/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <HoudiniEngine/HoudiniApi.h>

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Asset/AssetSerializer.h>
#include <AzFramework/Asset/GenericAssetHandler.h>

namespace HoudiniEngine 
{
    class HoudiniDigitalAsset
        : public AZ::Data::AssetData
    {
    public:
        static constexpr inline const char* DisplayName = "HoudiniDigitalAsset";
        static constexpr inline const char* Extension = "hda";
        static constexpr inline const char* Group = "Houdini";
        static constexpr inline const char* Icon = "";

        AZ_RTTI(HoudiniDigitalAsset, "{B9776D29-2A0B-46B7-8B27-8F753354AAFA}", AZ::Data::AssetData);
        AZ_CLASS_ALLOCATOR(HoudiniDigitalAsset, AZ::SystemAllocator);

        HoudiniDigitalAsset(const AZ::Data::AssetId& assetId, AZ::Data::AssetData::AssetStatus status = AssetStatus::NotLoaded)
            : AZ::Data::AssetData(assetId, status)
        {
        }

        HAPI_AssetLibraryId m_assetLibraryId = -1;
        AZStd::vector<AZStd::string> m_assetNames;

    };

    class HoudiniDigitalAssetHandler
        : public AZ::Data::AssetHandler
        , public AZ::AssetTypeInfoBus::Handler
    {

    public:

        HoudiniDigitalAssetHandler();
        ~HoudiniDigitalAssetHandler() override;

    protected:

        void Register();
        void Unregister();

        // AZ::Data::AssetHandler...
        AZ::Data::AssetPtr CreateAsset(const AZ::Data::AssetId& id, const AZ::Data::AssetType& type) override;
        AZ::Data::AssetHandler::LoadResult LoadAssetData(
            const AZ::Data::Asset<AZ::Data::AssetData>& asset,
            AZStd::shared_ptr<AZ::Data::AssetDataStream> stream,
            const AZ::Data::AssetFilterCB& assetLoadFilterCB) override;
        void DestroyAsset(AZ::Data::AssetPtr ptr) override;
        void GetHandledAssetTypes(AZStd::vector<AZ::Data::AssetType>& assetTypes) override;
        ///

        // AZ::AssetTypeInfoBus...
        AZ::Data::AssetType GetAssetType() const override;
        void GetAssetTypeExtensions(AZStd::vector<AZStd::string>& extensions) override;
        const char* GetAssetTypeDisplayName() const override;
        const char* GetBrowserIcon() const override;
        const char* GetGroup() const override;
        AZ::Uuid GetComponentTypeId() const override;
        bool CanCreateComponent(const AZ::Data::AssetId& assetId) const override;
        ///
    };

    // GMT-DEPRECATED: this can be removed, we'll use the AssetCatalog to collect/query for HDAs
    class HoudiniAsset : public IHoudiniAsset
    {
        protected:
            IHoudini* m_houdini;
            HAPI_Session* m_session;
            HAPI_AssetLibraryId m_id;
            AZStd::string m_hdaFile;
            AZStd::string m_hdaName;
            
            int m_assetCount;
            AZStd::vector<AZStd::string> m_assets;
        public:

            HoudiniAsset(IHoudini* houdini, HAPI_AssetLibraryId id, AZStd::string hdaFile, AZStd::string hdaName);
            virtual ~HoudiniAsset() = default;

            const AZStd::string& GetHdaName()
            {
                return m_hdaName;
            }
            
            const AZStd::string& GetHdaFile()
            {
                return m_hdaFile;
            }

            const AZStd::vector<AZStd::string>& getAssets()
            {
                return m_assets;
            }
    };
}
