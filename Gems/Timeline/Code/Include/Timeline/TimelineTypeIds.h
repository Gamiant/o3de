
#pragma once

namespace Timeline
{
    // System Component TypeIds
    inline constexpr const char* TimelineSystemComponentTypeId = "{09357508-5258-4042-8FC1-8BAB0E2DBFE7}";
    inline constexpr const char* TimelineEditorSystemComponentTypeId = "{F13ED67B-B0E9-4886-A6C5-4DD11E1E60E5}";

    // Module derived classes TypeIds
    inline constexpr const char* TimelineModuleInterfaceTypeId = "{ED0B3FC3-A48D-487D-915A-D275ABC5F5D3}";
    inline constexpr const char* TimelineModuleTypeId = "{7C15B1BA-D8C8-46D9-BD71-9489540BDD9D}";
    // The Editor Module by default is mutually exclusive with the Client Module
    // so they use the Same TypeId
    inline constexpr const char* TimelineEditorModuleTypeId = TimelineModuleTypeId;

    // Interface TypeIds
    inline constexpr const char* TimelineRequestsTypeId = "{B47E9C45-6983-4516-820A-6285A662094C}";
} // namespace Timeline
