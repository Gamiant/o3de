/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#if !defined(Q_MOC_RUN)
#include <HoudiniEngine/HoudiniEngineBus.h>

#include <QDialog>
#include <QMainWindow>
#endif

class QString;
class QLineEdit;
class QStringListModel;

namespace Ui 
{
    class HoudiniConfiguration;
}

namespace HoudiniEngine
{
    class HoudiniConfiguration
        : public QWidget
    {
        Q_OBJECT

    public:

        HoudiniConfiguration();
        ~HoudiniConfiguration() override;

    private Q_SLOTS:

        //REGISTER_INT("hou_multi_threaded", 0, 0, "Out of process version of Houdini running on its own thread");
        //REGISTER_INT("hou_state", 1, 0, "Houdini Enabled State");
        //REGISTER_STRING("hou_otl_path", "@projectroot@/Assets/hda", 0, "Location to search for HDAs - semicolon separated, Example: @devassets@/../../techart/houdini/DigitalAssets");
        //REGISTER_STRING("hou_named_pipe", "HOUDINI_O3DE", 0, "used for debug connections");
        //// FL[FD-10789] Support Mesh as Input to Houdini Digital Asset
        //REGISTER_FLOAT("hou_update_period", 0.25, 0, "Time between Houdini digital asset updates (valid range: 0.01 - 1.0 seconds, default = 0.25s)");

        void Apply();
        void Reset();

    private:

        Ui::HoudiniConfiguration* m_ui;
    };
}
