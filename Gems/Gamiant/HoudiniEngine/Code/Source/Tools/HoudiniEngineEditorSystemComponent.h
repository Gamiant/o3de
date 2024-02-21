/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <Clients/HoudiniEngineSystemComponent.h>

#include <HoudiniSettings.h>
#include <HoudiniEngine/HoudiniEngineBus.h>

#include <AzCore/XML/rapidxml.h>
#include <ISystem.h>
#include <AzCore/Component/EntityBus.h>

#include <ActionManager/Action/ActionManagerInterface.h>
#include <ActionManager/Menu/MenuManagerInterface.h>
#include <ActionManager/ToolBar/ToolBarManagerInterface.h>

#include <AzToolsFramework/ActionManager/ActionManagerRegistrationNotificationBus.h>
#include <ActionManager/Action/ActionManagerInternalInterface.h>

namespace AzToolsFramework
{
    class ActionManagerInterface;
    class MenuManagerInterface;
    class ToolBarManagerInterface;
}

namespace HoudiniEngine
{
    class Houdini;
    class HoudiniStatusPanel;
    class HoudiniConfiguration;
    class HoudiniSessionControls;
    class HoudiniNodeSync;

    class HoudiniDigitalAssetHandler;

    static constexpr const char* houdiniToolbarIdentifier = "o3de.toolbar.editor.houdini";

    /// System component for HoudiniEngine editor
    class HoudiniEngineEditorSystemComponent
        : public AZ::Component
        , HoudiniEngineRequestBus::Handler
        , SettingsBus::Handler
        , SessionNotificationBus::Handler
        , AZ::EntitySystemBus::Handler
        , AZ::TickBus::Handler
        , AzToolsFramework::EditorEvents::Bus::Handler
        , AzToolsFramework::ActionManagerRegistrationNotificationBus::Handler
    {

    public:

        AZ_COMPONENT(HoudiniEngineEditorSystemComponent, "{EA171C26-65F3-4CBF-88A8-7198442A3ED1}");
        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType&) {}
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType&) {}

        ~HoudiniEngineEditorSystemComponent() override;

        bool IsActive() override;
        HoudiniPtr GetHoudiniEngine() override;
        void CancelProcessorThread() override;
        void CancelProcessorJob(AZ::EntityId entityToRemove) override;
        void JoinProcessorThread() override;

        AZStd::string GetHoudiniResultByCode(int code) override;

    protected:

        HoudiniPtr m_houdiniInstance;

        SessionSettings m_sessionSettings;
        SessionSettings* GetSessionSettings() override
        {
            return &m_sessionSettings;
        }

        // AzToolsFramework::EditorEvents...
        void PopulateEditorGlobalContextMenu_SliceSection(QMenu* menu, const AZ::Vector2& point, int flags) override;
        void NotifyRegisterViews() override;
        ///

        // AzToolsFramework::ActionManagerRegistrationNotificationBus...
        void OnMenuRegistrationHook() override;
        void OnToolBarRegistrationHook() override;
        void OnToolBarBindingHook() override;
        void OnActionRegistrationHook() override;
        ///

        // EntitySystemBus...
        void OnEntityActivated(const AZ::EntityId&) override;
        void OnEntityDeactivated(const AZ::EntityId&) override;

        // AZ::Component...
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ///

        // SessionNotificationBus...
        void OnSessionStatusChange(SessionRequests::ESessionStatus sessionStatus) override;
        ///


        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        struct XmlData
        {
            bool m_okay = false;
            AZStd::string m_docSrc;
            AZ::rapidxml::xml_document<> m_doc;

            XmlData(const AZStd::string& docSource)
            {
                m_docSrc = docSource;
                m_okay = m_doc.parse<0>((char*)m_docSrc.c_str());
            }
        };

        AZ::Entity* FindTerrain();
        bool FindMatchingIds(const AzToolsFramework::EntityIdList& selectedEntityList, AZ::rapidxml::xml_node<>* contextMenuItemNode, AZStd::vector<AZ::EntityId>& matchingIds);

        void AddDynamicContextMenus(QMenu* menu, const AzToolsFramework::EntityIdList& selectedEntityList);
        void AddDynamicContextMenu(QMenu* menu, const AzToolsFramework::EntityIdList& selectedEntityList, AZStd::shared_ptr<XmlData> xml, AZ::rapidxml::xml_node<>* contextMenuNode);
        AZ::Entity* CreateNewHoudiniDigitalAsset(const AZStd::string& name, const AZ::EntityId& parent);


        AzToolsFramework::ActionManagerInterface* m_actionManagerInterface = nullptr;
        AzToolsFramework::ActionManagerInternalInterface* m_actionManagerInternalInterface = nullptr;
        AzToolsFramework::MenuManagerInterface* m_menuManagerInterface = nullptr;
        AzToolsFramework::ToolBarManagerInterface* m_toolBarManagerInterface = nullptr;

        void ConfigureEditorActions();

    private:

        HoudiniStatusPanel* m_houdiniStatusPanel;
        HoudiniConfiguration* m_configuration;
        HoudiniSessionControls* m_sessionControls;
        HoudiniNodeSync* m_nodeSync;

        HoudiniDigitalAssetHandler* m_hdaHandler;

        void RegisterAction(const AZStd::string& actionIdentifier, const AZStd::string& contextIdentifier, const AZStd::string& name, const AZStd::string& description, const AZStd::string& category, const AZStd::string& iconPath, AzToolsFramework::ActionVisibility visibility, AZStd::function<void()> lambda);

    };

} // namespace HoudiniEngine
