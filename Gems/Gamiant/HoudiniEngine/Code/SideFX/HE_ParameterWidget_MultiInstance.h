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

#include <AzCore/std/containers/vector.h>

#endif

class QGridLayout;
class QPushButton;

namespace HoudiniEngine
{
    const int MultiInstanceMaxButtonWidth = 20;

    class HE_ParameterWidget_MultiInstance : public HE_ParameterWidget
    {
        Q_OBJECT

    public:
        HE_ParameterWidget_MultiInstance() = delete;
        HE_ParameterWidget_MultiInstance(HAPI_ParmId ParmId, HAPI_ParmId ParentParmId);
        ~HE_ParameterWidget_MultiInstance();

        void AddParameter(HE_ParameterWidget* PWidget);

    private:
        QGridLayout *Layout;
        QPushButton *AddBefore;
        QPushButton *Remove;

        AZStd::vector<HE_ParameterWidget*> ParameterWidgets;

        HAPI_ParmId ParentId;
    };
}
