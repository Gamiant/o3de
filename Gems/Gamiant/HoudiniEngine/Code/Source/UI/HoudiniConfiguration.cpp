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

        SessionSettings* settings = nullptr;
        SettingsBus::BroadcastResult(settings, &SettingsBusRequests::GetSessionSettings);
        AZ_Assert(settings, "Settings cannot be null");

        m_ui->pteServerHost->setPlainText(settings->GetServerHost().c_str());
        m_ui->ptePipeName->setPlainText(settings->GetNamedPipe().c_str());
        m_ui->pteServerPort->setPlainText(AZStd::string::format("%d", settings->GetServerPort()).c_str());

        m_ui->cbSessionType->addItem(QString("TCP Socket"));
        m_ui->cbSessionType->addItem(QString("Named Pipe"));

        int sessionType = static_cast<int>(settings->GetSessionType());
        m_ui->cbSessionType->setCurrentIndex(sessionType);
        OnSessionTypeChanged(sessionType);

        connect(m_ui->pbApply, &QPushButton::clicked, this, &HoudiniConfiguration::Apply);
        connect(m_ui->pbReset, &QPushButton::clicked, this, &HoudiniConfiguration::Reset);

        connect(m_ui->cbSessionType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HoudiniConfiguration::OnSessionTypeChanged);
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
            settings->SetServerHost(Globals::SessionServer);
            settings->SetServerPort(Globals::SessionPort);
            settings->SetNamedPipe(Globals::SessionNamedPipe);
            settings->SetSessionType(0);
        }
    }

    void HoudiniConfiguration::OnSessionTypeChanged(int index)
    {
        if (index == 0)
        {
            m_ui->pteServerHost->setEnabled(true);
            m_ui->pteServerPort->setEnabled(true);
            m_ui->ptePipeName->setEnabled(false);
        }
        else
        {
            m_ui->pteServerHost->setEnabled(false);
            m_ui->pteServerPort->setEnabled(false);
            m_ui->ptePipeName->setEnabled(true);

        }
    }

}

#include <UI/moc_HoudiniConfiguration.cpp>
