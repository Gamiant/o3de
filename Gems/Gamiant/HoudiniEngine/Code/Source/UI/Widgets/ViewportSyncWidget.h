/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <HoudiniSettings.h>

#include <AzToolsFramework/Prefab/PrefabFocusNotificationBus.h>
#include <AzQtComponents/Components/Widgets/ComboBox.h>

#include <QComboBox>
#include <QLabel>
#include <QWidget>

namespace HoudiniEngine
{

    class ViewportSyncWidget
        : public QWidget
    {
    public:
        ViewportSyncWidget()
        {
            m_label = new QLabel(this);
            m_label->setText("Viewport Sync");

            m_comboBox = new QComboBox(this);
            m_comboBox->setMinimumWidth(140);

            m_comboBox->addItem(QObject::tr("Disabled"));
            m_comboBox->addItem(QObject::tr("Houdini to O3DE"));
            m_comboBox->addItem(QObject::tr("O3DE to Houdini"));
            m_comboBox->addItem(QObject::tr("Both"));

            connect(m_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index) {
                OnComboBoxValueChanged(index);
                });

            QHBoxLayout* layout = new QHBoxLayout(this);
            layout->addWidget(m_label);
            layout->addWidget(m_comboBox);
        }
        ~ViewportSyncWidget() {}

    private:

        void OnComboBoxValueChanged(int index)
        {
            SessionRequestBus::Broadcast(&SessionRequests::SetViewportSync, index);
        }

        QLabel* m_label = nullptr;
        QComboBox* m_comboBox = nullptr;
    };

}
