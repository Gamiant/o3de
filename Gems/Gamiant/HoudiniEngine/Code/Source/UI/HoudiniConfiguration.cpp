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


namespace HoudiniEngine
{
    //////////////////////////////////////////////////////////////////////////
    HoudiniConfiguration::HoudiniConfiguration()
        : QWidget(nullptr)
        , m_ui(new Ui::HoudiniConfiguration())
    {
        m_ui->setupUi(this);
        
        
    }
    
    HoudiniConfiguration::~HoudiniConfiguration()
    {

    }
    
    
}

#include <UI/moc_HoudiniConfiguration.cpp>
