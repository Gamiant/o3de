
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
    // Common parameters of animation node.
    class AnimationParameterType
    {
        //friend class CMovieSystem;
        //friend class AnimSerializer;

    public:
        AZ_TYPE_INFO_WITH_NAME_DECL(AnimationParameterType);

        static const uint kParamTypeVersion = 9;

        AnimationParameterType();

        AnimationParameterType(const AZStd::string& name);

        AnimationParameterType(AnimParamType type);

        // Convert from old enum or int
        void operator=(AnimParamType type);

        void operator=(const AZStd::string& name);

        // Convert to enum. This needs to be explicit,
        // otherwise operator== will be ambiguous
        constexpr AnimParamType GetType() const
        {
            return m_type;
        }

        // Get name
        const char* GetName() const
        {
            return m_name.c_str();
        }

        bool operator==(const AnimationParameterType& animParamType) const;

        bool operator!=(const AnimationParameterType& animParamType) const;

        bool operator<(const AnimationParameterType& animParamType) const;

        operator size_t() const
        {
            AZStd::hash<AnimParamType> paramTypeHasher;
            size_t retVal = paramTypeHasher(GetType());
            AZStd::hash_combine(retVal, AZ::Crc32(GetName()));
            return retVal;
        }

        void SaveToXml(XmlNodeRef& xmlNode) const;
        void LoadFromXml(const XmlNodeRef& xmlNode, const uint version = kParamTypeVersion);

        // Serialization
        void Serialize(XmlNodeRef& xmlNode, bool bLoading, const uint version = kParamTypeVersion);

    private:
        AnimationParameterType m_type;
        AZStd::string m_name;
    };

}
