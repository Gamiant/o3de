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
#include <AzCore/std/string/string.h>
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
        HE_ParameterWidget_Folder(HAPI_ParmId ParmId, const AZStd::string &FolderName);
        ~HE_ParameterWidget_Folder();

        void AppendNewRow(HE_ParameterWidget* Widget);
        void AddWidgetToRow(HE_ParameterWidget* Widget, int Row); 

        AZStd::string GetName() const;

    private:
        QGridLayout* Layout;

        AZStd::vector<HE_ParameterWidget*> ParameterWidgets;
        AZStd::vector<QHBoxLayout*> Rows;

        int RowCount;

        AZStd::string Name;
    };
}
