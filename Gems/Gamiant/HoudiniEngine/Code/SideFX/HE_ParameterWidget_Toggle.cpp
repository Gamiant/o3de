/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "HE_ParameterWidget_Toggle.h"

#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qlabel.h>

namespace HoudiniEngine
{
    HE_ParameterWidget_Toggle::HE_ParameterWidget_Toggle(HAPI_ParmId ParmId, const char *ParmLabel, bool Checked)
        : HE_ParameterWidget(ParmId)
    {
        Layout = new QHBoxLayout;
        Toggle = new QCheckBox;
        Label = new QLabel(ParmLabel);
        Spacer = new QSpacerItem(ToggleSpacerItemWidth, 0);

        Toggle->setChecked(Checked);

        Layout->setAlignment(Qt::AlignLeft);
        Layout->addSpacerItem(Spacer);
        Layout->addWidget(Toggle);
        Layout->addWidget(Label);

        this->setLayout(Layout);

        QObject::connect(Toggle, SIGNAL(stateChanged(int)),
                         this, SLOT(ToggleStateChanged(int)));
    }

    HE_ParameterWidget_Toggle::~HE_ParameterWidget_Toggle()
    {
        Layout->removeItem(Spacer);
        delete Spacer;
        delete Label;
        delete Toggle;
        delete Layout;
    }

    void
    HE_ParameterWidget_Toggle::SetHelpToolTip(AZStd::string HelpString)
    {
        if (Label)
        {
            Label->setToolTip(HelpString.c_str());
        }
    }

    void
    HE_ParameterWidget_Toggle::ToggleStateChanged(int /*State*/)
    {
        int On = (Toggle->isChecked() ? 1 : 0);
        emit Signal_ToggleParmUpdate(Id, On);
    }
}

#include "SideFX/moc_HE_ParameterWidget_Toggle.cpp"
