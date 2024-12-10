
#include "TimelineSystemComponent.h"

#include <Timeline/TimelineTypeIds.h>

#include <AzCore/Serialization/SerializeContext.h>

namespace Timeline
{
    AZ_COMPONENT_IMPL(TimelineSystemComponent, "TimelineSystemComponent",
        TimelineSystemComponentTypeId);

    void TimelineSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TimelineSystemComponent, AZ::Component>()
                ->Version(0)
                ;
        }
    }

    void TimelineSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("TimelineService"));
    }

    void TimelineSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("TimelineService"));
    }

    void TimelineSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void TimelineSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    TimelineSystemComponent::TimelineSystemComponent()
    {
        if (TimelineInterface::Get() == nullptr)
        {
            TimelineInterface::Register(this);
        }
    }

    TimelineSystemComponent::~TimelineSystemComponent()
    {
        if (TimelineInterface::Get() == this)
        {
            TimelineInterface::Unregister(this);
        }
    }

    void TimelineSystemComponent::Init()
    {
    }

    void TimelineSystemComponent::Activate()
    {
        TimelineRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void TimelineSystemComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        TimelineRequestBus::Handler::BusDisconnect();
    }

    void TimelineSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }

} // namespace Timeline
