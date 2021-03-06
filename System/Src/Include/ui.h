#ifndef UI_H
#define UI_H

#include "singleton.h"
#include "messages.h"
#include "control_desktop.h"
#include "control_tab.h"
#include "control_log.h"
#include "control_infoPanel.h"
#include "control_mainTab.h"
#include "control_toolsTab.h"

class UI : public Singleton<UI>, public MessageReceiver{
private:
    Desktop* desktop;
    InfoPanel* infoPanel;
    LogControl* logControl;
    TabControl* tabControl;
    MainTabControl* mainTabControl;
    ToolsTabControl* toolsTabControl;

public:
    UI(){
        desktop = new Desktop(0, 0, 80, 25, this);

        infoPanel = new InfoPanel(0, 0, this);
        logControl = new LogControl(0, 14);
        tabControl = new TabControl(0, 3, 80, 11, 2);
        mainTabControl = new MainTabControl(0, 3, 80, 11);
        toolsTabControl = new ToolsTabControl(0, 3, 80, 11, this);

        desktop->addChildControl(infoPanel);
        desktop->addChildControl(logControl);
        desktop->addChildControl(tabControl);
        tabControl->getTabPanel(0)->addChildControl(mainTabControl);
        tabControl->getTabPanel(1)->addChildControl(toolsTabControl);
        tabControl->setActiveTab(0);
        desktop->draw();
    }

    ~UI(){}

    virtual void onMessage(Message message){
        if ((message.from == MESSAGE_FROM_OFFSET_CONTROLS + infoPanel->getId()) && (message.msg == InfoPanel::INFO_PANEL_MESSAGE_NEW_USO_MODE)){
            if (message.par2 == UsoModeControl::USO_MODE_TOOLS)
                tabControl->setActiveTab(1);
        }

        if ((message.from == MESSAGE_FROM_OFFSET_CONTROLS + toolsTabControl->getId()) &&
            (message.msg == ToolsTabControl::TOOLS_TAB_CONTROL_MESSAGE_EXIT)){
            infoPanel->getUsoModeControl()->setMode(UsoModeControl::USO_MODE_PREV, UsoModeControl::USO_MODE_CONTROL_ACTOR_TOOLS);
            tabControl->setActiveTab(0);
        }
    }

    UsoModeControl* getUsoModeControl(){
        return infoPanel->getUsoModeControl();
    }

    MainTabControl* getMainTabControl(){
        return mainTabControl;
    }
};

#endif
