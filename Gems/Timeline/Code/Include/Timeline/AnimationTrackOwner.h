/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

namespace Timeline
{
    struct IAnimationNodeOwner
    {
        virtual ~IAnimationNodeOwner() = default;

        virtual void OnNodeAnimated([[maybe_unused]] IAnimNode* pNode) {}

        virtual void OnNodeVisibilityChanged(IAnimNode* pNode, const bool bHidden) = 0;

        virtual void OnNodeReset([[maybe_unused]] IAnimNode* pNode) {}

        virtual void MarkAsModified() = 0;
    };

} // namespace Timeline
