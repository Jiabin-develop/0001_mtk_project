#pragma once

#include "../common.h"
typedef enum
{
    TASK_PRINT_PRINT,
} cmd_list_print_t;

typedef struct
{
    cmd_list_print_t cmd;
    char msg[20];
    char *value;
} cmd_print_t;

void task_print(INT stacd, void *exinf);
void xmtk_create_task_print();
void xmtk_start_task_print();
