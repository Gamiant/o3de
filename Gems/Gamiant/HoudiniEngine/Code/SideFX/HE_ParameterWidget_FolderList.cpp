/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "HE_ParameterWidget_FolderList.h"

#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qtabwidget.h>

namespace HoudiniEngine
{
    HE_ParameterWidget_FolderList::HE_ParameterWidget_FolderList(HAPI_ParmId ParmId)
        : HE_ParameterWidget(ParmId)
    {
        Layout = new QVBoxLayout;
        FolderContainer = new QTabWidget;

        Layout->setAlignment(Qt::AlignTop);
        Layout->addWidget(FolderContainer);

        this->setLayout(Layout);

        //FolderContainer->hide();
    }

    HE_ParameterWidget_FolderList::~HE_ParameterWidget_FolderList()
    {
        for (std::vector<HE_ParameterWidget_Folder*>::size_type i = 0; i < Folders.size(); i++)
        {
            if (Folders[i])
            {
                delete Folders[i];
            }
        }

        delete FolderContainer;
        delete Layout;
    }

    void
    HE_ParameterWidget_FolderList::AppendFolder(HE_ParameterWidget_Folder* Folder)
    {
        Folders.push_back(Folder);
        FolderContainer->addTab(Folder, Folder->GetName().c_str());
        FolderContainer->show();
    }
}

#include "SideFX/moc_HE_ParameterWidget_FolderList.cpp"
