/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/RTTI/RTTI.h>
#include "TimelineDefinitions.h"

namespace AZ
{
    class ReflectContext;
}

namespace Timeline
{
    class IAnimationNode;

    //! Animation sequence, operates on animation nodes contained in it
    class IAnimationSequence
    {
        AZ_TYPE_INFO_WITH_NAME_DECL(IAnimationSequence);
        AZ_RTTI_NO_TYPE_INFO_DECL();
        AZ_CLASS_ALLOCATOR_DECL;

        static const int kSequenceVersion = 5;

        static void Reflect(AZ::ReflectContext* context);

        //! Flags used for SetFlags(),GetFlags(),SetParentFlags(),GetParentFlags() methods.
        enum EAnimSequenceFlags
        {
            eSeqFlags_PlayOnReset               = AZ_BIT(0), //!< Start playing this sequence immediately after reset of movie system(Level load).
            eSeqFlags_OutOfRangeConstant        = AZ_BIT(1), //!< Constant Out-Of-Range,time continues normally past sequence time range.
            eSeqFlags_OutOfRangeLoop            = AZ_BIT(2), //!< Loop Out-Of-Range,time wraps back to the start of range when reaching end of range.
            eSeqFlags_CutScene                  = AZ_BIT(3), //!< Cut scene sequence.
            eSeqFlags_NoHUD                     = AZ_BIT(4), //!< @deprecated - Don`t display HUD
            eSeqFlags_NoPlayer                  = AZ_BIT(5), //!< Disable input and drawing of player
            eSeqFlags_NoGameSounds              = AZ_BIT(9), //!< Suppress all game sounds.
            eSeqFlags_NoSeek                    = AZ_BIT(10), //!< Cannot seek in sequence.
            eSeqFlags_NoAbort                   = AZ_BIT(11), //!< Cutscene can not be aborted
            eSeqFlags_NoSpeed                   = AZ_BIT(13), //!< Cannot modify sequence speed - TODO: add interface control if required
            // eSeqFlags_CanWarpInFixedTime     = AZ_BIT(14), //!< @deprecated - Timewarp by scaling a fixed time step - removed July 2017, unused
            eSeqFlags_EarlyMovieUpdate          = AZ_BIT(15), //!< Turn the 'sys_earlyMovieUpdate' on during the sequence.
            eSeqFlags_LightAnimationSet         = AZ_BIT(16), //!< A special unique sequence for light animations
            eSeqFlags_NoMPSyncingNeeded         = AZ_BIT(17), //!< this sequence doesn't require MP net syncing
            eSeqFlags_DisplayAsFramesOrSeconds  = AZ_BIT(18), //!< Display Start/End time as frames or seconds
        };

        virtual ~IAnimationSequence(){};

        // for intrusive_ptr support
        friend struct AZStd::IntrusivePtrCountPolicy<IAnimationSequence>;

        virtual void add_ref() = 0;
        virtual void release() = 0;

        //! Set the name of this sequence. (ex. "Intro" in the same case as above)
        virtual void SetName(const char* name) = 0;
        //! Get the name of this sequence. (ex. "Intro" in the same case as above)
        virtual const char* GetName() const = 0;
        //! Get the ID (unique in a level and consistent across renaming) of this sequence.
        virtual uint32 GetId() const = 0;
        //! Resets the ID to the next available ID - used on sequence loads into levels to resolve ID collisions
        virtual void ResetId() = 0;

        // Legacy sequence objects are connected by pointers. SequenceComponents are connected by AZ::EntityId
        virtual void SetLegacySequenceObject(IAnimLegacySequenceObject* legacySequenceObject) = 0;
        virtual IAnimLegacySequenceObject* GetLegacySequenceObject() const = 0;
        virtual void SetSequenceEntityId(const AZ::EntityId& entityOwnerId) = 0;
        virtual const AZ::EntityId& GetSequenceEntityId() const = 0;

        //! Set the currently active director node.
        virtual void SetActiveDirector(IAnimationNode* pDirectorNode) = 0;
        //! Get the currently active director node, if any.
        virtual IAnimationNode* GetActiveDirector() const = 0;

        //! Set animation sequence flags
        virtual void SetFlags(int flags) = 0;
        //! Get animation sequence flags
        virtual int GetFlags() const = 0;
        //! Get cutscene related animation sequence flags
        virtual int GetCutSceneFlags(const bool localFlags = false) const = 0;
        //! Set parent animation sequence
        virtual void SetParentSequence(IAnimationSequence* pParentSequence) = 0;
        //! Get parent animation sequence
        virtual const IAnimationSequence* GetParentSequence() const = 0;
        //! Check whether this sequence has the given sequence as a descendant through one of its sequence tracks.
        virtual bool IsAncestorOf(const IAnimationSequence* sequence) const = 0;

        //! Return number of animation nodes in sequence.
        virtual int GetNodeCount() const = 0;
        //! Get animation node at specified index.
        virtual IAnimationNode* GetNode(int index) const = 0;

