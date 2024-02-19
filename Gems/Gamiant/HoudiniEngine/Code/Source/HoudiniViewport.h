/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

namespace HoudiniEngine
{
    class Viewport
    {
    public:

        Viewport();

        void SyncToO3DE();
        void SyncToHoudini();

    protected:

        AZ::Vector3 m_syncedO3DEViewportPosition;
        AZ::Quaternion m_syncedO3DEViewportRotation;
        AZ::Vector3 m_syncedO3DEViewportLookatPosition;

        AZ::Vector3 m_syncedViewportLookAtPosition;
        AZ::Vector3 m_syncedHoudiniViewportPivotPosition;
        AZ::Quaternion m_syncedHoudiniViewportQuat;
        float m_syncedHoudiniViewportOffset;

        bool m_clearOffset = false;
        float m_zeroOffset = 0.f;
    };
}
