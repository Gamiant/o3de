/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

#include <AzToolsFramework/API/EntityCompositionRequestBus.h>
#include <AzToolsFramework/API/ViewPaneOptions.h>
#include <AzToolsFramework/Editor/ActionManagerIdentifiers/EditorContextIdentifiers.h>
#include <AzToolsFramework/Editor/ActionManagerIdentifiers/EditorMenuIdentifiers.h>
#include <AzToolsFramework/Entity/EditorEntityContextBus.h>

#include "HoudiniCommon.h"
#include "HoudiniEngineEditorSystemComponent.h"
#include "HoudiniPaintAttribTool.h"
#include "HoudiniPropertyHandlers.h"

#include <UI/HoudiniStatusPanel.h>
#include <UI/HoudiniConfiguration.h>
#include <UI/HoudiniSessionControls.h>
#include <UI/Widgets/ViewportSyncWidget.h>

#if defined(AZ_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <QFile>
#include <QTextStream>

#include "GameEngine.h"
#include <QtViewPaneManager.h>
#include <Editor/ActionManagerIdentifiers/EditorToolBarIdentifiers.h>

namespace HoudiniEngine
{

    void HoudiniEngineEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<HoudiniEngineEditorSystemComponent, AZ::Component>()
                ->Version(0);

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<HoudiniEngineEditorSystemComponent>("HoudiniEngine", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System", 0xc94d118b))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    void HoudiniEngineEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("HoudiniEngineService"));
    }

