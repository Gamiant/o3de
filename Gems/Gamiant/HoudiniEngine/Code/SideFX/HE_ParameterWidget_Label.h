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

#include <AzCore/std/string/string.h>
#endif

class QGridLayout;
class QLabel;

namespace HoudiniEngine
{
    class HE_ParameterWidget_Label : public HE_ParameterWidget
    {
        Q_OBJECT

    public:
        HE_ParameterWidget_Label() = delete;
        HE_ParameterWidget_Label(HAPI_ParmId ParmId, const char *LabelText);
        ~HE_ParameterWidget_Label();

        void SetHelpToolTip(AZStd::string HelpString) override;

    private:
        QGridLayout* Layout;
        QLabel* Label;
    };
}
