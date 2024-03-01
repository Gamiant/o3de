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
        , SessionNotificationBus::Handler
    {
        Q_OBJECT

    public:

        HoudiniSessionControls();
        ~HoudiniSessionControls() override;

    private Q_SLOTS:

        void OnOpenHoudini();
        void OnCloseHoudini();
        void OnStartSession();
        void OnStopSession();
        void OnRestartSession();
        void OnSyncViewportChanged(int index);

    private:

        Ui::HoudiniSessionControls* m_ui;

    protected:

        // SessionNotificationBus::Handler...
        void OnViewportSyncChange(SessionSettings::EViewportSync) override;
        ///
    };
}
