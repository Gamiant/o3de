/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */


#include <Source/UI/ui_HoudiniSessionControls.h>
#include "HoudiniSessionControls.h"

#include <HoudiniEngine/HoudiniApi.h>
#include <HoudiniEngine/HoudiniEngineBus.h>
#include "Components/HoudiniNodeComponentConfig.h"
#include <HoudiniCommon.h>

#include <QtUtilWin.h>
#include <QFile>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QJsonDocument>
#include <QSettings>

#include <QPushButton>
#include <QStringListModel>
#include <QComboBox>

namespace HoudiniEngine
{
    //////////////////////////////////////////////////////////////////////////
    HoudiniSessionControls::HoudiniSessionControls()
        : QWidget(nullptr)
        , m_ui(new Ui::HoudiniSessionControls())
    {
        m_ui->setupUi(this);

        m_ui->cbSyncViewport->addItem(QString("Disabled"));
        m_ui->cbSyncViewport->addItem(QString("Houdini to O3DE"));
        m_ui->cbSyncViewport->addItem(QString("O3DE to Houdini"));
        m_ui->cbSyncViewport->addItem(QString("Both"));

        SessionSettings* settings = nullptr;
        SettingsBus::BroadcastResult(settings, &SettingsBusRequests::GetSessionSettings);
        AZ_Assert(settings, "Settings cannot be null");

        bool syncHoudiniViewport = settings->GetSyncHoudiniViewport();
        bool syncO3DEViewport = settings->GetSyncO3DEViewport();

        int index = 0;
        if (syncHoudiniViewport && syncO3DEViewport)
        {
            index = static_cast<int>(SessionSettings::EViewportSync::Both);
        }
        else
        if (syncHoudiniViewport)
        {
            index = static_cast<int>(SessionSettings::EViewportSync::HoudiniToO3DE);
        }
        else
        if (syncO3DEViewport)
        {
            index = static_cast<int>(SessionSettings::EViewportSync::O3DEToHoudini);
        }

        SessionRequestBus::Broadcast(&SessionRequests::SetViewportSync, index);
        m_ui->cbSyncViewport->setCurrentIndex(index);

        connect(m_ui->cbSyncViewport, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HoudiniSessionControls::OnSyncViewportChanged);

        connect(m_ui->pbStartSession, &QPushButton::clicked, this, &HoudiniSessionControls::OnStartSession);
        connect(m_ui->pbStopSession, &QPushButton::clicked, this, &HoudiniSessionControls::OnStopSession);
        connect(m_ui->pbRestartSession, &QPushButton::clicked, this, &HoudiniSessionControls::OnRestartSession);
        connect(m_ui->pbOpenHoudini, &QPushButton::clicked, this, &HoudiniSessionControls::OnOpenHoudini);
        connect(m_ui->pbCloseHoudini, &QPushButton::clicked, this, &HoudiniSessionControls::OnCloseHoudini);

        SessionNotificationBus::Handler::BusConnect();

    }

    HoudiniSessionControls::~HoudiniSessionControls()
    {
        SessionNotificationBus::Handler::BusDisconnect();
    }

    void HoudiniSessionControls::OnOpenHoudini()
    {
        SessionRequestBus::Broadcast(&SessionRequests::OpenHoudini);
    }

    void HoudiniSessionControls::OnCloseHoudini()
    {
        SessionRequestBus::Broadcast(&SessionRequests::CloseHoudini);
    }

    void HoudiniSessionControls::OnStartSession()
    {
        SessionRequestBus::Broadcast(&SessionRequests::StartSession);
    }

    void HoudiniSessionControls::OnStopSession()
    {
        SessionRequestBus::Broadcast(&SessionRequests::StopSession);
    }

    void HoudiniSessionControls::OnRestartSession()
    {
        SessionRequestBus::Broadcast(&SessionRequests::RestartSession);
    }

    void HoudiniSessionControls::OnSyncViewportChanged(int index)
    {
        SessionNotificationBus::Handler::BusDisconnect();

        SessionRequestBus::Broadcast(&SessionRequests::SetViewportSync, index);

        SessionNotificationBus::Handler::BusConnect();
    }

    void HoudiniSessionControls::OnViewportSyncChange(SessionSettings::EViewportSync sync)
    {
        m_ui->cbSyncViewport->setCurrentIndex(static_cast<int>(sync));
    }

}

#include <UI/moc_HoudiniSessionControls.cpp>
