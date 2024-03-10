/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#if !defined(Q_MOC_RUN)

#include <HoudiniSettings.h>

#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>

#endif

namespace HoudiniEngine
{

    class StatusWidget
        : public QWidget
        , SessionNotificationBus::Handler
    {
    public:
        StatusWidget()
        {
            SessionNotificationBus::Handler::BusConnect();

            m_label = new QLabel(this);
            m_label->setText("");

            QHBoxLayout* layout = new QHBoxLayout(this);
            layout->addWidget(m_label);

            OnSessionStatusChange(SessionRequests::ESessionStatus::Offline);
        }

        ~StatusWidget()
        {
            SessionNotificationBus::Handler::BusDisconnect();
        }

    protected:

        void OnSessionStatusChange(SessionRequests::ESessionStatus sessionStatus) override
        {
            switch (sessionStatus)
            {
            case SessionRequests::ESessionStatus::Offline:
                m_label->setText("Not Connected");
                break;
            case SessionRequests::ESessionStatus::Connecting:
                m_label->setText("Connecting");
                break;
            case SessionRequests::ESessionStatus::Ready:
                m_label->setText("Connected");
                break;
            }
        }

        void OnSendToHoudini() override
        {
            m_label->setText("Send Success");
        }

        void OnFetchFromHoudini() override
        {
            m_label->setText("Fetch Success");
        }

    private:

        QLabel* m_label = nullptr;
    };

}
