/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Platform.h>
#include <AzCore/PlatformIncl.h>

#include <HoudiniEngine/HoudiniPlatform.h>
#include <HoudiniEngine/HoudiniVersion.h>
#include <AzCore/std/string/conversions.h>
#include <AzCore/IO/SystemFile.h>

namespace HoudiniEngine
{
    // Get the path to the Houdini installlation folder
    AZStd::string GetHoudiniInstallationPath(int /*houdiniVersionMajor*/, int /*houdiniVersionMinor*/, int /*houdiniVersionBuild*/, int /*houdiniVersionPatch*/)
    {
        return {};
    }
}
