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

#include <vector>
#include <string>
#endif

class QGridLayout;
class QHBoxLayout;

namespace HoudiniEngine
{
    class HE_ParameterWidget_Folder : public HE_ParameterWidget
    {
        Q_OBJECT

    public:
        HE_ParameterWidget_Folder() = delete;
        HE_ParameterWidget_Folder(HAPI_ParmId ParmId, const std::string &FolderName);
        ~HE_ParameterWidget_Folder();

        void AppendNewRow(HE_ParameterWidget* Widget);
        void AddWidgetToRow(HE_ParameterWidget* Widget, int Row); 

        std::string GetName() const;

    private:
        QGridLayout* Layout;

        std::vector<HE_ParameterWidget*> ParameterWidgets;
        std::vector<QHBoxLayout*> Rows;

        int RowCount;

        std::string Name;
    };
}