    void HoudiniEngineEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("HoudiniEngineService"));
    }

    void HoudiniEngineEditorSystemComponent::Init()
    {
        SettingsBus::Handler::BusConnect();
        HoudiniEngineRequestBus::Handler::BusConnect();

        m_houdiniInstance = HoudiniPtr(new Houdini());
    }

    HoudiniEngineEditorSystemComponent::~HoudiniEngineEditorSystemComponent()
    {
        m_houdiniInstance->Shutdown();

        delete m_houdiniStatusPanel;
        delete m_configuration;
        delete m_sessionControls;
    }

    void HoudiniEngineEditorSystemComponent::Activate()
    {
        AzToolsFramework::ActionManagerRegistrationNotificationBus::Handler::BusConnect();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
        AZ::EntitySystemBus::Handler::BusConnect();

        //TODO: maybe call this in a system or manager class if we have one later.
        PropertyFolderListHandler::Register();

        m_actionManagerInterface = AZ::Interface<AzToolsFramework::ActionManagerInterface>::Get();
        m_menuManagerInterface = AZ::Interface<AzToolsFramework::MenuManagerInterface>::Get();
        m_toolBarManagerInterface = AZ::Interface<AzToolsFramework::ToolBarManagerInterface>::Get();
        m_actionManagerInternalInterface = AZ::Interface<AzToolsFramework::ActionManagerInternalInterface>::Get();
    }

    void HoudiniEngineEditorSystemComponent::Deactivate()
    {
        SettingsBus::Handler::BusDisconnect();
        AzToolsFramework::ActionManagerRegistrationNotificationBus::Handler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
        HoudiniEngineRequestBus::Handler::BusDisconnect();
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        AZ::EntitySystemBus::Handler::BusDisconnect();
        m_houdiniInstance.reset();
    }

    void HoudiniEngineEditorSystemComponent::OnSessionStatusChange(SessionSettings::ESessionStatus sessionStatus)
    {
        m_actionManagerInternalInterface->GetAction("o3de.houdini.session.open")->setEnabled(sessionStatus == SessionSettings::ESessionStatus::Offline);
        m_actionManagerInternalInterface->GetAction("o3de.houdini.session.start")->setEnabled(sessionStatus == SessionSettings::ESessionStatus::Offline);
        m_actionManagerInternalInterface->GetAction("o3de.houdini.session.stop")->setEnabled(sessionStatus == SessionSettings::ESessionStatus::Ready);
        m_actionManagerInternalInterface->GetAction("o3de.houdini.session.restart")->setEnabled(sessionStatus == SessionSettings::ESessionStatus::Ready);
    }

    void HoudiniEngineEditorSystemComponent::ConfigureEditorActions()
    {
        static constexpr const char* houdiniMenuIdentifier = "o3de.menu.editor.houdini";

        static constexpr const char* houdiniStatusActionIdentifier = "o3de.menu.editor.houdini.status";
        static constexpr const char* houdiniConfigurationActionIdentifier = "o3de.menu.editor.houdini.config";
        static constexpr const char* houdiniSessionActionIdentifier = "o3de.menu.editor.houdini.session";

        AzToolsFramework::MenuProperties menuProperties;
        menuProperties.m_name = "Houdini";
        auto outcome = m_menuManagerInterface->RegisterMenu(houdiniMenuIdentifier, menuProperties);
        AZ_Assert(outcome.IsSuccess(), "Failed to register '%s' Menu", houdiniMenuIdentifier);


        AzToolsFramework::ActionProperties actionProperties;
        actionProperties.m_name = "Configuration";
        actionProperties.m_description = "Configure Houdini";
        actionProperties.m_menuVisibility = AzToolsFramework::ActionVisibility::AlwaysShow;

        m_actionManagerInterface->RegisterAction(
            EditorIdentifiers::MainWindowActionContextIdentifier,
            houdiniConfigurationActionIdentifier,
            actionProperties,
            []
            {
                AzToolsFramework::EditorRequestBus::Broadcast(&AzToolsFramework::EditorRequestBus::Events::OpenViewPane, "HoudiniConfiguration");
            }
        );

        outcome = m_menuManagerInterface->AddActionToMenu(houdiniMenuIdentifier, houdiniConfigurationActionIdentifier, 0);
        AZ_Assert(outcome.IsSuccess(), "Failed to register '%s' Menu", houdiniConfigurationActionIdentifier);

        actionProperties.m_name = "Session Controls";
        actionProperties.m_description = "Houdini Session Controls";
        actionProperties.m_menuVisibility = AzToolsFramework::ActionVisibility::AlwaysShow;

        m_actionManagerInterface->RegisterAction(
            EditorIdentifiers::MainWindowActionContextIdentifier,
            houdiniSessionActionIdentifier,
            actionProperties,
            []
            {
                AzToolsFramework::EditorRequestBus::Broadcast(&AzToolsFramework::EditorRequestBus::Events::OpenViewPane, "HoudiniSessionControls");
            }
        );


        outcome = m_menuManagerInterface->AddActionToMenu(houdiniMenuIdentifier, houdiniSessionActionIdentifier, 100);
        AZ_Assert(outcome.IsSuccess(), "Failed to register '%s' Menu", houdiniSessionActionIdentifier);

        actionProperties.m_name = "Status";
        actionProperties.m_description = "Houdini Status Panel";
        actionProperties.m_menuVisibility = AzToolsFramework::ActionVisibility::AlwaysShow;

        m_actionManagerInterface->RegisterAction(
            EditorIdentifiers::MainWindowActionContextIdentifier,
            houdiniStatusActionIdentifier,
            actionProperties,
            []
            {
                AzToolsFramework::EditorRequestBus::Broadcast(&AzToolsFramework::EditorRequestBus::Events::OpenViewPane, "HoudiniStatusPanel");
            }
        );


        outcome = m_menuManagerInterface->AddActionToMenu(houdiniMenuIdentifier, houdiniStatusActionIdentifier, 200);
        AZ_Assert(outcome.IsSuccess(), "Failed to register '%s' Menu", houdiniStatusActionIdentifier);

        m_menuManagerInterface->AddMenuToMenuBar(EditorIdentifiers::EditorMainWindowMenuBarIdentifier, houdiniMenuIdentifier, 560);

    }

    void HoudiniEngineEditorSystemComponent::OnMenuRegistrationHook()
    {
        ConfigureEditorActions();
    }

    void HoudiniEngineEditorSystemComponent::OnToolBarRegistrationHook()
    {
        // Setup Toolbar
        AzToolsFramework::ToolBarProperties toolBarProperties;
        toolBarProperties.m_name = "Houdini ToolBar";
        m_toolBarManagerInterface->RegisterToolBar(houdiniToolbarIdentifier, toolBarProperties);
    }

    void HoudiniEngineEditorSystemComponent::RegisterAction(const AZStd::string& actionIdentifier, const AZStd::string& contextIdentifier, const AZStd::string& name, const AZStd::string& description, const AZStd::string& category, const AZStd::string& iconPath, AzToolsFramework::ActionVisibility visibility, AZStd::function<void()> lambda)
    {
        AzToolsFramework::ActionProperties actionProperties;
        actionProperties.m_name = name;
        actionProperties.m_description = description;
        actionProperties.m_category = category;
        actionProperties.m_iconPath = iconPath;
        actionProperties.m_menuVisibility = visibility;

        m_actionManagerInterface->RegisterAction(
            contextIdentifier,
            actionIdentifier,
            actionProperties,
            lambda
        );
    }

    void HoudiniEngineEditorSystemComponent::OnActionRegistrationHook()
    {
        {
            AzToolsFramework::WidgetActionProperties widgetActionProperties;
            widgetActionProperties.m_name = "Houdini Engine";
            widgetActionProperties.m_category = "Game";

            m_actionManagerInterface->RegisterWidgetAction(
                "o3de.houdini.houdiniEngineLabel",
                widgetActionProperties,
                [&]
                {
                    QLabel* label = new QLabel();
                    label->setText("Houdini Engine");
                    return label;
                }
            );

            RegisterAction("o3de.houdini.nodesync.send",
                EditorIdentifiers::MainWindowActionContextIdentifier,
                "Send",
                "Send selection to Houdini",
                "Game",
                ":/houdini/load.svg",
                AzToolsFramework::ActionVisibility::AlwaysShow,
                [] {
                    // EBus to send node sync
                });

            RegisterAction("o3de.houdini.session.open",
                EditorIdentifiers::MainWindowActionContextIdentifier,
                "Open Houdini",
                "Opens Houdini and establishes a Session Sync",
                "Game",
                ":/houdini/open.svg",
                AzToolsFramework::ActionVisibility::HideWhenDisabled,
                [] {
                    SessionRequestBus::Broadcast(&SessionRequests::OpenHoudini);
                });

            RegisterAction("o3de.houdini.session.start",
                EditorIdentifiers::MainWindowActionContextIdentifier,
                "Start Session",
                "Starts a session with Houdini",
                "Game",
                ":/houdini/play.svg",
                AzToolsFramework::ActionVisibility::HideWhenDisabled,
                [] {
                    SessionRequestBus::Broadcast(&SessionRequests::StartSession);
                });

            RegisterAction("o3de.houdini.session.stop",
                EditorIdentifiers::MainWindowActionContextIdentifier,
                "Stop Session",
                "Stops the Houdini session",
                "Game",
                ":/houdini/stop.svg",
                AzToolsFramework::ActionVisibility::HideWhenDisabled,
                [] {
                    SessionRequestBus::Broadcast(&SessionRequests::StopSession);
                });

            RegisterAction("o3de.houdini.session.restart",
                EditorIdentifiers::MainWindowActionContextIdentifier,
                "Restart Session",
                "Restarts the Houdini session",
                "Game",
                ":/houdini/reset.svg",
                AzToolsFramework::ActionVisibility::HideWhenDisabled,
                [] {
                    SessionRequestBus::Broadcast(&SessionRequests::RestartSession);
                });

            // Viewport Widget
            {
                widgetActionProperties.m_name = "Sync Viewport";
                widgetActionProperties.m_category = "Game";

                auto outcome = m_actionManagerInterface->RegisterWidgetAction(
                    "o3de.houdini.session.viewportSync",
                    widgetActionProperties,
                    []() -> QWidget*
                    {
                        return new ViewportSyncWidget();
                    }
                );
            }

            SessionNotificationBus::Handler::BusConnect();
            OnSessionStatusChange(SessionSettings::ESessionStatus::Offline);

        }
    }

    void HoudiniEngineEditorSystemComponent::OnToolBarBindingHook()
    {
        m_toolBarManagerInterface->AddToolBarToToolBarArea(
            EditorIdentifiers::MainWindowTopToolBarAreaIdentifier, houdiniToolbarIdentifier, 100);

        // Play Controls
        {
            m_toolBarManagerInterface->AddWidgetToToolBar(houdiniToolbarIdentifier, "o3de.widgetAction.expander", 100);
            m_toolBarManagerInterface->AddSeparatorToToolBar(houdiniToolbarIdentifier, 200);
            m_toolBarManagerInterface->AddWidgetToToolBar(houdiniToolbarIdentifier, "o3de.houdini.houdiniEngineLabel", 300);
            m_toolBarManagerInterface->AddActionToToolBar(houdiniToolbarIdentifier, "o3de.houdini.nodesync.send", 400);
            m_toolBarManagerInterface->AddActionToToolBar(houdiniToolbarIdentifier, "o3de.houdini.session.open", 500);
            m_toolBarManagerInterface->AddActionToToolBar(houdiniToolbarIdentifier, "o3de.houdini.session.start", 600);
            m_toolBarManagerInterface->AddActionToToolBar(houdiniToolbarIdentifier, "o3de.houdini.session.stop", 700);
            m_toolBarManagerInterface->AddActionToToolBar(houdiniToolbarIdentifier, "o3de.houdini.session.restart", 800);
            m_toolBarManagerInterface->AddWidgetToToolBar(houdiniToolbarIdentifier, "o3de.houdini.session.viewportSync", 900);


            //m_toolBarManagerInterface->AddActionWithSubMenuToToolBar(houdiniToolbarIdentifier, "o3de.houdini.nodesync.send", EditorIdentifiers::PlayGameMenuIdentifier, 400);
            //m_toolBarManagerInterface->AddSeparatorToToolBar(houdiniToolbarIdentifier, 500);
            //m_toolBarManagerInterface->AddActionToToolBar(houdiniToolbarIdentifier, "o3de.action.game.simulate", 600);
        }

    }

    void HoudiniEngineEditorSystemComponent::NotifyRegisterViews()
    {
        AzToolsFramework::ViewPaneOptions viewOptions;
        viewOptions.isPreview = false;
        viewOptions.showInMenu = false;
        viewOptions.isStandard = true;
        viewOptions.isDeletable = false;
        viewOptions.preferedDockingArea = Qt::DockWidgetArea::RightDockWidgetArea;

        m_houdiniStatusPanel = new HoudiniStatusPanel();
        m_houdiniStatusPanel->OnInterrupt = [this]()
        {
            if (m_houdiniInstance != nullptr && m_houdiniInstance->IsActive())
            {
                //TODO: This seems to not work as expected.  WIP.
                //HAPI_Interrupt(&m_houdiniInstance->GetSession());
            }
        };

        m_configuration = new HoudiniConfiguration();
        m_sessionControls = new HoudiniSessionControls();

        AzToolsFramework::EditorRequestBus::Broadcast(&AzToolsFramework::EditorRequestBus::Events::RegisterViewPane, "HoudiniStatusPanel", "Houdini", viewOptions, [this](QWidget* /*unused*/) { return m_houdiniStatusPanel; });
        AzToolsFramework::EditorRequestBus::Broadcast(&AzToolsFramework::EditorRequestBus::Events::RegisterViewPane, "HoudiniConfiguration", "Houdini", viewOptions, [this](QWidget* /*unused*/) { return m_configuration; });
        AzToolsFramework::EditorRequestBus::Broadcast(&AzToolsFramework::EditorRequestBus::Events::RegisterViewPane, "HoudiniSessionControls", "Houdini", viewOptions, [this](QWidget* /*unused*/) { return m_sessionControls; });

    }

    void HoudiniEngineEditorSystemComponent::OnEntityActivated(const AZ::EntityId& id)
    {
        if (m_houdiniInstance)
        {
            m_houdiniInstance->LookupId(id);
        }
    }

    void HoudiniEngineEditorSystemComponent::OnEntityDeactivated(const AZ::EntityId& id)
    {
        if (m_houdiniInstance)
        {
            m_houdiniInstance->RemoveLookupId(id);
        }
    }

    void HoudiniEngineEditorSystemComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
    {
        AZ_PROFILE_FUNCTION(Editor);
        /* TODO: This doesn't work today:
        static bool InitTypes = true;
        if (InitTypes)
        {
            auto assetCatalog = AZ::Data::AssetCatalogRequestBus::FindFirstHandler();
            if (assetCatalog)
            {
                assetCatalog->EnableCatalogForAsset(AZ::AzTypeInfo<AnyAsset>::Uuid());
                InitTypes = false;
            }
        }
        */

        if (m_houdiniInstance != nullptr && m_houdiniInstance->GetInputNodeManager() != nullptr && GetIEditor()->IsInGameMode() == false)
        {
            m_houdiniInstance->GetInputNodeManager()->OnTick(deltaTime, time);

            int percent, assetsInQueue = 0;
            AZStd::string text;
            m_houdiniInstance->GetProgress(text, percent, assetsInQueue);
            m_houdiniStatusPanel->UpdatePercent(text, percent, assetsInQueue, m_houdiniInstance->GetHoudiniMode() == 1);
        }
    }

    AZ::Entity* HoudiniEngineEditorSystemComponent::FindTerrain()
    {
        // Retrieve Id map from game entity context (editor->runtime).
        AzFramework::EntityContextId editorEntityContextId = AzFramework::EntityContextId::CreateNull();
        AzToolsFramework::EditorEntityContextRequestBus::BroadcastResult(editorEntityContextId, &AzToolsFramework::EditorEntityContextRequestBus::Events::GetEditorEntityContextId);

        //O3DECONVERT
        /*
        // Get the Root Slice Component
        AZ::SliceComponent* rootSliceComponent;
        AzFramework::EntityContextRequestBus::EventResult(rootSliceComponent, editorEntityContextId, &AzFramework::EntityContextRequests::GetRootSlice);

        if (rootSliceComponent)
        {
            // Get an unordered_set of all EntityIds in the slice
            AZ::SliceComponent::EntityIdSet entityIds;
            rootSliceComponent->GetEntityIds(entityIds);

            for (const AZ::EntityId& id : entityIds)
            {
                AZ::Entity* entity = nullptr;
                AZ::ComponentApplicationBus::BroadcastResult(entity, &AZ::ComponentApplicationRequests::FindEntity, id);
                if (entity != nullptr)
                {
                    auto* terrain = entity->FindComponent<HoudiniTerrainComponent>();
                    if (terrain != nullptr)
                    {
                        return entity;
                    }
                }
            }
        }*/

        return nullptr;
    }

    bool HoudiniEngineEditorSystemComponent::IsActive()
    {
        if (m_houdiniInstance != nullptr)
        {
            return m_houdiniInstance->IsActive();
        }

        return false;
    }

    HoudiniPtr HoudiniEngineEditorSystemComponent::GetHoudiniEngine()
    {
        return m_houdiniInstance;
    }

    void HoudiniEngineEditorSystemComponent::CancelProcessorThread()
    {
        if (m_houdiniInstance)
        {
            m_houdiniInstance->CancelProcessorThread();
        }
    }

    void HoudiniEngineEditorSystemComponent::CancelProcessorJob(AZ::EntityId entityToRemove)
    {
        if (m_houdiniInstance)
        {
            m_houdiniInstance->CancelProcessorJob(entityToRemove);
        }
    }

    void HoudiniEngineEditorSystemComponent::JoinProcessorThread()
    {
        if (m_houdiniInstance)
        {
            m_houdiniInstance->JoinProcessorThread();
        }
    }

    AZStd::string HoudiniEngineEditorSystemComponent::GetHoudiniResultByCode(int code)
    {
        static AZStd::map<HAPI_Result, AZStd::string> codeMapping = {
            { HAPI_RESULT_SUCCESS, "HAPI_RESULT_SUCCESS" },
            { HAPI_RESULT_FAILURE, "HAPI_RESULT_FAILURE" },
            { HAPI_RESULT_ALREADY_INITIALIZED, "HAPI_RESULT_ALREADY_INITIALIZED" },
            { HAPI_RESULT_NOT_INITIALIZED, "HAPI_RESULT_NOT_INITIALIZED" },
            { HAPI_RESULT_CANT_LOADFILE, "HAPI_RESULT_CANT_LOADFILE" },
            { HAPI_RESULT_PARM_SET_FAILED, "HAPI_RESULT_PARM_SET_FAILED" },
            { HAPI_RESULT_INVALID_ARGUMENT, "HAPI_RESULT_INVALID_ARGUMENT" },
            { HAPI_RESULT_CANT_LOAD_GEO, "HAPI_RESULT_CANT_LOAD_GEO" },
            { HAPI_RESULT_CANT_GENERATE_PRESET, "HAPI_RESULT_CANT_GENERATE_PRESET" },
            { HAPI_RESULT_CANT_LOAD_PRESET, "HAPI_RESULT_CANT_LOAD_PRESET" },
            { HAPI_RESULT_ASSET_DEF_ALREADY_LOADED, "HAPI_RESULT_ASSET_DEF_ALREADY_LOADED" },

            { HAPI_RESULT_NO_LICENSE_FOUND, "HAPI_RESULT_NO_LICENSE_FOUND" },
            { HAPI_RESULT_DISALLOWED_NC_LICENSE_FOUND, "HAPI_RESULT_DISALLOWED_NC_LICENSE_FOUND" },
            { HAPI_RESULT_DISALLOWED_NC_ASSET_WITH_C_LICENSE, "HAPI_RESULT_DISALLOWED_NC_ASSET_WITH_C_LICENSE" },
            { HAPI_RESULT_DISALLOWED_NC_ASSET_WITH_LC_LICENSE, "HAPI_RESULT_DISALLOWED_NC_ASSET_WITH_LC_LICENSE" },
            { HAPI_RESULT_DISALLOWED_LC_ASSET_WITH_C_LICENSE, "HAPI_RESULT_DISALLOWED_LC_ASSET_WITH_C_LICENSE" },
            { HAPI_RESULT_DISALLOWED_HENGINEINDIE_W_3PARTY_PLUGIN, "HAPI_RESULT_DISALLOWED_HENGINEINDIE_W_3PARTY_PLUGIN" },

            { HAPI_RESULT_ASSET_INVALID, "HAPI_RESULT_ASSET_INVALID" },
            { HAPI_RESULT_NODE_INVALID, "HAPI_RESULT_NODE_INVALID" },

            { HAPI_RESULT_USER_INTERRUPTED, "HAPI_RESULT_USER_INTERRUPTED" },

            { HAPI_RESULT_INVALID_SESSION, "HAPI_RESULT_INVALID_SESSION" }
        };

        auto it = codeMapping.find((HAPI_Result)code);
        if (it != codeMapping.end())
        {
            return it->second;
        }

        return "{UNKNOWN RESULT CODE}";
    }

    void HoudiniEngine::HoudiniEngineEditorSystemComponent::PopulateEditorGlobalContextMenu_SliceSection(
        QMenu* menu, const AZ::Vector2& /*point*/, int /*flags*/)
    {
        IEditor* editor;
        AzToolsFramework::EditorRequests::Bus::BroadcastResult(editor, &AzToolsFramework::EditorRequests::GetEditor);

        CGameEngine* gameEngine = editor->GetGameEngine();
        if (!gameEngine || !gameEngine->IsLevelLoaded())
        {
            return;
        }

        //Check if we have Houdini setup:
        HoudiniEngine::HoudiniPtr hou;
        HoudiniEngine::HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngine::HoudiniEngineRequestBus::Events::GetHoudiniEngine);
        if (hou == nullptr || hou->IsActive() == false)
        {
            return;
        }

        AzToolsFramework::EntityIdList selectedEntityList;
        AzToolsFramework::ToolsApplicationRequests::Bus::BroadcastResult(selectedEntityList, &AzToolsFramework::ToolsApplicationRequests::GetSelectedEntities);

        AddDynamicContextMenus(menu, selectedEntityList);

        if (!selectedEntityList.empty())
        {
            //Is there an item selected that wants to add something we know about?
            int splineCount = 0;

            for (auto roadEntityId : selectedEntityList)
            {
                AZ::ConstSplinePtr spline = nullptr;
                LmbrCentral::SplineComponentRequestBus::EventResult(spline, roadEntityId, &LmbrCentral::SplineComponentRequests::GetSpline);
                if (spline != nullptr)
                {
                    splineCount++;
                }
            }
        }
    }

    void HoudiniEngineEditorSystemComponent::AddDynamicContextMenus(QMenu* menu, const AzToolsFramework::EntityIdList& selectedEntityList)
    {
        HoudiniPtr hou;
        HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);
        AZStd::vector<AZStd::string> output;
        output.push_back("");

        if (hou != nullptr && hou->IsActive())
        {
            QMenu* houMenu = new QMenu("Houdini");

            auto assets = hou->GetAvailableAssets();

            for (auto asset : assets)
            {
                auto result = asset->getAssets();

                QString configPath = asset->GetHdaFile().c_str();
                configPath = configPath.toLower();
                configPath = configPath.replace(".hda", ".xml");

                if (QFile::exists(configPath))
                {
                    QFile file(configPath);
                    if (file.open(QIODevice::ReadOnly))
                    {
                        QTextStream in(&file);
                        QString docSrc = in.readAll();
                        file.close();

                        using namespace AZ::rapidxml;

                        AZStd::shared_ptr<XmlData> xml(new XmlData(docSrc.toUtf8().data()));

                        if (xml->m_okay)
                        {
                            xml_node<>* hdaNode = xml->m_doc.first_node("HDA", 0, false);
                            if (hdaNode)
                            {
                                xml_node<char>* contextMenuItemNode = hdaNode->first_node("ContextMenuItem", 0, false);

                                for (; contextMenuItemNode; contextMenuItemNode = contextMenuItemNode->next_sibling("ContextMenuItem", 0, false))
                                {
                                    AddDynamicContextMenu(houMenu, selectedEntityList, xml, contextMenuItemNode);
                                }
                            }
                        }
                    }
                }
            }

            if (houMenu->actions().empty() == false)
            {
                menu->addMenu(houMenu);
            }
        }
    }

    bool HoudiniEngineEditorSystemComponent::FindMatchingIds(const AzToolsFramework::EntityIdList& selectedEntityList, AZ::rapidxml::xml_node<>* contextMenuItemNode, AZStd::vector<AZ::EntityId>& matchingIds)
    {
        using namespace AZ::rapidxml;

        for (auto selectedId : selectedEntityList)
        {
            AZ::Entity* selectedEntity = nullptr;
            AZ::ComponentApplicationBus::BroadcastResult(selectedEntity, &AZ::ComponentApplicationRequests::FindEntity, selectedId);
            if (selectedEntity == nullptr)
            {
                continue;
            }

            // ===  "IfComponent" === //
            bool hasComponent = true;
            bool hasOperator = true;

            xml_node<char>* ifCompNode = contextMenuItemNode->first_node("IfComponent", 0, false);
            for (; ifCompNode; ifCompNode = ifCompNode->next_sibling("IfComponent", 0, false))
            {
                hasComponent = false;
                auto* nameNode = ifCompNode->first_attribute("Name", 0, false);
                if (nameNode)
                {
                    QString expectedCompName = nameNode->value();

                    //Check for this component:
                    for (auto* comp : selectedEntity->GetComponents())
                    {
                        QString currentComponentName = comp->RTTI_GetTypeName();

                        if (expectedCompName.compare(currentComponentName, Qt::CaseInsensitive) == 0)
                        {
                            hasComponent = true;
                            break;
                        }
                    }
                }
            }

            // ===  "IfOperator" === //
            xml_node<char>* ifOpNode = contextMenuItemNode->first_node("IfOperator", 0, false);
            for (; ifOpNode; ifOpNode = ifOpNode->next_sibling("IfOperator", 0, false))
            {
                hasOperator = false;
                auto* nameNode = ifOpNode->first_attribute("Name", 0, false);
                if (nameNode)
                {
                    QString opName = nameNode->value();

                    HoudiniEngine::IHoudiniNodeComponentConfig* config = nullptr;
                    HoudiniEngine::HoudiniAssetRequestBus::EventResult(config, selectedId, &HoudiniEngine::HoudiniAssetRequests::GetConfig);
                    if (config != nullptr)
                    {
                        if (opName.compare(config->GetOperatorName().c_str(), Qt::CaseInsensitive) == 0)
                        {
                            hasOperator = true;
                            break;
                        }
                    }
                }
            }

            if (hasComponent && hasOperator)
            {
                matchingIds.push_back(selectedId);
            }
        }

        return true;
    }

    void HoudiniEngineEditorSystemComponent::AddDynamicContextMenu(QMenu* menu, const AzToolsFramework::EntityIdList& selectedEntityList, AZStd::shared_ptr<XmlData> xml, AZ::rapidxml::xml_node<>* contextMenuItemNode)
    {
        using namespace AZ::rapidxml;

        /* Find all name to Component mappings:

        AZ::EBusAggregateResults<AZ::ComponentDescriptor*> components;
        AZ::ComponentDescriptorBus::BroadcastResult(components, &AZ::ComponentDescriptorBus::Events::GetDescriptor);

        AZStd::map<AZStd::string, AZ::Uuid> nameMap;
        for (auto& descriptor : components.values)
        {
            AZStd::string name = descriptor->GetName();
            nameMap[name] = descriptor->GetUuid();
        }*/

        auto* nameAttrib = contextMenuItemNode->first_attribute("Name", 0, false);
        if (nameAttrib)
        {
            //TODO: Check for right click selector pass:
            AZStd::vector<AZ::EntityId> matchingIds;
            FindMatchingIds(selectedEntityList, contextMenuItemNode, matchingIds);

            if (matchingIds.size() == 0)
            {
                //Nothing matches:
                return;
            }

            QAction* action = menu->addAction(nameAttrib->value());
            QObject::connect(action, &QAction::triggered, [=]()
                {
                    AZStd::string errors = xml->m_doc.getError();

                    for (const AZ::EntityId& id : matchingIds)
                    {
                        // ===  "CreateAttribute" === //
                        xml_node<char>* createAttribNode = contextMenuItemNode->first_node("CreateAttribute", 0, false);
                        for (; createAttribNode; createAttribNode = createAttribNode->next_sibling("CreateAttribute", 0, false))
                        {
                            auto* attribNameToAddNode = createAttribNode->first_attribute("Name", 0, false);
                            if (attribNameToAddNode)
                            {
                                //Find the attribute name to add:
                                AZStd::string attribName = attribNameToAddNode->value();

                                //Does the attribute already exist:
                                AZ::EBusAggregateResults<AZStd::string> names;
                                HoudiniEngine::HoudiniCurveAttributeRequestBus::EventResult(names, id, &HoudiniEngine::HoudiniCurveAttributeRequests::GetName);

                                bool exists = false;
                                for (const AZStd::string& previousName : names.values)
                                {
                                    if (previousName == attribName)
                                    {
                                        exists = true;
                                    }
                                }

                                //Add the attribute if it does not exist:
                                if (!exists)
                                {
                                    const auto& Attributes = AZ::ComponentTypeList{ AZ::Uuid(HOUDINI_CURVE_ATTRIBUTE_COMPONENT_GUID) };

                                    AzToolsFramework::EntityCompositionRequests::AddComponentsOutcome addedComponentsResult;
                                    AzToolsFramework::EntityCompositionRequestBus::BroadcastResult(
                                        addedComponentsResult,
                                        &AzToolsFramework::EntityCompositionRequests::AddComponentsToEntities,
                                        AzToolsFramework::EntityIdList{ id }, Attributes
                                    );

                                    for (auto& componentsAddedToEntity : addedComponentsResult.GetValue())
                                    {
                                        if (componentsAddedToEntity.second.m_addedValidComponents.size() > 0)
                                        {
                                            auto* compo = componentsAddedToEntity.second.m_addedValidComponents[0];
                                            HoudiniEngine::HoudiniCurveAttributeRequestBus::Event(id, &HoudiniEngine::HoudiniCurveAttributeRequests::SetName, compo->GetId(), attribName);
                                        }
                                    }
                                }
                            }
                        }

                        //Create the HDA if requested:
                        // ===  "CreateEntityHDA" === //
                        xml_node<char>* createEntityNode = contextMenuItemNode->first_node("CreateEntityHDA", 0, false);
                        for (; createEntityNode; createEntityNode = createEntityNode->next_sibling("CreateEntityHDA", 0, false))
                        {
                            AZStd::string newEntityName;
                            AZStd::string newEntityOperator;

                            AZ::ComponentApplicationBus::BroadcastResult(newEntityName, &AZ::ComponentApplicationRequests::GetEntityName, id);

                            // ===  "Operator" === //
                            auto* operatorAttribute = createEntityNode->first_attribute("Operator", 0, false);
                            if (operatorAttribute)
                            {
                                newEntityOperator = operatorAttribute->value();
                            }

                            // ===  "NameSuffix" === //
                            auto* nameAttrib = createEntityNode->first_attribute("NameSuffix", 0, false);
                            if (nameAttrib)
                            {
                                newEntityName += nameAttrib->value();
                            }

                            AZStd::vector<AZStd::pair<AZStd::string, AZStd::string>> materialsToSet;
                            AZStd::vector<AZStd::pair<AZStd::string, AZ::EntityId>> entitiesToSet;

                            //Create the entity and assign properties:
                            if (newEntityOperator.empty() == false)
                            {
                                // ===  "SetEntity" === //
                                xml_node<char>* setNode = createEntityNode->first_node("SetEntity", 0, false);
                                for (; setNode; setNode = setNode->next_sibling("SetEntity", 0, false))
                                {
                                    auto* keyAttrib = setNode->first_attribute("Key", 0, false);
                                    auto* valueAttrib = setNode->first_attribute("Value", 0, false);

                                    if (keyAttrib && valueAttrib)
                                    {
                                        QString valueStr = valueAttrib->value();
                                        if (valueStr.compare("%SelectedEntity%", Qt::CaseInsensitive) == 0)
                                        {
                                            entitiesToSet.push_back(AZStd::make_pair(keyAttrib->value(), id));
                                        }
                                        else if (valueStr.compare("%TerrainEntity%", Qt::CaseInsensitive) == 0)
                                        {
                                            AZ::Entity* terrain = FindTerrain();
                                            if (terrain != nullptr)
                                            {
                                                entitiesToSet.push_back(AZStd::make_pair(keyAttrib->value(), terrain->GetId()));
                                            }
                                        }
                                        else
                                        {
                                            //TODO: Search for an entity with the matching name?
                                            //config->SetPropertyValueEntityId(keyAttrib->value(), splineId);
                                        }
                                    }
                                }

                                // ===  "SetMaterial" === //
                                xml_node<char>* setMaterialNode = createEntityNode->first_node("SetMaterial", 0, false);
                                for (; setMaterialNode; setMaterialNode = setMaterialNode->next_sibling("SetMaterial", 0, false))
                                {
                                    auto* keyAttrib = setMaterialNode->first_attribute("Key", 0, false);
                                    auto* valueAttrib = setMaterialNode->first_attribute("Value", 0, false);

                                    if (keyAttrib && valueAttrib)
                                    {
                                        AZStd::string valueStr = valueAttrib->value();
                                        materialsToSet.push_back(AZStd::make_pair(keyAttrib->value(), valueStr));
                                    }
                                }


                                AZ::Entity* newEntity = CreateNewHoudiniDigitalAsset(newEntityName, id);

                                AZStd::function<void(IHoudiniNode*)> OnLoad = [newEntity, entitiesToSet, materialsToSet](HoudiniEngine::IHoudiniNode* hdaNode)
                                {
                                    if (hdaNode != nullptr)
                                    {
                                        //Set Property Values:
                                        HoudiniEngine::IHoudiniNodeComponentConfig* config;
                                        HoudiniEngine::HoudiniAssetRequestBus::EventResult(config, newEntity->GetId(), &HoudiniEngine::HoudiniAssetRequests::GetConfig);

                                        if (config != nullptr)
                                        {
                                            for (auto& keyPair : entitiesToSet)
                                            {
                                                config->SetPropertyValueEntityId(keyPair.first, keyPair.second);
                                            }

                                            for (auto& keyPair : materialsToSet)
                                            {
                                                HoudiniEngine::HoudiniMaterialRequestBus::Event(newEntity->GetId(), &HoudiniEngine::HoudiniMaterialRequests::SetMaterialPath, keyPair.first, keyPair.second);
                                            }
                                        }
                                    }
                                };

                                HoudiniEngine::IHoudiniNode* hdaNode = nullptr;
                                HoudiniEngine::HoudiniAssetRequestBus::EventResult(hdaNode, newEntity->GetId(), &HoudiniEngine::HoudiniAssetRequests::LoadHda, newEntityOperator, OnLoad);

                            }
                        }
                    }
                });

        }
    }

    AZ::Entity* HoudiniEngineEditorSystemComponent::CreateNewHoudiniDigitalAsset(const AZStd::string& name, const AZ::EntityId& parent)
    {
        //Add the Houdini digital asset for the road.
        AZ::EntityId newEntityId;
        AzToolsFramework::EditorEntityContextRequestBus::BroadcastResult(newEntityId,
            &AzToolsFramework::EditorEntityContextRequests::CreateNewEditorEntity, name.c_str());

        AZ::Entity* newEntity;
        AZ::ComponentApplicationBus::BroadcastResult(newEntity, &AZ::ComponentApplicationRequests::FindEntity, newEntityId);

        AZ::Transform transform = AZ::Transform::CreateIdentity();

        if (parent.IsValid())
        {
            AZ::TransformBus::Event(newEntityId, &AZ::TransformInterface::SetParent, parent);
        }

        AZ::TransformBus::Event(newEntityId, &AZ::TransformInterface::SetLocalTM, transform);

        newEntity->Deactivate();
        newEntity->CreateComponent(AZ::Uuid(HOUDINI_ASSET_COMPONENT_GUID));
        newEntity->Activate();

        return newEntity;
    }

}
