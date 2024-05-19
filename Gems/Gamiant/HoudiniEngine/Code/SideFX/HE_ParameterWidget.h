/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#if !defined(Q_MOC_RUN)
#include <QtWidgets/qwidget.h>
#include <HAPI/HAPI.h>
#include <AzCore/std/string/string.h>
#endif

namespace HoudiniEngine
{
    class HE_ParameterWidget : public QWidget
    {
        Q_OBJECT

    public:
        HE_ParameterWidget() = delete;
        HE_ParameterWidget(HAPI_ParmId ParmId);
        ~HE_ParameterWidget();

        virtual void SetHelpToolTip(AZStd::string HelpString);

    protected:
        HAPI_ParmId Id;
    };
}
