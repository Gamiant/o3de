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

#include <HAPI/HAPI.h>
#endif

class QGridLayout;
class QLabel;
class QLineEdit;
class QSlider;

namespace HoudiniEngine
{
    class HE_ParameterWidget_Float : public HE_ParameterWidget
    {
        Q_OBJECT

    public:
        HE_ParameterWidget_Float() = delete;
        HE_ParameterWidget_Float(HAPI_ParmId ParmId, const char *ParmLabel, const AZStd::vector<float> &Values, int ParmSize);
        HE_ParameterWidget_Float(HAPI_ParmId ParmId, const char *ParmLabel, float Value, float Min, float Max);
        ~HE_ParameterWidget_Float();

        void SetHelpToolTip(AZStd::string HelpString) override;

    private:
        QGridLayout *Layout;
        QLabel *Label;
        AZStd::vector<QLineEdit*> Floats;
        QSlider* Slider;

        float UIMin;
        float UIMax;

        int CalculateSliderPosition();
        void UpdateSliderPosition();
        float CalculateFloatValueFromSliderPosition();
        float CalculateFloatValueFromSliderPosition(int Position);

        AZStd::vector<float> GetFloatValues();

    private slots:
        void EditingFinished();
        void SliderValueChanged(int Value);
        void SliderReleased();

    signals:
        void Signal_FloatParmUpdate(HAPI_ParmId, AZStd::vector<float>);
    };
}
