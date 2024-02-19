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
    class HoudiniSessionControls;
}

namespace HoudiniEngine
{
    class HoudiniSessionControls
        : public QWidget
    {
        Q_OBJECT

    public:

        HoudiniSessionControls();

    private Q_SLOTS:

        void OnOpenHoudini();
        void OnStartSession();
        void OnStopSession();
        void OnRestartSession();
        void OnSyncViewportChanged(int index);

    private:

        Ui::HoudiniSessionControls* m_ui;
    };
}
