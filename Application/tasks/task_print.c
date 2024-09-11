#include "task_print.h"

EXPORT ID CommandQueue_print;
EXPORT ID Task_print;

T_CTSK ctsk_print = {
    // Task creation information
    .itskpri = 10,
    .stksz = 1024,
    .task = task_print,
    .tskatr = TA_HLNG | TA_RNG3,
};

T_CMBF cmbf_task_print = {
    // Message buffer creation information
    .mbfatr = TA_TFIFO | TA_MFIFO,
    .bufsz = 1024,
    .maxmsz = sizeof(cmd_print_t),
};

void xmtk_create_task_print()
{
    CommandQueue_print = tk_cre_mbf(&cmbf_task_print);
    Task_print = tk_cre_tsk(&ctsk_print);
}

void xmtk_start_task_print()
{
    tk_sta_tsk(Task_print, 0);
}
void task_print(INT stacd, void *exinf)
{
    cmd_print_t print_cmd;
    INT len = 0;
    tm_printf((UB *)"task task_print_cmd on\n");
    while (1)
    {
        len = xmtk_receive_command(CommandQueue_print, print_cmd);
        if (len > 0)
        {
            switch (print_cmd.cmd)
            {
            case TASK_PRINT_PRINT:
                tm_printf((UB *)"print: got cmd: %d, msg: %s, value: %s\n", print_cmd.cmd, print_cmd.msg, print_cmd.value);
                break;
            default:
                break;
            }
        }
    }
}