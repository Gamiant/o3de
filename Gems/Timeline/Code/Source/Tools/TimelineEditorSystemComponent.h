
#pragma once

#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <Clients/TimelineSystemComponent.h>

namespace Timeline
{
    /// System component for Timeline editor
    class TimelineEditorSystemComponent
        : public TimelineSystemComponent
        , protected AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = TimelineSystemComponent;
    public:
        AZ_COMPONENT_DECL(TimelineEditorSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        TimelineEditorSystemComponent();
        ~TimelineEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;
    };
} // namespace Timeline
