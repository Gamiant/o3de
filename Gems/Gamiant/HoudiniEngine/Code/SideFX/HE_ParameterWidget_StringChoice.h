/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#if !defined(Q_MOC_RUN)
#include "HE_ParameterWidget.h"

#include <HAPI/HAPI.h>

#include <AzCore/std/containers/vector.h>
#include <AzCore/std/string/string.h>

#endif

class QGridLayout;
class QLabel;
class QComboBox;

namespace HoudiniEngine
{
    class HE_ParameterWidget_StringChoice : public HE_ParameterWidget
    {
        Q_OBJECT

    public:
        HE_ParameterWidget_StringChoice() = delete;
        HE_ParameterWidget_StringChoice(HAPI_ParmId ParmId, const char *ParmLabel,
                                        int ChoiceSize,
                                        const AZStd::vector<AZStd::string> &Choices,
                                        const AZStd::vector<AZStd::string> &ChoiceVals,
                                        AZStd::string CurrentChoice);
        ~HE_ParameterWidget_StringChoice();

        void SetHelpToolTip(AZStd::string HelpString) override;

    private:
        QGridLayout *Layout;
        QLabel *Label;
        QComboBox *StringChoices;

        AZStd::vector<AZStd::string> Values;

    private slots:
        void CurrentIndexChanged(int Index);

    signals:
        void Signal_StringChoiceParmUpdate(HAPI_ParmId, AZStd::string);
    };
}
