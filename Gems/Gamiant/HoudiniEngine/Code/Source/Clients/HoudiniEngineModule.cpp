/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <HoudiniEngineModuleInterface.h>
#include "HoudiniEngineSystemComponent.h"

namespace HoudiniEngine
{
    class HoudiniEngineModule
        : public HoudiniEngineModuleInterface
    {
    public:
        AZ_RTTI(HoudiniEngineModule, "{11F7E041-CA6B-4BF5-9E84-79E77947BD2E}", HoudiniEngineModuleInterface);
        AZ_CLASS_ALLOCATOR(HoudiniEngineModule, AZ::SystemAllocator, 0);
    };
}// namespace HoudiniEngine

AZ_DECLARE_MODULE_CLASS(Gem_HoudiniEngine, HoudiniEngine::HoudiniEngineModule)
