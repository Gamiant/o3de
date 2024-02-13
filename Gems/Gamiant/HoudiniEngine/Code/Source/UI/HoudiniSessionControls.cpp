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
    //////////////////////////////////////////////////////////////////////////
    HoudiniSessionControls::HoudiniSessionControls()
        : QWidget(nullptr)
        , m_ui(new Ui::HoudiniSessionControls())
    {
        m_ui->setupUi(this);
        
        //TODO: This doesn't seem to work atm:
        //connect(m_ui->cmdInterrupt, &QPushButton::clicked, this, &HoudiniSessionControls::OnClickInterrupt);
        
    }
    
    HoudiniSessionControls::~HoudiniSessionControls()
    {

    }

}

#include <UI/moc_HoudiniSessionControls.cpp>
