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


namespace HoudiniEngine
{
    /*static std::string get_last_error()
    {
        int buffer_length;
        HAPI_GetStatusStringBufLength(
            nullptr, HAPI_STATUS_CALL_RESULT, HAPI_STATUSVERBOSITY_ERRORS, &buffer_length);
        if (buffer_length > 0)
        {
            char* buf = new char[buffer_length];
            HAPI_GetStatusString(nullptr, HAPI_STATUS_CALL_RESULT, buf, buffer_length);
            std::string result(buf);
            delete[] buf;
            return result;
        }
        return {};
    }*/


    //////////////////////////////////////////////////////////////////////////
    HoudiniSessionControls::HoudiniSessionControls()
        : QWidget(nullptr)
        , m_ui(new Ui::HoudiniSessionControls())
    {
        m_ui->setupUi(this);


        connect(m_ui->pbStartSession, &QPushButton::clicked, this, &HoudiniSessionControls::OnStartSession);
        connect(m_ui->pbStopSession, &QPushButton::clicked, this, &HoudiniSessionControls::OnStopSession);
        connect(m_ui->pbRestartSession, &QPushButton::clicked, this, &HoudiniSessionControls::OnRestartSession);
        connect(m_ui->pbOpenHoudini, &QPushButton::clicked, this, &HoudiniSessionControls::OnOpenHoudini);
    }

    void HoudiniSessionControls::OnOpenHoudini()
    {
        SessionRequestBus::Broadcast(&SessionRequests::OpenHoudini);
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

}

#include <UI/moc_HoudiniSessionControls.cpp>
