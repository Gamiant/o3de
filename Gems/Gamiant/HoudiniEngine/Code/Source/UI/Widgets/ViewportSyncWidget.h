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
        , SessionNotificationBus::Handler
    {
    public:
        ViewportSyncWidget()
        {
            SessionNotificationBus::Handler::BusConnect();

            m_label = new QLabel(this);
            m_label->setText("Viewport Sync");

            m_comboBox = new QComboBox(this);
            m_comboBox->setMinimumWidth(140);

            m_comboBox->addItem(QObject::tr("Disabled"));
            m_comboBox->addItem(QObject::tr("Houdini to O3DE"));
            m_comboBox->addItem(QObject::tr("O3DE to Houdini"));
            m_comboBox->addItem(QObject::tr("Both"));

            connect(m_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [&](int index)
                {
                    OnComboBoxValueChanged(index);
                });

            SessionSettings* settings = nullptr;
            SettingsBus::BroadcastResult(settings, &SettingsBusRequests::GetSessionSettings);
            AZ_Assert(settings, "Settings cannot be null");

            if (settings->GetSyncO3DEViewport() && settings->GetSyncHoudiniViewport())
            {
                m_comboBox->setCurrentIndex(static_cast<int>(SessionSettings::EViewportSync::Both));
            }
            else
            if (settings->GetSyncO3DEViewport())
            {
                m_comboBox->setCurrentIndex(static_cast<int>(SessionSettings::EViewportSync::O3DEToHoudini));
            }
            else
            if (settings->GetSyncHoudiniViewport())
            {
                m_comboBox->setCurrentIndex(static_cast<int>(SessionSettings::EViewportSync::HoudiniToO3DE));
            }

            QHBoxLayout* layout = new QHBoxLayout(this);
            layout->addWidget(m_label);
            layout->addWidget(m_comboBox);
        }

        ~ViewportSyncWidget()
        {
            SessionNotificationBus::Handler::BusDisconnect();
        }

    protected:

        void OnViewportSyncChange(SessionSettings::EViewportSync sync)
        {
            m_comboBox->setCurrentIndex(static_cast<int>(sync));
        }

        void OnComboBoxValueChanged(int index)
        {
            SessionNotificationBus::Handler::BusDisconnect();

            SessionRequestBus::Broadcast(&SessionRequests::SetViewportSync, index);

            SessionNotificationBus::Handler::BusConnect();
        }

        QLabel* m_label = nullptr;
        QComboBox* m_comboBox = nullptr;
    };

}
