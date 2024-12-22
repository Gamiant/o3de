
#include "TimelineModuleInterface.h"
#include <AzCore/Memory/Memory.h>

#include <Timeline/TimelineTypeIds.h>

#include <Clients/TimelineSystemComponent.h>

#include <Timeline/IAnimationNode.h>

namespace Timeline
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(TimelineModuleInterface,
        "TimelineModuleInterface", TimelineModuleInterfaceTypeId);
    AZ_RTTI_NO_TYPE_INFO_IMPL(TimelineModuleInterface, AZ::Module);
    AZ_CLASS_ALLOCATOR_IMPL(TimelineModuleInterface, AZ::SystemAllocator);

    TimelineModuleInterface::TimelineModuleInterface()
    {
        // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
        // Add ALL components descriptors associated with this gem to m_descriptors.
        // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
        // This happens through the [MyComponent]::Reflect() function.
        m_descriptors.insert(m_descriptors.end(), {
            TimelineSystemComponent::CreateDescriptor(),
            });
    }

    AZ::ComponentTypeList TimelineModuleInterface::GetRequiredSystemComponents() const
    {
        return AZ::ComponentTypeList{
            azrtti_typeid<TimelineSystemComponent>(),
        };
    }
} // namespace Timeline
