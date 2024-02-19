/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <HoudiniViewport.h>
#include <HoudiniSettings.h>
#include <HoudiniEngine/HoudiniEngineBus.h>
#include <HoudiniEngine/HoudiniGlobals.h>

#include <Atom/RPI.Public/Base.h>
#include <Atom/RPI.Public/ViewportContextBus.h>
#include <Atom/RPI.Public/ViewportContext.h>

#include <HAPI/HAPI.h>

namespace HoudiniEngine
{
    Viewport::Viewport()
        : m_syncedHoudiniViewportPivotPosition(AZ::Vector3::CreateZero())
        , m_syncedHoudiniViewportQuat(AZ::Quaternion::CreateIdentity())
        , m_syncedHoudiniViewportOffset(0.f)
        , m_clearOffset(false)
    {}

    bool Viewport::IsViewportSyncEnabled() const
    {
        SessionSettings* settings = nullptr;
        SettingsBus::BroadcastResult(settings, &SettingsBusRequests::GetSessionSettings);
        AZ_Assert(settings, "Settings cannot be null");

        return settings->GetSyncViewports();

    }

    void Viewport::SyncToO3DE()
    {
        if (!IsViewportSyncEnabled())
        {
            return;
        }

        const auto viewportContextRequests = AZ::RPI::ViewportContextRequests::Get();

        AZ::RPI::ViewportContextPtr viewportContext = viewportContextRequests->GetDefaultViewportContext();
        if (!viewportContext)
        {
            return;
        }

        HAPI_Session* session = nullptr;
        SessionRequestBus::BroadcastResult(session, &SessionRequests::GetSessionPtr);
        if (!session)
        {
            return;
        }

        HAPI_Viewport hapiViewport;
        if (HAPI_GetViewport(session, &hapiViewport) != HAPI_RESULT_SUCCESS)
        {
            return;
        }


        // Get Hapi viewport's PivotPosition, Offset and Quat,  w.r.t Houdini's coordinate and scale.
        AZ::Vector3 viewportPivotPosition = AZ::Vector3(hapiViewport.position[0], hapiViewport.position[1], hapiViewport.position[2]);
        float viewportOffset = hapiViewport.offset;
        AZ::Quaternion viewportQuat = AZ::Quaternion(hapiViewport.rotationQuaternion[0],
            hapiViewport.rotationQuaternion[1],
            hapiViewport.rotationQuaternion[2],
            hapiViewport.rotationQuaternion[3]);

        // Only update if the viewport has changed
        if (m_syncedHoudiniViewportPivotPosition.IsClose(viewportPivotPosition) &&
            m_syncedHoudiniViewportQuat.IsClose(viewportQuat) &&
            AZ::IsClose(m_syncedHoudiniViewportOffset, viewportOffset))
        {
            return;
        }

        // Set zero value of offset when needed
        if (m_clearOffset)
        {
            m_clearOffset = hapiViewport.offset;
            m_clearOffset = false;
        }

        // Translate Houdini's camera transform to O3DE


        AZ::Vector3 o3deViewportPivotPosition = AZ::Vector3(hapiViewport.position[0], hapiViewport.position[2], hapiViewport.position[1]) * Globals::ScaleFactorTranslation;

        static float test = 0.1f;
        float o3deOffset = (hapiViewport.offset - m_zeroOffset) * (Globals::ScaleFactorTranslation * test);

        AZ::Quaternion o3deQuaternion = AZ::Quaternion(hapiViewport.rotationQuaternion[0], hapiViewport.rotationQuaternion[2], hapiViewport.rotationQuaternion[1], -hapiViewport.rotationQuaternion[3]);
        o3deQuaternion = o3deQuaternion * AZ::Quaternion::CreateFromEulerAnglesDegrees(AZ::Vector3(0.f, 0.f, 180.f));


        //AZ::Vector3 o3deForward = viewportContext->GetCameraViewMatrixAsMatrix3x4().GetBasisX(); //AZ::Vector3::CreateAxisX();
        AZ::Vector3 o3deForward = AZ::Vector3::CreateAxisX();

        AZ::Quaternion o3deQuat = AZ::Quaternion::CreateIdentity();
        AZ::Vector3 viewPosition = o3deQuat.TransformVector(o3deForward) * o3deOffset + o3deViewportPivotPosition;

        AZ::Transform cameraTransform;
        cameraTransform.SetTranslation(viewPosition);
        cameraTransform.SetRotation(o3deQuaternion);
        viewportContext->SetCameraTransform(cameraTransform);

        m_syncedHoudiniViewportOffset = viewportOffset;
        m_syncedHoudiniViewportPivotPosition = viewportPivotPosition;
        m_syncedHoudiniViewportQuat = viewportQuat;

    }

