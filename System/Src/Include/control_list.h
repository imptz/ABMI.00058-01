#ifndef CONTROL_LIST_H
#define CONTROL_LIST_H

#include "control.h"

class List : public Control{
private:
    unsigned int stringCount;
    unsigned int stringLength;
    unsigned int addCount;

    char* strings;
    List();

public:
    List(unsigned int _stringCount, unsigned _stringLength, unsigned int _positionX, unsigned int _positionY, MessageReceiver* _messageReceiver = nullptr);
    virtual ~List();
    virtual void draw();
    virtual void onMessage(Message message);

    void clear(bool redraw = false);
    void insert(char* str, unsigned int index, bool redraw = false);
    void add(char* str, unsigned int index, bool redraw = false);
    void addToTop(char* str, bool redraw = false);
    void addToEnd(char* str, bool redraw = false);
};

#endif
