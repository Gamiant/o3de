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
#include <AzCore/std/string/string.h>
#endif

class QGridLayout;
class QPushButton;

namespace HoudiniEngine
{
    class HE_ParameterWidget_Button : public HE_ParameterWidget
    {
        Q_OBJECT

    public:
        HE_ParameterWidget_Button() = delete;
        HE_ParameterWidget_Button(HAPI_ParmId ParmId, const char *ParmLabel);
        ~HE_ParameterWidget_Button();

        void SetHelpToolTip(AZStd::string HelpString) override;

    private:
        QGridLayout *Layout;
        QPushButton *Button;

    private slots:
        void ButtonClicked();

    signals:
        void Signal_ButtonParmUpdate(HAPI_ParmId);
    };
}
