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
    class ExtractPoints;
}

namespace HoudiniEngine
{
    class IHoudiniAsset;

    class ExtractPoints
        : public QDialog
    {
        Q_OBJECT
    public:

        ExtractPoints(IHoudiniNode* node);
        virtual ~ExtractPoints();
       
        const AZStd::vector<AZStd::string>& GetSelectedGroups() const
        {
            return m_selectedGroups;
        }

        bool IsReuseChildren() 
        {
            return m_reuseChildren; 
        }

    private slots:
        void OnSelectionChanged();
        void OnSelectionDoubleClick(QListWidgetItem *item);
        bool eventFilter(QObject *obj, QEvent *event);

    private:
        void LoadData();

        bool m_reuseChildren = true;
        AZStd::vector<AZStd::string> m_selectedGroups;
        IHoudiniNode* m_node;

        Ui::ExtractPoints* m_ui;
    };
}
