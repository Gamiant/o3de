/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "HE_ParameterWidget_MultiInstance.h"

#include "HE_ParameterWidget.h"

#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qpushbutton.h>

#include <AzCore/Casting/numeric_cast.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/string/string.h>


namespace HoudiniEngine
{
    HE_ParameterWidget_MultiInstance::HE_ParameterWidget_MultiInstance(HAPI_ParmId ParmId, HAPI_ParmId ParentParmId)
        : HE_ParameterWidget(ParmId)
    {
        ParentId = ParentParmId;

        Layout = new QGridLayout;
        AddBefore = new QPushButton("+");
        Remove = new QPushButton("X");

        AddBefore->setMaximumWidth(MultiInstanceMaxButtonWidth);
        Remove->setMaximumWidth(MultiInstanceMaxButtonWidth);

        Layout->addWidget(AddBefore, 0, 0, Qt::AlignLeft);
        Layout->setColumnStretch(0, 0);
        Layout->addWidget(Remove, 0, 1, Qt::AlignLeft);
        Layout->setColumnStretch(1, 0);

        this->setLayout(Layout);
    }

    HE_ParameterWidget_MultiInstance::~HE_ParameterWidget_MultiInstance()
    {
        for (int i = 0; i < ParameterWidgets.size(); i++)
        {
            if (ParameterWidgets[i])
            {
                delete ParameterWidgets[i];
            }
        }

        delete Remove;
        delete AddBefore;
        delete Layout;
    }

    void
    HE_ParameterWidget_MultiInstance::AddParameter(HE_ParameterWidget* PWidget)
    {
        ParameterWidgets.push_back(PWidget);

        if (ParameterWidgets.size() == 1)
        {
            Layout->addWidget(PWidget, aznumeric_cast<int>(ParameterWidgets.size()) - 1, 2, 1, 1);
        }
        else
        {
            Layout->addWidget(PWidget, aznumeric_cast<int>(ParameterWidgets.size()) - 1, 0, 1, 3);
        }
    }
}

#include "SideFX/moc_HE_ParameterWidget_MultiInstance.cpp"
