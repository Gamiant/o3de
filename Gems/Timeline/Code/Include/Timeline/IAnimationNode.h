/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include "AnimationParamType.h"
#include "AnimationValueType.h"
#include "AnimationNodeTypes.h"
#include "TimelineDefinitions.h"

#include <AzCore/std/containers/vector.h>

class IMovieSystem;

namespace Timeline
{
    class IAnimationSequence;

    class IAnimationNode
    {
    public:
        AZ_TYPE_INFO_WITH_NAME_DECL(IAnimationNode);
        AZ_RTTI_NO_TYPE_INFO_DECL()
        AZ_CLASS_ALLOCATOR_DECL;

        static void Reflect(AZ::ReflectContext* context);

        //////////////////////////////////////////////////////////////////////////
        // Supported params.
        //////////////////////////////////////////////////////////////////////////
        enum ESupportedParamFlags
        {
            eSupportedParamFlags_MultipleTracks = 0x01, // Set if parameter can be assigned multiple tracks.
            eSupportedParamFlags_Hidden = 0x02, // Hidden from the Track View UI.
        };

        struct SParamInfo
        {
            SParamInfo();
            SParamInfo(const char* _name, AnimationParameterType _paramType, AnimationValueType _valueType, ESupportedParamFlags _flags);

            AZStd::string name; // parameter name.
            AnimationParameterType paramType; // parameter id.
            AnimationValueType valueType; // value type, defines type of track to use for animating this parameter.
            ESupportedParamFlags flags; // combination of flags from ESupportedParamFlags.
        };

        using AnimParamInfos = AZStd::vector<SParamInfo>;

        virtual ~IAnimationNode(){};

        // for intrusive_ptr support
        virtual void add_ref() = 0;
        virtual void release() = 0;

        //! Set node name.
        virtual void SetName(const char* name) = 0;

        //! Get node name.
        virtual const char* GetName() const = 0;

        // Get Type of this node.
        virtual AnimationNodeType GetType() const = 0;

        // Return Animation Sequence that owns this node.
        virtual IAnimationSequence* GetSequence() const = 0;

        // Set the Animation Sequence that owns this node.
        virtual void SetSequence(IAnimationSequence* sequence) = 0;

        // Called when sequence is activated / deactivated
        virtual void Activate(bool bActivate) = 0;

        // Set AnimNode flags.
        // @param flags One or more flags from EAnimNodeFlags.
        // @see EAnimNodeFlags
        virtual void SetFlags(int flags) = 0;

        // Get AnimNode flags.
        // @return flags set on node.
        // @see EAnimNodeFlags
        virtual int GetFlags() const = 0;

        // return true if flagsToCheck is set on the node or any of the node's parents
        virtual bool AreFlagsSetOnNodeOrAnyParent(AnimationNodeFlags flagsToCheck) const = 0;

        // AZ::Entity is bound/handled via their Id over EBuses, as opposed to directly with pointers.
        virtual void SetAzEntityId(const AZ::EntityId& id) = 0;
        virtual AZ::EntityId GetAzEntityId() const = 0;

        // Return movie system that created this node.
        virtual IMovieSystem* GetMovieSystem() const = 0;

        virtual void SetPos(float time, const AZ::Vector3& pos) = 0;
        virtual void SetRotate(float time, const AZ::Quaternion& rot) = 0;
        virtual void SetScale(float time, const AZ::Vector3& scale) = 0;

        //! Compute and return the offset which brings the current position to the given position
        virtual AZ::Vector3 GetOffsetPosition(const AZ::Vector3& position);

        //! Get current entity position.
        virtual AZ::Vector3 GetPos() = 0;
        //! Get current entity rotation.
        virtual AZ::Quaternion GetRotate() = 0;
        //! Get entity rotation at specified time.
        virtual AZ::Quaternion GetRotate(float time) = 0;
        //! Get current entity scale.
        virtual AZ::Vector3 GetScale() = 0;

