/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/RTTI/ReflectContext.h>
namespace HoudiniEngine
{
    class HoudiniFbxConfig
    {
    public:
        AZ_CLASS_ALLOCATOR(HoudiniFbxConfig, AZ::SystemAllocator);
        AZ_TYPE_INFO(HoudiniFbxConfig, "{75F6B0E3-685B-459C-896E-6164AA567B34}");
        static void Reflect(AZ::ReflectContext* context);

        bool m_removeHdaAfterBake = false;
        bool m_replacePreviousBake = false;
        bool m_cmdSaveFbx = false;
        AZ::u32 m_bakeCounter = 0;
        void Bake();
    };
}