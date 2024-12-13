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
    class IAnimationTrack
    {
        AZ_TYPE_INFO_WITH_NAME_DECL(IAnimationTrack);
        AZ_RTTI_NO_TYPE_INFO_DECL();
        AZ_CLASS_ALLOCATOR_DECL

        static void Reflect(AZ::ReflectContext* context);

        //! Flags that can be set on animation track.
        enum EAnimTrackFlags
        {
            eAnimTrackFlags_Linear = BIT(1), //!< Use only linear interpolation between keys.
            eAnimTrackFlags_Loop = BIT(2), //!< Play this track in a loop.
            eAnimTrackFlags_Cycle = BIT(3), //!< Cycle track.
            eAnimTrackFlags_Disabled = BIT(4), //!< Disable this track.

            //////////////////////////////////////////////////////////////////////////
            // Used by editor.
            //////////////////////////////////////////////////////////////////////////
            eAnimTrackFlags_Hidden = BIT(5), //!< Set when track is hidden in track view.
            eAnimTrackFlags_Muted =
                BIT(8), //!< Mute this sound track or music track. This only affects the playback in editor on these types of tracks.
        };

        virtual ~IAnimationTrack(){};

        // for intrusive_ptr support
        virtual void add_ref() = 0;
        virtual void release() = 0;

        //////////////////////////////////////////////////////////////////////////
        virtual EAnimCurveType GetCurveType() = 0;
        virtual AnimValueType GetValueType() = 0;

#ifdef MOVIESYSTEM_SUPPORT_EDITING
        // This color is used for the editor.
        virtual ColorB GetCustomColor() const = 0;
        virtual void SetCustomColor(ColorB color) = 0;
        virtual bool HasCustomColor() const = 0;
        virtual void ClearCustomColor() = 0;
#endif

        // Return what parameter of the node, this track is attached to.
        virtual const CAnimParamType& GetParameterType() const = 0;

        // Assign node parameter ID for this track.
        virtual void SetParameterType(CAnimParamType type) = 0;

        virtual void SetNode(IAnimNode* node) = 0;
        // Return Animation Sequence that owns this node.
        virtual IAnimNode* GetNode() = 0;

        //////////////////////////////////////////////////////////////////////////
        // Animation track can contain sub-tracks (Position XYZ anim track have sub-tracks for x,y,z)
        // Get count of sub tracks.
        virtual int GetSubTrackCount() const = 0;
        // Retrieve pointer the specfied sub track.
        virtual IAnimationTrack* GetSubTrack(int nIndex) const = 0;
        virtual AZStd::string GetSubTrackName(int nIndex) const = 0;
        virtual void SetSubTrackName(int nIndex, const char* name) = 0;
        //////////////////////////////////////////////////////////////////////////

        virtual void GetKeyValueRange(float& fMin, float& fMax) const = 0;
        virtual void SetKeyValueRange(float fMin, float fMax) = 0;

        //! Return number of keys in track.
        virtual int GetNumKeys() const = 0;

        //! Return true if keys exists in this track
        virtual bool HasKeys() const = 0;

        //! Set number of keys in track.
        //! If needed adds empty keys at end or remove keys from end.
        virtual void SetNumKeys(int numKeys) = 0;

        //! Remove specified key.
        virtual void RemoveKey(int num) = 0;

        //! Get key at specified location.
        //! @param key Must be valid pointer to compatible key structure, to be filled with specified key location.
        virtual void GetKey(int index, IKey* key) const = 0;

        //! Get time of specified key.
        //! @return key time.
        virtual float GetKeyTime(int index) const = 0;

        //! Find key at given time.
        //! @return Index of found key, or -1 if key with this time not found.
        virtual int FindKey(float time) = 0;

        //! Get flags of specified key.
        //! @return key time.
        virtual int GetKeyFlags(int index) = 0;

        //! Set key at specified location.
        //! @param key Must be valid pointer to compatible key structure.
        virtual void SetKey(int index, IKey* key) = 0;

        //! Set time of specified key.
        virtual void SetKeyTime(int index, float time) = 0;

        //! Set flags of specified key.
        virtual void SetKeyFlags(int index, int flags) = 0;

        //! Sort keys in track (after time of keys was modified).
        virtual void SortKeys() = 0;

        //! Get track flags.
        virtual int GetFlags() = 0;

        //! Check if track is masked by mask
        // TODO: Mask should be stored with dynamic length
        virtual bool IsMasked(const uint32 mask) const = 0;

        //! Set track flags.
        virtual void SetFlags(int flags) = 0;

        //! Create key at given time, and return its index.
        //! @return Index of new key.
        virtual int CreateKey(float time) = 0;

        //! Clone key at specified index.
        //! @retun Index of new key.
        virtual int CloneKey(int key) = 0;

        //! Clone key at specified index from another track of SAME TYPE.
        //! @retun Index of new key.
        virtual int CopyKey(IAnimationTrack* pFromTrack, int nFromKey) = 0;

        //! Get info about specified key.
        //! @param Short human readable text description of this key.
        //! @param duration of this key in seconds.
        virtual void GetKeyInfo(int key, const char*& description, float& duration) = 0;

        //////////////////////////////////////////////////////////////////////////
        // Get track value at specified time.
        // Interpolates keys if needed.
        // Applies a scale multiplier set in SetMultiplier(), if requested
        //////////////////////////////////////////////////////////////////////////
        virtual void GetValue(float time, float& value, bool applyMultiplier = false) = 0;
        virtual void GetValue(float time, AZ::Vector3& value, bool applyMultiplier = false) = 0;
        virtual void GetValue(float time, AZ::Vector4& value, bool applyMultiplier = false) = 0;
        virtual void GetValue(float time, AZ::Quaternion& value) = 0;
        virtual void GetValue(float time, bool& value) = 0;
        virtual void GetValue(float time, Maestro::AssetBlends<AZ::Data::AssetData>& value) = 0;

        //////////////////////////////////////////////////////////////////////////
        // Set track value at specified time.
        // Adds new keys if required.
        //////////////////////////////////////////////////////////////////////////
        virtual void SetValue(float time, const float& value, bool bDefault = false, bool applyMultiplier = false) = 0;
        virtual void SetValue(float time, const AZ::Vector3& value, bool bDefault = false, bool applyMultiplier = false) = 0;
        virtual void SetValue(float time, const AZ::Vector4& value, bool bDefault = false, bool applyMultiplier = false) = 0;
        virtual void SetValue(float time, const AZ::Quaternion& value, bool bDefault = false) = 0;
        virtual void SetValue(float time, const bool& value, bool bDefault = false) = 0;
        virtual void SetValue(float time, const Maestro::AssetBlends<AZ::Data::AssetData>& value, bool bDefault = false) = 0;

        // Only for position tracks, offset all track keys by this amount.
        virtual void OffsetKeyPosition(const AZ::Vector3& value) = 0;

        // Used to update the data in tracks after the parent entity has been changed.
        virtual void UpdateKeyDataAfterParentChanged(const AZ::Transform& oldParentWorldTM, const AZ::Transform& newParentWorldTM) = 0;

        // Assign active time range for this track.
        virtual void SetTimeRange(const Range& timeRange) = 0;

        //! @deprecated - IAnimationTracks use AZ::Serialization now. Legacy - Serialize this animation track to XML.
        virtual bool Serialize(XmlNodeRef& xmlNode, bool bLoading, bool bLoadEmptyTracks = true) = 0;
        virtual bool SerializeSelection(XmlNodeRef& xmlNode, bool bLoading, bool bCopySelected = false, float fTimeOffset = 0) = 0;

        virtual void InitPostLoad(IAnimSequence* /*sequence*/){};

        //! For custom track animate parameters.
        virtual void Animate([[maybe_unused]] SAnimContext& ec){};

        // Get access to the internal spline of the track.
        virtual ISplineInterpolator* GetSpline() const
        {
            return 0;
        };

        virtual bool IsKeySelected([[maybe_unused]] int key) const
        {
            return false;
        }

        virtual void SelectKey([[maybe_unused]] int key, [[maybe_unused]] bool select)
        {
        }

        virtual void SetSortMarkerKey([[maybe_unused]] unsigned int keyIndex, [[maybe_unused]] bool enabled)
        {
        }
        virtual bool IsSortMarkerKey([[maybe_unused]] unsigned int keyIndex) const
        {
            return false;
        }

        //! Return the index of the key which lies right after the given key in time.
        //! @param key Index of of key.
        //! @return Index of the next key in time. If the last key given, this returns -1.
        // In case of keys sorted, it's just 'key+1', but if not sorted, it can be another value.
        virtual int NextKeyByTime(int key) const;

        //! Get the animation layer index assigned. (only for character/look-at tracks ATM)
        virtual int GetAnimationLayerIndex() const
        {
            return -1;
        }
        //! Set the animation layer index. (only for character/look-at tracks ATM)
        virtual void SetAnimationLayerIndex([[maybe_unused]] int index)
        {
        }

        //! Returns whether the track responds to muting (false by default), which only affects the Edtior.
        //! Tracks that use mute should override this, such as CSoundTrack
        //! @return Boolean of whether the track respnnds to muting or not
        virtual bool UsesMute() const
        {
            return false;
        }

        //! Set a multiplier which will be multiplied to track values in SetValue and divided out in GetValue if requested
        virtual void SetMultiplier(float trackValueMultiplier) = 0;

        // Expanded state interface
        virtual void SetExpanded(bool expanded) = 0;
        virtual bool GetExpanded() const = 0;

        virtual unsigned int GetId() const = 0;
        virtual void SetId(unsigned int id) = 0;

    };
}
