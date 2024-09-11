#include "task_template.h"

EXPORT ID CommandQueue_template;
EXPORT ID Task_template;
command_template_t template_cmd;

T_CTSK ctsk_template = {
    // Task creation information
    .itskpri = 10,
    .stksz = 1024,
    .task = task_template,
    .tskatr = TA_HLNG | TA_RNG3,
};

T_CMBF cmbf_task_template = {
    // Message buffer creation information
    .mbfatr = TA_TFIFO | TA_MFIFO,
    .bufsz = 1024,
    .maxmsz = sizeof(command_template_t),
};

void xmtk_create_task_template()
{
    CommandQueue_template = tk_cre_mbf(&cmbf_task_template);
    Task_template = tk_cre_tsk(&ctsk_template);
}

void xmtk_start_task_template()
{
    tk_sta_tsk(Task_template, 0);
}
void task_template(INT stacd, void *exinf)
{
    INT len = 0;
    tm_printf((UB *)"task task_template on\n");
    while (1)
    {
        len = xmtk_receive_command(CommandQueue_template, template_cmd);
        if (len > 0)
        {
            tm_printf((UB *)"template: got cmd: %d, msg: %s, value: %s\n", template_cmd.cmd, template_cmd.msg, template_cmd.value);
            switch (template_cmd.cmd)
            {
            case TEMPLATE_CMD_0:
                // implementation
                break;
            case TEMPLATE_CMD_1:
                // implementation
                break;
            default:
                break;
            }
        }
    }
}