#pragma once

#include "../common.h"
typedef enum
{
    TEMPLATE_CMD_0,
    TEMPLATE_CMD_1,
} cmd_list_template_t;

typedef struct
{
    cmd_list_template_t cmd;
    char msg[20];
    char *value;
} command_template_t;

void task_template(INT stacd, void *exinf);
void xmtk_create_task_template();
void xmtk_start_task_template();
