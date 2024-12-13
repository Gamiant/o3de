#pragma once

namespace Timeline
{
    //////////////////////////////////////////////////////////////////////////
    //! Node-Types
    //
    // You need to register new types in Movie.cpp/RegisterNodeTypes for serialization.
    // Note: Enums are serialized by string now, there is no need for specific IDs
    // anymore for new parameters. Values are for backward compatibility.
    enum class AnimationNodeType
    {
        Invalid = 0x00,
        Entity = 0x01, // Legacy Cry Entities (CE is AzEntity)
        Director = 0x02,
        // Camera                    = 0x03,       // Legacy Cry Camera Object
        CVar = 0x04,
        ScriptVar = 0x05,
        Material = 0x06,
        Event = 0x07,
        Group = 0x08,
        Layer = 0x09,
        Comment = 0x10,
        RadialBlur = 0x11,
        ColorCorrection = 0x12,
        DepthOfField = 0x13,
        ScreenFader = 0x14,
        Light = 0x15, // Legacy Cry Light
        // HDRSetup               = 0x16,       // deprecated Jan 2016
        ShadowSetup = 0x17,
        Alembic = 0x18, // Used in cinebox, added so nobody uses that number
        GeomCache = 0x19,
        ScreenDropsSetup, // deprecated Jan 2016
        AzEntity,
        Component,
        Num
    };

}
