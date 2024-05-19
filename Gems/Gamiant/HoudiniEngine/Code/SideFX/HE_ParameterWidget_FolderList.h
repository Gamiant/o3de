/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#if !defined(Q_MOC_RUN)
#include "HE_ParameterWidget_Folder.h"

#include <AzCore/std/containers/vector.h>

#endif

class QVBoxLayout;
class QTabWidget;

namespace HoudiniEngine
{
    class HE_ParameterWidget_FolderList : public HE_ParameterWidget
    {
        Q_OBJECT

    public:
        HE_ParameterWidget_FolderList() = delete;
        HE_ParameterWidget_FolderList(HAPI_ParmId ParmId);
        ~HE_ParameterWidget_FolderList();

        void AppendFolder(HE_ParameterWidget_Folder* Folder);

    private:
        QVBoxLayout* Layout;
        QTabWidget* FolderContainer;

        AZStd::vector<HE_ParameterWidget_Folder*> Folders;
    };
}
