/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */


#include <Source/UI/ui_HoudiniConfiguration.h>
#include "HoudiniConfiguration.h"

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
#include <HoudiniSettings.h>


namespace HoudiniEngine
{
    //////////////////////////////////////////////////////////////////////////
    HoudiniConfiguration::HoudiniConfiguration()
        : QWidget(nullptr)
        , m_ui(new Ui::HoudiniConfiguration())
    {
        m_ui->setupUi(this);

        m_ui->cbSessionType->addItem(QString("Named Pipe"));
        m_ui->cbSessionType->addItem(QString("TCP Socket"));

        connect(m_ui->pbApply, &QPushButton::clicked, this, &HoudiniConfiguration::Apply);
        connect(m_ui->pbReset, &QPushButton::clicked, this, &HoudiniConfiguration::Reset);
    }
    
    HoudiniConfiguration::~HoudiniConfiguration()
    {

    }

    void HoudiniConfiguration::Apply()
    {
        SessionSettings* settings = nullptr;
        SettingsBus::BroadcastResult(settings, &SettingsBusRequests::GetSessionSettings);
        if (settings)
        {
            settings->SetServerHost(m_ui->pteServerHost->toPlainText().toUtf8().data());
            settings->SetServerPort(m_ui->pteServerPort->toPlainText().toInt());
            settings->SetNamedPipe(m_ui->ptePipeName->toPlainText().toUtf8().data());
            settings->SetSessionType(m_ui->cbSessionType->currentIndex());
        }

    }

    void HoudiniConfiguration::Reset()
    {
        SessionSettings* settings = nullptr;
        SettingsBus::BroadcastResult(settings, &SettingsBusRequests::GetSessionSettings);
        if (settings)
        {
            settings->SetServerHost("localhost");
            settings->SetServerPort(9090);
            settings->SetNamedPipe("o3de_houdini");
            settings->SetSessionType(0);
        }
    }
}

#include <UI/moc_HoudiniConfiguration.cpp>
