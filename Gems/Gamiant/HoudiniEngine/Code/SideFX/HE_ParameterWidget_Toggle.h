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
#include <string>
#endif

class QHBoxLayout;
class QCheckBox;
class QLabel;
class QSpacerItem;

namespace HoudiniEngine
{
    const int ToggleSpacerItemWidth = 40;

    class HE_ParameterWidget_Toggle : public HE_ParameterWidget
    {
        Q_OBJECT

    public:
        HE_ParameterWidget_Toggle() = delete;
        HE_ParameterWidget_Toggle(HAPI_ParmId ParmId, const char *ParmLabel, bool Checked);
        ~HE_ParameterWidget_Toggle();

        void SetHelpToolTip(std::string HelpString) override;

    private:
        QHBoxLayout *Layout;
        QCheckBox *Toggle;
        QLabel *Label;
        QSpacerItem *Spacer;

    private slots:
        void ToggleStateChanged(int);

    signals:
        void Signal_ToggleParmUpdate(HAPI_ParmId, int);
    };
}
