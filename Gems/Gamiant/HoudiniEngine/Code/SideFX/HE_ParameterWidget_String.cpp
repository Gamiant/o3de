/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "HE_ParameterWidget_String.h"

#include "HE_ParameterWidget.h"

#include <AzCore/std/containers/vector.h>
#include <AzCore/std/string/string.h>

#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>

namespace HoudiniEngine
{
    HE_ParameterWidget_String::HE_ParameterWidget_String(HAPI_ParmId ParmId, const char *ParmLabel,
                                                            const AZStd::vector<AZStd::string>& Values, int ParmSize)
        : HE_ParameterWidget(ParmId)
    {
        Layout = new QGridLayout;
        Label = new QLabel(ParmLabel);

        Layout->setAlignment(Qt::AlignTop);
        Label->setAlignment(Qt::AlignCenter);
        Layout->setColumnMinimumWidth(0, 100);
        Layout->addWidget(Label, 0, 0);

        Strings.resize(ParmSize);
        for (int i = 0; i < ParmSize; i++)
        {
            Strings[i] = new QLineEdit(Values[i].c_str());
            Layout->addWidget(Strings[i], 0, i + 1);

            QObject::connect(Strings[i], SIGNAL(editingFinished()), this, SLOT(EditingFinished()));
        }

        this->setLayout(Layout);
    }

    HE_ParameterWidget_String::~HE_ParameterWidget_String()
    {
        for (int i = 0; i < Strings.size(); i++)
        {
            if (Strings[i])
            {
                delete Strings[i];
            }
        }

        delete Label;
        delete Layout;
    }

    void
    HE_ParameterWidget_String::SetHelpToolTip(AZStd::string HelpString)
    {
        if (Label)
        {
            Label->setToolTip(HelpString.c_str());
        }
    }

    void
    HE_ParameterWidget_String::EditingFinished()
    {
        AZStd::vector<AZStd::string> StringVals;
        for (int i = 0; i < Strings.size(); i++)
        {
            StringVals.push_back(Strings[i]->text().toStdString().c_str());
        }
        emit Signal_StringParmUpdate(Id, StringVals);
    }
}

#include "SideFX/moc_HE_ParameterWidget_String.cpp"
