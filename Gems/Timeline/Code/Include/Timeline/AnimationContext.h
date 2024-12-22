#pragma once

namespace Timeline
{
    class IAnimationSequence;

    //! Structure passed to Animate function.
    struct AnimationContext
    {
        AnimationContext();

        float time;             //!< Current time in seconds.
        float dt;               //!< Delta of time from previous animation frame in seconds.
        float fps;              //!< Last calculated frames per second value.
        bool singleFrame;       //!< This is not a playing animation, more a single-frame update.
        bool forcePlay;         //!< Set when force playing animation.
        bool resetting;         //!< Set when animation sequence is resetting.

        IAnimationSequence* sequence; //!< Sequence in which animation performed.

        // TODO: Replace trackMask with something more type safe
        // TODO: Mask should be stored with dynamic length
        uint32 trackMask;       //!< To update certain types of tracks only
        float startTime;        //!< The start time of this playing sequence
    };
}
