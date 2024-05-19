/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "HoudiniFbxConfig.h"
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <HoudiniEngine/HoudiniEngineBus.h>
namespace HoudiniEngine
{
    void HoudiniFbxConfig::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<HoudiniFbxConfig>()
                ->Version(0)
                ->Field("RemoveHdaAfterBake", &HoudiniFbxConfig::m_removeHdaAfterBake)
                ->Field("ReplacePreviousBake", &HoudiniFbxConfig::m_replacePreviousBake)
                ->Field("BakeCounter", &HoudiniFbxConfig::m_bakeCounter)
                ;

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<HoudiniFbxConfig>("", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(AZ::Edit::UIHandlers::CheckBox, &HoudiniFbxConfig::m_removeHdaAfterBake, "Remove HDA Output After Bake<br/>", "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::CheckBox, &HoudiniFbxConfig::m_replacePreviousBake, "Replace Previous Bake", "")
                    //->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->UIElement(AZ::Edit::UIHandlers::Button, "Bake")
                    ->Attribute(AZ::Edit::Attributes::ButtonText, "Bake")
                    ->Attribute(AZ::Edit::Attributes::NameLabelOverride, "")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniFbxConfig::Bake)
                    ;
            }
        }
    }

    void HoudiniFbxConfig::Bake()
    {
        HoudiniAssetRequestBus::Broadcast(&HoudiniAssetRequests::SaveToFbx);
    }
}
