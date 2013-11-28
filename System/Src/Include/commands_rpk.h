#ifndef COMMANDS_RPK_H
#define COMMANDS_RPK_H

enum RPK_COMMANDS{
	RPK_COMMANDS_GATE_OPEN = 9,
	RPK_COMMANDS_GATE_CLOSE = 10,
	RPK_COMMANDS_GATE_STOP = 11,
	RPK_COMMANDS_UP = 17,
	RPK_COMMANDS_GATE_INFO = 158,
	RPK_COMMANDS_START_SCAN_PROGRAM_OP = 29,
	RPK_COMMANDS_START_SCAN_PROGRAM = 30,
	RPK_COMMANDS_STOP_SCAN_PROGRAM = 31,
	RPK_COMMANDS_FLAGS = 100,
	RPK_COMMANDS_POVOROTNIY = 103,
	RPK_COMMANDS_MOVE_TO_POINT = 117,
	RPK_COMMANDS_GET_STATUS_MOVE_TO_POINT = 118,
	RPK_COMMANDS_REPLY = 120,
	RPK_COMMANDS_GET_NETWORK_CONTROLLER_INFO = 160,
	RPK_COMMANDS_START_SEARCH = 233,
	RPK_COMMANDS_GET_SEARCH_RESULT = 235,
	RPK_COMMANDS_SET_PRESSURE_TABLE = 236
};

enum SCAN_PROGRAM_BALLISTICS{
	SCAN_PROGRAM_BALLISTICS_OFF = 0,
	SCAN_PROGRAM_BALLISTICS_ON = 1
};

enum SCAN_PROGRAM_STEP{
	SCAN_PROGRAM_STEP_LOW = 3
};

enum FLAGS{
	FLAGS_SCAN_PROGRAM_START = 0
};

enum SCAN_PROGRAM_FLAGS{
	SCAN_PROGRAM_FLAGS_NASADOK_OFF = 1,
	SCAN_PROGRAM_FLAGS_NASADOK_ON = 0
};

#endif
