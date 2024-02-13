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
    class HoudiniStatusPanel;
}

namespace HoudiniEngine
{
    class HoudiniStatusPanel
        : public QWidget
    {
        Q_OBJECT
    public:

        HoudiniStatusPanel();
        ~HoudiniStatusPanel() override;
        
        void UpdatePercent(AZStd::string text, int percent, int assetsInQueue, bool asyncMode);

        AZStd::function<void()> OnInterrupt = {};

    private Q_SLOTS:
        

    private:

        void OnClickInterrupt();

        Ui::HoudiniStatusPanel* m_ui;
    };
}
