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
    class HE_ParameterWidget_IntegerChoice : public HE_ParameterWidget
    {
        Q_OBJECT

    public:
        HE_ParameterWidget_IntegerChoice() = delete;
        HE_ParameterWidget_IntegerChoice(HAPI_ParmId ParmId, const char *ParmLabel,
                                            int ChoiceSize,
                                            const AZStd::vector<AZStd::string> &Choices,
                                            int CurrentChoice);
        ~HE_ParameterWidget_IntegerChoice();

        void SetHelpToolTip(AZStd::string HelpString) override;

    private:
        QGridLayout *Layout;
        QLabel *Label;
        QComboBox *IntChoices;

    private slots:
        void CurrentIndexChanged(int);

    signals:
        void Signal_IntegerChoiceParmUpdate(HAPI_ParmId, int);
    };
}
