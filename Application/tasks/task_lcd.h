#pragma once

#include "../common.h"
typedef enum
{
    TASK_LCD_TEST,
    TASK_LCD_UPDATE_PULSE,
    TASK_LCD_CHANGE_UPDATE,
    TASK_LCD_ON,
    TASK_LCD_OFF,
} cmd_list_lcd_t;

typedef struct
{
    cmd_list_lcd_t cmd;
    char msg[20];
    char *value;
} command_lcd_t;

void task_lcd(INT stacd, void *exinf);
void xmtk_create_task_lcd();
void xmtk_start_task_lcd();

extern ID mbfid_lcd;
extern ID tskid_lcd;