        //! Add animation node to sequence.
        //! @return True if node added, same node will not be added 2 times.
        virtual bool AddNode(IAnimationNode* node) = 0;

        // Reorders the array of nodes, so the specified node is placed after or before the given pivot node depending on the parameter
        // 'next'.
        virtual void ReorderNode(IAnimationNode* node, IAnimationNode* pPivotNode, bool next) = 0;

        // Description:
        //      Creates a new animation node with specified type.
        // Arguments:
        //      nodeType - Type of node, one of AnimNodeType enums.
        virtual IAnimationNode* CreateNode(AnimationNodeType nodeType) = 0;

        // Description:
        //      Creates a new animation node from serialized node XML.
        // Arguments:
        //      node - Serialized form of node
        virtual IAnimationNode* CreateNode(XmlNodeRef node) = 0;

        //! Remove animation node from sequence.
        virtual void RemoveNode(IAnimationNode* node, bool removeChildRelationships /*=true*/) = 0;

        // Finds node by name, can be slow.
        // If the node belongs to a director, the director node also should be given
        // since there can be multiple instances of the same node(i.e. the same name)
        // across multiple director nodes.
        virtual IAnimationNode* FindNodeByName(const char* sNodeName, const IAnimationNode* pParentDirector) = 0;

        //! Remove all nodes from sequence.
        virtual void RemoveAll() = 0;

        // Activate sequence by binding sequence animations to nodes.
        // must be called prior animating sequence.
        virtual void Activate() = 0;

        /** Check if sequence is activated
         */
        virtual bool IsActivated() const = 0;

        // Deactivates sequence by unbinding sequence animations from nodes.
        virtual void Deactivate() = 0;

        // Pre-caches data associated with this anim sequence.
        virtual void PrecacheData(float startTime = 0.0f) = 0;

        // Update sequence while not playing animation.
        virtual void StillUpdate() = 0;

        // Render function call for some special node.
        virtual void Render() = 0;

        // Evaluate animations of all nodes in sequence.
        // Sequence must be activated before animating.
        virtual void Animate(const SAnimContext& ec) = 0;

        //! Set time range of this sequence.
        virtual void SetTimeRange(Range timeRange) = 0;

        //! Get time range of this sequence.
        virtual Range GetTimeRange() = 0;

        //! Resets the sequence
        virtual void Reset(bool bSeekToStart) = 0;

        //! This can have more time-consuming tasks performed additional to tasks of the usual 'Reset()' method.
        virtual void ResetHard() = 0;

        // Called to pause sequence.
        virtual void Pause() = 0;

        // Called to resume sequence.
        virtual void Resume() = 0;

        /** Called to check if sequence is paused.
         */
        virtual bool IsPaused() const = 0;

        /** Called when a sequence is looped.
         */
        virtual void OnLoop() = 0;

        /** Move/Scale all keys in tracks from previous time range to new time range.
         */
        virtual void AdjustKeysToTimeRange(const Range& timeRange) = 0;

        /** Called when time was explicitly jumped to/set.
         */
        virtual void TimeChanged(float newTime) = 0;

        // fix up internal pointers after load from Sequence Component
        virtual void InitPostLoad() = 0;

        // Copy some nodes of this sequence to XML.
        virtual void CopyNodes(XmlNodeRef& xmlNode, IAnimationNode** pSelectedNodes, uint32 count) = 0;

        // Paste nodes given by the XML to this sequence.
        virtual void PasteNodes(const XmlNodeRef& xmlNode, IAnimationNode* pParent) = 0;

        // Summary:
        //  Adds/removes track events in sequence.
        virtual bool AddTrackEvent(const char* szEvent) = 0;
        virtual bool RemoveTrackEvent(const char* szEvent) = 0;
        virtual bool RenameTrackEvent(const char* szEvent, const char* szNewEvent) = 0;
        virtual bool MoveUpTrackEvent(const char* szEvent) = 0;
        virtual bool MoveDownTrackEvent(const char* szEvent) = 0;
        virtual void ClearTrackEvents() = 0;

        // Summary:
        //  Gets the track events in the sequence.
        virtual int GetTrackEventsCount() const = 0;
        // Summary:
        //  Gets the specified track event in the sequence.
        virtual char const* GetTrackEvent(int iIndex) const = 0;

        virtual IAnimStringTable* GetTrackEventStringTable() = 0;

        // Summary:
        //   Called to trigger a track event.
        virtual void TriggerTrackEvent(const char* event, const char* param = NULL) = 0;

        //! Track event listener
        virtual void AddTrackEventListener(ITrackEventListener* pListener) = 0;
        virtual void RemoveTrackEventListener(ITrackEventListener* pListener) = 0;

        // return the sequence type - legacy or new component entity
        virtual SequenceType GetSequenceType() const = 0;

        // Expanded state interface
        virtual void SetExpanded(bool expanded) = 0;
        virtual bool GetExpanded() const = 0;

        virtual unsigned int GetUniqueTrackIdAndGenerateNext() = 0;

        // </interfuscator:shuffle>
    };

}
