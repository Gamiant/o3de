
#include <AzCore/Serialization/SerializeContext.h>
#include "TimelineEditorSystemComponent.h"

#include <Timeline/TimelineTypeIds.h>

namespace Timeline
{
    AZ_COMPONENT_IMPL(TimelineEditorSystemComponent, "TimelineEditorSystemComponent",
        TimelineEditorSystemComponentTypeId, BaseSystemComponent);

    void TimelineEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TimelineEditorSystemComponent, TimelineSystemComponent>()
                ->Version(0);
        }
    }

    TimelineEditorSystemComponent::TimelineEditorSystemComponent() = default;

    TimelineEditorSystemComponent::~TimelineEditorSystemComponent() = default;

    void TimelineEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("TimelineEditorService"));
    }

    void TimelineEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("TimelineEditorService"));
    }

    void TimelineEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void TimelineEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void TimelineEditorSystemComponent::Activate()
    {
        TimelineSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void TimelineEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        TimelineSystemComponent::Deactivate();
    }

} // namespace Timeline
