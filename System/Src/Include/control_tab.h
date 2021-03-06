#ifndef CONTROL_TAB_H
#define CONTROL_TAB_H

#include "control_panel.h"

class TabControl : public Control{
protected:
    int visibleControlIndex;

    unsigned int tabCount;

    TabControl();

public:
    TabControl(unsigned int _positionX, unsigned int _positionY, unsigned int _width, unsigned int _height, unsigned int _tabCount,
        Window::BORDER_STYLE _borderStyle = Window::BORDER_STYLE_INVISIBLE, MessageReceiver* _messageReceiver = nullptr);
    virtual ~TabControl();
    virtual void draw();
    virtual void onMessage(Message message);

    Panel* getTabPanel(int index);
    void setActiveTab(unsigned int index, bool redraw = true);
    virtual void setVisible(bool _visible, bool childVisible = true);

    enum TAB_CONTROL_MESSAGE{
        TAB_CONTROL_MESSAGE_SET_ACTIVE = 15
    };

};

#endif