    void Viewport::SyncToHoudini()
    {
        if (!IsViewportSyncEnabled())
        {
            return;
        }

        HAPI_Session* session = nullptr;
        SessionRequestBus::BroadcastResult(session, &SessionRequests::GetSessionPtr);
        if (!session)
        {
            return;
        }

        HAPI_Viewport hapiViewport;
        if (HAPI_GetViewport(session, &hapiViewport) != HAPI_RESULT_SUCCESS)
        {
            return;
        }

        const auto viewportContextRequests = AZ::RPI::ViewportContextRequests::Get();

        AZ::RPI::ViewportContextPtr viewportContext = viewportContextRequests->GetDefaultViewportContext();
        if (!viewportContext)
        {
            return;
        }

        AZ::Quaternion viewportRotation = viewportContext->GetCameraTransform().GetRotation();
        AZ::Vector3 viewportPivotPosition = viewportContext->GetCameraTransform().GetTranslation();
        AZ::Vector3 viewportLookAtPosition = viewportContext->GetCameraViewMatrix().GetTranslation();

        // Only update if the viewport has changed
        if (m_syncedHoudiniViewportPivotPosition.IsClose(viewportPivotPosition) &&
            m_syncedHoudiniViewportQuat.IsClose(viewportRotation) &&
            m_syncedViewportLookAtPosition.IsClose(viewportLookAtPosition))
        {
            return;
        }

        // Set zero value of offset when needed
        if (m_clearOffset)
        {
            m_clearOffset = hapiViewport.offset;
            m_clearOffset = false;
        }

        // TODO-GMT: check orbit camera support
        AZ::Quaternion hapiQuaternion = viewportRotation;
        hapiQuaternion = hapiQuaternion * AZ::Quaternion::CreateFromEulerAnglesDegrees(AZ::Vector3(0.f, 0.f, 180.f));




        /* Update Hapi H_View */
        // Note: There are infinte number of H_View representation for current viewport
        //       Each choice of pivot point determines an equivalent representation.
        //       We just find an equivalent when the pivot position is the view position, and offset is 0

        HAPI_Viewport hapiView;
        hapiView.position[0] = viewportPivotPosition.GetX() / Globals::ScaleFactorTranslation;
        hapiView.position[1] = viewportPivotPosition.GetY() / Globals::ScaleFactorTranslation;
        hapiView.position[2] = viewportPivotPosition.GetZ() / Globals::ScaleFactorTranslation;

        // Set HAPI_Offset always 0 when syncing Houdini to O3DE viewport
        hapiView.offset = 0.f;

        hapiView.rotationQuaternion[0] = -hapiQuaternion.GetX();
        hapiView.rotationQuaternion[1] = -hapiQuaternion.GetZ();
        hapiView.rotationQuaternion[2] = -hapiQuaternion.GetY();
        hapiView.rotationQuaternion[3] = hapiQuaternion.GetW();

        HAPI_SetViewport(session, &hapiView);

        // Update the Synced viewport values
        //   We need to sync both the viewport values in Houdini and O3DE when the viewport changes

        // We need to get the H_Viewport again, since it is possible the value is a different equivalence of what we set.
        HAPI_Viewport currentHapiView;
        HAPI_GetViewport(session, &currentHapiView);

        // Hapi values are in Houdini coordinate and scale
        m_syncedHoudiniViewportPivotPosition = AZ::Vector3(currentHapiView.position[0], currentHapiView.position[1], currentHapiView.position[2]);
        m_syncedHoudiniViewportQuat = AZ::Quaternion(currentHapiView.rotationQuaternion[0], currentHapiView.rotationQuaternion[1], currentHapiView.rotationQuaternion[2], currentHapiView.rotationQuaternion[3]);
        m_syncedHoudiniViewportOffset = currentHapiView.offset;

        m_syncedO3DEViewportPosition = viewportContext->GetCameraTransform().GetTranslation();
        m_syncedO3DEViewportRotation = viewportContext->GetCameraTransform().GetRotation();
        m_syncedO3DEViewportLookatPosition = viewportContext->GetCameraViewMatrix().GetTranslation();

        // When sync Houdini to O3DE, we set offset to be 0.
        // So we need to zero out offset for the next time syncing O3DE to Houdini
        m_clearOffset = true;
    }

}