        // General Set param.
        // Set float/vec3/vec4 parameter at given time.
        // @return true if parameter set, false if this parameter not exist in node.
        virtual bool SetParamValue(float time, AnimationParameterType param, float value) = 0;
        virtual bool SetParamValue(float time, AnimationParameterType param, const AZ::Vector3& value) = 0;
        virtual bool SetParamValue(float time, AnimationParameterType param, const AZ::Vector4& value) = 0;

        // Get float/vec3/vec4 parameter at given time.
        // @return true if parameter exist, false if this parameter not exist in node.
        virtual bool GetParamValue(float time, AnimationParameterType param, float& value) = 0;
        virtual bool GetParamValue(float time, AnimationParameterType param, AZ::Vector3& value) = 0;
        virtual bool GetParamValue(float time, AnimationParameterType param, AZ::Vector4& value) = 0;

        //! Evaluate animation node while not playing animation.
        virtual void StillUpdate() = 0;

        //! Evaluate animation to the given time.
        virtual void Animate(SAnimContext& ec) = 0;

        // Description:
        //      Returns number of supported parameters by this animation node (position,rotation,scale,etc..).
        // Returns:
        //      Number of supported parameters.
        virtual unsigned int GetParamCount() const = 0;

        // Description:
        //      Returns the type of a param by index
        // Arguments:
        //      nIndex - parameter index in range 0 <= nIndex < GetSupportedParamCount()
        virtual AnimationParameterType GetParamType(unsigned int nIndex) const = 0;

        // Description:
        //      Check if parameter is supported by this node.
        virtual bool IsParamValid(const AnimationParameterType& paramType) const = 0;

        // Description:
        //      Returns name of supported parameter of this animation node or NULL if not available
        // Arguments:
        //          paramType - parameter id
        virtual AZStd::string GetParamName(const AnimationParameterType& paramType) const = 0;

        // Description:
        //      Returns the params value type
        virtual AnimValueType GetParamValueType(const AnimationParameterType& paramType) const = 0;

        // Description:
        //      Returns the params value type
        virtual ESupportedParamFlags GetParamFlags(const AnimationParameterType& paramType) const = 0;

        // Called node data is re-initialized, such as when changing the entity associated with it.
        virtual void OnReset() = 0;

        //////////////////////////////////////////////////////////////////////////
        // Working with Tracks.
        //////////////////////////////////////////////////////////////////////////
        virtual int GetTrackCount() const = 0;

        // Return track assigned to the specified parameter.
        virtual IAnimTrack* GetTrackByIndex(int nIndex) const = 0;

        // Return first track assigned to the specified parameter.
        virtual IAnimTrack* GetTrackForParameter(const AnimationParameterType& paramType) const = 0;

        // Return the i-th track assigned to the specified parameter in case of multiple tracks.
        virtual IAnimTrack* GetTrackForParameter(const AnimationParameterType& paramType, uint32 index) const = 0;

        // Get the index of a given track among tracks with the same parameter type in this node.
        virtual uint32 GetTrackParamIndex(const IAnimTrack* pTrack) const = 0;

        // Creates a new track for given parameter.
        virtual IAnimTrack* CreateTrack(const AnimationParameterType& paramType) = 0;

        // Initializes track default values after de-serialization / user creation. Only called in editor.
        virtual void InitializeTrackDefaultValue(IAnimTrack* pTrack, const AnimationParameterType& paramType) = 0;

        // Assign animation track to parameter.
        // if track parameter is NULL track with parameter id param will be removed.
        virtual void SetTrack(const AnimationParameterType& paramType, IAnimTrack* track) = 0;

        // Set time range for all tracks in this sequence.
        virtual void SetTimeRange(Range timeRange) = 0;

        // Remove track from anim node.
        virtual void AddTrack(IAnimTrack* pTrack) = 0;

        // Remove track from anim node.
        virtual bool RemoveTrack(IAnimTrack* pTrack) = 0;

