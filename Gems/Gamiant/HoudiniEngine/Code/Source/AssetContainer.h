#pragma once

namespace HoudiniEngine
{
    class IHoudiniAsset;

    class AssetContainer
    {
    public:
        IHoudiniAsset* asset = nullptr;
    };

} // namespace HoudiniEngine

Q_DECLARE_METATYPE(HoudiniEngine::AssetContainer);
