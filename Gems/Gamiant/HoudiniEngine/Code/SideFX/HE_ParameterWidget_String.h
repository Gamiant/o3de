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
#endif

class QGridLayout;
class QLabel;
class QLineEdit;

namespace HoudiniEngine
{
    class HE_ParameterWidget_String : public HE_ParameterWidget
    {
        Q_OBJECT

    public:
        HE_ParameterWidget_String() = delete;
        HE_ParameterWidget_String(HAPI_ParmId ParmId, const char *ParmLabel, const std::vector<std::string>& Values, int ParmSize);
        ~HE_ParameterWidget_String();

        void SetHelpToolTip(std::string HelpString) override;

    private:
        QGridLayout *Layout;
        QLabel *Label;
        std::vector<QLineEdit*> Strings;

    private slots:
        void EditingFinished();

    signals:
        void Signal_StringParmUpdate(HAPI_ParmId, std::vector<std::string>);
    };
}
