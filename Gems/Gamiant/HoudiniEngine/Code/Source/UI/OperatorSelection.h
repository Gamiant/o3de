/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#if !defined(Q_MOC_RUN)
#include <HoudiniEngine/HoudiniEngineBus.h>

#include <QDialog>
#include <QMainWindow>
#include <QListWidgetItem>
#endif

class QString;
class QLineEdit;
class QStringListModel;

namespace Ui 
{
    class OperatorSelection;
}

namespace HoudiniEngine
{
    class IHoudiniAsset;

    class OperatorSelection
        : public QDialog
    {
        Q_OBJECT
    public:

        OperatorSelection(OperatorMode selectionMode, AZ::Data::Asset<HoudiniDigitalAsset> asset);
        virtual ~OperatorSelection();
       
        const AZStd::string& GetSelectedOperator() const
        {
            return m_selectedOperator;
        }

    private slots:
        void OnSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);
        void OnSelectionDoubleClick(QListWidgetItem *item);
        void OnLinkClicked(const QUrl& item);
        bool eventFilter(QObject *obj, QEvent *event);

    private:
        void LoadData();
        void AddAsset(const AZStd::string& operatorName);

        AZStd::string m_selectedOperator;
        OperatorMode m_selectionMode = OperatorMode::Assets;

        AZ::Data::Asset<HoudiniDigitalAsset> m_asset;

        Ui::OperatorSelection* m_ui;
    };
}
