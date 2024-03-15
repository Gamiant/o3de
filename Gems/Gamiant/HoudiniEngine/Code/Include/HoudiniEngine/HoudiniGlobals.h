/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/base.h>
#include <AzCore/std/string/string_view.h>

namespace HoudiniEngine
{
    namespace Globals
    {
        constexpr float ScaleFactorPosition = 1.f;
        constexpr float ScaleFactorTranslation = 1.f;

        constexpr AZ::u32 SessionPort = 9090;
        constexpr AZStd::string_view SessionServer = "localhost";
        constexpr AZStd::string_view SessionNamedPipe = "o3de_houdini";

        constexpr AZStd::string_view DefaultModelName = "HoudiniEngineMesh";

        constexpr AZStd::string_view DefaultMaterial = "materials/houdinidefault.azmaterial";

    }
}
