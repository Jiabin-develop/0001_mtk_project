#pragma once

#include "task_print.h"
#include "task_lcd.h"
#include "task_hr_sensor.h"
#include "task_template.h"

IMPORT ID CommandQueue_template;
IMPORT ID Task_template;

IMPORT ID CommandQueue_print;
IMPORT ID Task_print;

IMPORT ID CommandQueue_hr;
IMPORT ID Task_hr;

IMPORT ID CommandQueue_lcd;
IMPORT ID Task_lcd;

#define xmtk_send_command(commandQueue, command) tk_snd_mbf(commandQueue, &command, sizeof(command), TMO_POL);
#define xmtk_receive_command(commandQueue, command) tk_rcv_mbf(commandQueue, &command, TMO_FEVR);