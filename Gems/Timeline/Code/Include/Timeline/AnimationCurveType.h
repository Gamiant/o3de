#pragma once

namespace Timeline
{
    //! Types of animation track.
    // Do not change values! they are serialized
    //
    // Attention: This should only be expanded if you add a completely new way how tracks store data!
    // If you just want to control a new parameter of an entity etc. extend EParamType
    //
    // Note: TCB splines are only for backward compatibility, Bezier is the default
    //
    enum EAnimationCurveType : unsigned int
    {
        eAnimCurveType_TCBFloat = 1,
        eAnimCurveType_TCBVector = 2,
        eAnimCurveType_TCBQuat = 3,
        eAnimCurveType_BezierFloat = 4,

        eAnimCurveType_Unknown = 0xFFFFFFFF
    };

}
