
#pragma once

#include <Timeline/TimelineTypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace Timeline
{
    class TimelineRequests
    {
    public:
        AZ_RTTI(TimelineRequests, TimelineRequestsTypeId);
        virtual ~TimelineRequests() = default;
        // Put your public methods here
    };

    class TimelineBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using TimelineRequestBus = AZ::EBus<TimelineRequests, TimelineBusTraits>;
    using TimelineInterface = AZ::Interface<TimelineRequests>;

} // namespace Timeline
