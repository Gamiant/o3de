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

#endif

namespace Ui 
{
    class HoudiniNodeSync;
}

namespace HoudiniEngine
{
    class HoudiniNodeSync
        : public QWidget
        , SessionNotificationBus::Handler
    {
        Q_OBJECT

    public:

        HoudiniNodeSync();
        ~HoudiniNodeSync() override;

    private Q_SLOTS:

        void SendToHoudini();
        void FetchFromHoudini();

    protected:

        void OnSessionStatusChange(SessionRequests::ESessionStatus sessionStatus) override;

    private:

        Ui::HoudiniNodeSync* m_ui;
    };
}
