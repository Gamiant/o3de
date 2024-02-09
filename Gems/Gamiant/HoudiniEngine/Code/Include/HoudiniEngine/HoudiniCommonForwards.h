/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/std/smart_ptr/shared_ptr.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

namespace HoudiniEngine
{
    class IHoudini;
    class IHoudiniAsset;
    class IHoudiniNode;
    class IHoudiniParameter;
    class IInputNodeManager;

    typedef AZStd::shared_ptr<IHoudini> HoudiniPtr;
    typedef AZStd::shared_ptr<IHoudiniAsset> HoudiniAssetPtr;
    typedef AZStd::shared_ptr<IHoudiniNode> HoudiniNodePtr;
    typedef AZStd::shared_ptr<IHoudiniParameter> HoudiniParameterPtr;
    typedef AZStd::shared_ptr<IInputNodeManager> InputNodeManagerPtr;
}
