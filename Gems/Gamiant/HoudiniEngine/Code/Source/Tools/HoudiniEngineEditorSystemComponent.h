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

#include <HoudiniEngine/HoudiniEngineBus.h>

#include <AzCore/XML/rapidxml.h>
#include <ISystem.h>
#include <AzCore/Component/EntityBus.h>

#include <ActionManager/Action/ActionManagerInterface.h>
#include <ActionManager/Menu/MenuManagerInterface.h>
#include <ActionManager/ToolBar/ToolBarManagerInterface.h>
#include <AzToolsFramework/ActionManager/ActionManagerRegistrationNotificationBus.h>

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

    /// System component for HoudiniEngine editor
    class HoudiniEngineEditorSystemComponent
        : public AZ::Component
        //, protected AzToolsFramework::EditorEvents::Bus::Handler
        , protected HoudiniEngineRequestBus::Handler
        , public CrySystemEventBus::Handler
        , public ISystemEventListener
        , public AZ::TickBus::Handler
        , public AzToolsFramework::EditorEvents::Bus::Handler
        , public AZ::EntitySystemBus::Handler
        , AzToolsFramework::ActionManagerRegistrationNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(HoudiniEngineEditorSystemComponent, "{EA171C26-65F3-4CBF-88A8-7198442A3ED1}");
        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        bool IsActive() override;
        HoudiniPtr GetHoudiniEngine() override;
        void CancelProcessorThread() override;
        void CancelProcessorJob(AZ::EntityId entityToRemove) override;
        void JoinProcessorThread() override;
        virtual AZStd::string GetHoudiniResultByCode(int code) override;  // FL[FD-10714] Houdini integration to 1.21

        HoudiniPtr m_houdiniInstance;

        HoudiniStatusPanel* m_houdiniStatusPanel;

        HoudiniConfiguration* m_configuration;
        HoudiniSessionControls* m_sessionControls;

    protected:
        ////////////////////////////////////////////////////////////////////////
        // HoudiniEngineRequestBus interface implementation        
        ////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////
        // AzToolsFramework::EditorEvents
        void PopulateEditorGlobalContextMenu_SliceSection(QMenu* menu, const AZ::Vector2& point, int flags) override;
        void NotifyRegisterViews() override;
        //////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////
        // AzToolsFramework::ActionManagerRegistrationNotificationBus...
        void OnMenuRegistrationHook() override;
        //////////////////////////////////////////////////////////////////////////

        // Entity System Bus:
        void OnEntityActivated(const AZ::EntityId&) override;
        void OnEntityDeactivated(const AZ::EntityId&) override;

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////

        bool LoadHoudiniEngine();
        void OnSystemEvent(ESystemEvent /*event*/, UINT_PTR /*wparam*/, UINT_PTR /*lparam*/) override;
        void OnCrySystemInitialized(ISystem& system, const SSystemInitParams& systemInitParams) override;
        void OnCrySystemShutdown(ISystem&) override;

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
        AzToolsFramework::MenuManagerInterface* m_menuManagerInterface = nullptr;
        AzToolsFramework::ToolBarManagerInterface* m_toolBarManagerInterface = nullptr;

        void ConfigureEditorActions();

    };
} // namespace HoudiniEngine
