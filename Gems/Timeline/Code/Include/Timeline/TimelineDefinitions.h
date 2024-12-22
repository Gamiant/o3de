/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once


#ifndef AZ_BIT
#define AZ_BIT(x) (1u << x)
#endif

namespace
{
    //! Flags that can be set on animation node.
    enum AnimationNodeFlags
    {
        eAnimNodeFlags_Expanded             = AZ_BIT(0), //!< Deprecated, handled by sandbox now
        eAnimNodeFlags_EntitySelected       = AZ_BIT(1), //!< Set if the referenced entity is selected in the editor
        eAnimNodeFlags_CanChangeName        = AZ_BIT(2), //!< Set if this node allow changing of its name.
        eAnimNodeFlags_Disabled             = AZ_BIT(3), //!< Disable this node.
        eAnimNodeFlags_DisabledForComponent = AZ_BIT(4), //!< Disable this node for a disabled or pending entity component.
    };

    enum NodeExportTypes
    {
        eNodeExportType_Global = 0,
        eNodeExportType_Local = 1,
    };

}
