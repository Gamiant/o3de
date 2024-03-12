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
    // Get the path to the Houdini installlation folder from the Windows registry

    AZStd::string GetHoudiniInstallationPath(int houdiniVersionMajor, int houdiniVersionMinor, int houdiniVersionBuild, int houdiniVersionPatch)
    {
        LPCWSTR registryPath = TEXT("SOFTWARE\\Side Effects Software\\Houdini");

        HKEY keyHandle = nullptr;

        LSTATUS returnCode = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, registryPath, 0, KEY_READ, &keyHandle);

        if (returnCode == ERROR_SUCCESS)
        {
            wchar_t houdiniInstallPath[AZ_MAX_PATH_LEN] = { 0 };
            DWORD dataType = REG_SZ;
            DWORD dataSize = sizeof(houdiniInstallPath);

            AZStd::wstring version = AZStd::wstring::format(L"%d.%d.%d.%d", houdiniVersionMajor, houdiniVersionMinor, houdiniVersionPatch, houdiniVersionBuild);

            returnCode = RegQueryValueExW(keyHandle, version.c_str(), 0, &dataType, (LPBYTE)houdiniInstallPath, &dataSize);
            if (returnCode == ERROR_SUCCESS)
            {
                RegCloseKey(keyHandle);

                using FixedMaxPathString = AZStd::fixed_string<AZ_MAX_PATH_LEN>;
                FixedMaxPathString path;
                AZStd::to_string(path, houdiniInstallPath);
                return path.c_str();
            }

            RegCloseKey(keyHandle);
        }
        return {};
    }
}
