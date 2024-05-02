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

        void Apply();
        void Reset();
        void OnSessionTypeChanged(int index);

    private:

        Ui::HoudiniConfiguration* m_ui;
    };
}