        // Description:
        //      Creates default set of tracks supported by this node.
        virtual void CreateDefaultTracks() = 0;

        // returns the tangent type to use for created keys. Override this if you have an animNode that you wish
        // to have tangents other than UNIFIED created for new keys.
        virtual int GetDefaultKeyTangentFlags() const
        {
            return SPLINE_KEY_TANGENT_UNIFIED;
        }

        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////
        // Callback for animation node used by editor.
        // Register notification callback with animation node.
        virtual void SetNodeOwner(IAnimNodeOwner* pOwner) = 0;
        virtual IAnimNodeOwner* GetNodeOwner() = 0;

        /**
         * O3DE_DEPRECATION_NOTICE(GHI-9326)
         * Serialization for Sequence data in Component Entity Sequences now occurs through AZ::SerializeContext and the Sequence Component
         **/
        virtual void Serialize(XmlNodeRef& xmlNode, bool bLoading, bool bLoadEmptyTracks) = 0;

        /**
         * O3DE_DEPRECATION_NOTICE(GHI-9326)
         * Serialization for Sequence data in Component Entity Sequences now occurs through AZ::SerializeContext and the Sequence Component
         **/
        virtual void SerializeAnims(XmlNodeRef& xmlNode, bool bLoading, bool bLoadEmptyTracks) = 0;

        // Sets up internal pointers post load from Sequence Component
        virtual void InitPostLoad(IAnimSequence* sequence) = 0;

        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        // Groups interface
        //////////////////////////////////////////////////////////////////////////
        virtual void SetParent(IAnimNode* pParent) = 0;
        virtual IAnimNode* GetParent() const = 0;
        virtual IAnimNode* HasDirectorAsParent() const = 0;
        //////////////////////////////////////////////////////////////////////////

        virtual void Render() = 0;

        virtual bool NeedToRender() const = 0;

        // Called from editor if dynamic params need updating
        virtual void UpdateDynamicParams() = 0;
        // </interfuscator:shuffle>

        // Used by AnimCameraNode
        virtual bool GetShakeRotation([[maybe_unused]] const float& time, [[maybe_unused]] Quat& rot)
        {
            return false;
        }

        virtual void SetCameraShakeSeed([[maybe_unused]] const uint shakeSeed){};

        // override this method to handle explicit setting of time
        virtual void TimeChanged([[maybe_unused]] float newTime){};

        // Compares all of the node's track values at the given time with the associated property value and
        //     sets a key at that time if they are different to match the latter
        // Returns the number of keys set
        virtual int SetKeysForChangedTrackValues([[maybe_unused]] float time)
        {
            return 0;
        };

        // Callbacks used when Game/Simulation mode is started and stopped in the Editor. Override if you want to handle these events
        virtual void OnStartPlayInEditor(){};
        virtual void OnStopPlayInEditor(){};

        ////////////////////////////////////////////////////////////////////////////////
        // interface for Components - implemented by CAnimComponentNode

        // Override if the derived node has an associated component type (e.g. CAnimComponentNode)
        virtual void SetComponent([[maybe_unused]] AZ::ComponentId comopnentId, [[maybe_unused]] const AZ::Uuid& componentTypeId)
        {
        }

        // returns the componentId of the component the node is associate with, if applicable, or a AZ::InvalidComponentId otherwise
        virtual AZ::ComponentId GetComponentId() const
        {
            return AZ::InvalidComponentId;
        }
        // ~interface for Components
        ////////////////////////////////////////////////////////////////////////////////

        // Used to disable any animation that is overridden by a SceneNode during camera interpolation, such as
        // FoV, transform, nearZ
        virtual void SetSkipInterpolatedCameraNode([[maybe_unused]] const bool skipNodeCameraAnimation){};

        // Expanded state interface
        virtual void SetExpanded(bool expanded) = 0;
        virtual bool GetExpanded() const = 0;

        // Return the node id. This id is unique within a given sequence.
        virtual int GetId() const = 0;

    };
}
