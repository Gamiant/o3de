/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "HE_ParameterWidget.h"

#include <HAPI/HAPI.h>
#include <string>

namespace HoudiniEngine
{
    HE_ParameterWidget::HE_ParameterWidget(HAPI_ParmId ParmId)
    {
        Id = ParmId;
    }

    HE_ParameterWidget::~HE_ParameterWidget()
    {

    }

    void
    HE_ParameterWidget::SetHelpToolTip(std::string HelpString)
    {
        this->setToolTip(HelpString.c_str());
    }
}

#include "SideFX/moc_HE_ParameterWidget.cpp"
