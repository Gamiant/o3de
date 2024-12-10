
#include <Timeline/TimelineTypeIds.h>
#include <TimelineModuleInterface.h>
#include "TimelineSystemComponent.h"

namespace Timeline
{
    class TimelineModule
        : public TimelineModuleInterface
    {
    public:
        AZ_RTTI(TimelineModule, TimelineModuleTypeId, TimelineModuleInterface);
        AZ_CLASS_ALLOCATOR(TimelineModule, AZ::SystemAllocator);
    };
}// namespace Timeline

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), Timeline::TimelineModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_Timeline, Timeline::TimelineModule)
#endif
