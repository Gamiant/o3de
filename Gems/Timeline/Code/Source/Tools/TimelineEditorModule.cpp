
#include <Timeline/TimelineTypeIds.h>
#include <TimelineModuleInterface.h>
#include "TimelineEditorSystemComponent.h"

namespace Timeline
{
    class TimelineEditorModule
        : public TimelineModuleInterface
    {
    public:
        AZ_RTTI(TimelineEditorModule, TimelineEditorModuleTypeId, TimelineModuleInterface);
        AZ_CLASS_ALLOCATOR(TimelineEditorModule, AZ::SystemAllocator);

        TimelineEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                TimelineEditorSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<TimelineEditorSystemComponent>(),
            };
        }
    };
}// namespace Timeline

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME, _Editor), Timeline::TimelineEditorModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_Timeline_Editor, Timeline::TimelineEditorModule)
#endif
