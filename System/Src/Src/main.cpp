#include "init.h"
#include "root.h"
#include "application.h"
#include "lowDisplay.h"

static int o = 90;

void main()
{
	asm volatile ("\tmovw	$0x10, %ax\n");
	asm volatile ("\tmovw	%ax, %fs\n");
	asm volatile ("\tmovw	%ax, %gs\n");

	init();

	print("main", 40, getNextPos());
	Root root;

	Application::getSingleton().start();
}


