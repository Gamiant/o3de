/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */


#include <Source/UI/ui_HoudiniNodeSync.h>

#include "HoudiniNodeSync.h"

#include <HoudiniEngine/HoudiniEngineBus.h>

namespace HoudiniEngine
{
    HoudiniNodeSync::HoudiniNodeSync()
        : QWidget(nullptr)
        , m_ui(new Ui::HoudiniNodeSync())
    {
        m_ui->setupUi(this);

        connect(m_ui->pbSendToHoudini, &QPushButton::clicked, this, &HoudiniNodeSync::SendToHoudini);
        connect(m_ui->pbFetchFromHoudini, &QPushButton::clicked, this, &HoudiniNodeSync::FetchFromHoudini);

        SessionRequests::ESessionStatus status = SessionRequests::ESessionStatus::Offline;
        SessionRequestBus::BroadcastResult(status, &SessionRequests::GetSessionStatus);
        OnSessionStatusChange(status);

        SessionSettings* settings = nullptr;
        SettingsBus::BroadcastResult(settings, &SettingsBusRequests::GetSessionSettings);
        AZ_Assert(settings, "Settings cannot be null");

        m_ui->pteSendPath->setPlainText(settings->GetSendNodePath().c_str());
        m_ui->pteAssetImportFolder->setPlainText(settings->GetFetchPath().c_str());
        m_ui->pteHoudiniPath->setPlainText(settings->GetSendNodePath().c_str());
        m_ui->pteAssetName->setPlainText    (settings->GetDefaultAssetName().c_str());


        SessionNotificationBus::Handler::BusConnect();
    }
    
    HoudiniNodeSync::~HoudiniNodeSync()
    {
        SessionNotificationBus::Handler::BusDisconnect();
    }

    void HoudiniNodeSync::OnSessionStatusChange(SessionRequests::ESessionStatus sessionStatus)
    {
        switch (sessionStatus)
        {
        case SessionRequests::ESessionStatus::Offline:
            m_ui->lbStatus->setText("Not Connected to Houdini");
            break;
        case SessionRequests::ESessionStatus::Connecting:
            m_ui->lbStatus->setText("Connection in progress...");
            break;
        case SessionRequests::ESessionStatus::Ready:
            m_ui->lbStatus->setText("Connected to Houdini");
            break;
        }
    }

    void HoudiniNodeSync::SendToHoudini()
    {
        NodeSyncRequestBus::Broadcast(&NodeSyncRequests::SendToHoudini);
    }

    void HoudiniNodeSync::FetchFromHoudini()
    {
        NodeSyncRequestBus::Broadcast(&NodeSyncRequests::FetchFromHoudini);
    }

}

#include <UI/moc_HoudiniNodeSync.cpp>
