/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "HE_ParameterWidget_Button.h"

#include "HE_ParameterWidget.h"

#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qpushbutton.h>
#include <string>

namespace HoudiniEngine
{
    HE_ParameterWidget_Button::HE_ParameterWidget_Button(HAPI_ParmId ParmId, const char *ParmLabel)
        : HE_ParameterWidget(ParmId)
    {
        Layout = new QGridLayout;
        Button = new QPushButton(ParmLabel);

        Layout->setAlignment(Qt::AlignTop);
        Layout->addWidget(Button);
        
        this->setLayout(Layout);

        QObject::connect(Button, SIGNAL(clicked()),
                         this, SLOT(ButtonClicked()));
    }

    HE_ParameterWidget_Button::~HE_ParameterWidget_Button()
    {
        delete Button;
        delete Layout;
    }

    void
    HE_ParameterWidget_Button::SetHelpToolTip(std::string HelpString)
    {
        if (Button)
        {
            Button->setToolTip(HelpString.c_str());
        }
    }

    void
    HE_ParameterWidget_Button::ButtonClicked()
    {
        emit Signal_ButtonParmUpdate(Id);
    }
}

#include "SideFX/moc_HE_ParameterWidget_Button.cpp"
