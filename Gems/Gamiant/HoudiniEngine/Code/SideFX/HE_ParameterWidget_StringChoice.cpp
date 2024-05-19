/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "HE_ParameterWidget_StringChoice.h"

#include "HE_ParameterWidget.h"

#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qcombobox.h>

#include <AzCore/std/containers/vector.h>
#include <AzCore/std/string/string.h>

namespace HoudiniEngine
{
    HE_ParameterWidget_StringChoice::HE_ParameterWidget_StringChoice(HAPI_ParmId ParmId, const char *ParmLabel,
                                                                     int ChoiceSize,
                                                                     const AZStd::vector<AZStd::string> &Choices,
                                                                     const AZStd::vector<AZStd::string> &ChoiceVals,
                                                                     AZStd::string CurrentChoice)
        : HE_ParameterWidget(ParmId)
    {
        Layout = new QGridLayout;
        Label = new QLabel(ParmLabel);
        StringChoices = new QComboBox;

        Layout->setAlignment(Qt::AlignTop);
        Label->setAlignment(Qt::AlignCenter);
        Layout->addWidget(Label, 0, 0);
        Layout->addWidget(StringChoices, 0, 1);
        Layout->setColumnMinimumWidth(0, 100);

        this->setLayout(Layout);

        QStringList QChoices;
        int ChoiceIndex = 0;
        for (int i = 0; i < ChoiceSize; i++)
        {
            QChoices << Choices[i].c_str();
            if (ChoiceVals[i] == CurrentChoice)
            {
                ChoiceIndex = i;
            }

            Values.push_back(ChoiceVals[i]);
        }

        StringChoices->addItems(QChoices);

        StringChoices->setCurrentIndex(ChoiceIndex);

        QObject::connect(StringChoices, SIGNAL(currentIndexChanged(int)),
                         this, SLOT(CurrentIndexChanged(int)));
    }

    HE_ParameterWidget_StringChoice::~HE_ParameterWidget_StringChoice()
    {
        delete StringChoices;
        delete Label;
        delete Layout;
    }

    void
    HE_ParameterWidget_StringChoice::SetHelpToolTip(AZStd::string HelpString)
    {
        if (Label)
        {
            Label->setToolTip(HelpString.c_str());
        }
    }

    void
    HE_ParameterWidget_StringChoice::CurrentIndexChanged(int Index)
    {
        emit Signal_StringChoiceParmUpdate(Id, Values[Index]);
    }
}

#include "SideFX/moc_HE_ParameterWidget_StringChoice.cpp"
