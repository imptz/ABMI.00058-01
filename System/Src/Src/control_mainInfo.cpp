#include "control_mainInfo.h"
#include "log.h"
#include "low.h"
#include "control_button.h"
#include "debug.h"

MainInfo::MainInfo(unsigned int _positionX, unsigned int _positionY, MessageReceiver* _messageReceiver)
	:	Control(_positionX, _positionY, WIDTH, HEIGHT, _messageReceiver)
{
	draw();
}

MainInfo::~MainInfo()
{

}

void MainInfo::draw()
{
	drawChildControls();
}

void MainInfo::onMessage(Message message)
{
}
