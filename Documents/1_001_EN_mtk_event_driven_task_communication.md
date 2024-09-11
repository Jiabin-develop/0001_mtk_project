
# Event-driven tasks implementation in μT-Kernel RTOS

## Introduction

This document explains the implementation of an event-driven (command-driven) task communication in μT-Kernel. The communications and controls of tasks have the following characteristics:
- A task is executed only after it has received a command (request) from other resources. If there is no command, the task is in a waiting (blocked) state `TMO_FEVR`. This allows the CPU resources to be available to other tasks.
    - Interrupt that requests a task to process requests
    - Requested by other tasks
- Each task has its own command queue, which is used to receive commands from other tasks. A task has the following components:
    - `CommandQueue_xxx`: receives commands from other tasks
    - `Task_xxx`: the functions of the task
- Two functions for sending and receiving commands are redefined respectively:
    - `xmtk_send_command(commandQueue, command)`: sends commands to a specific command queue
    - `xmtk_receive_command(commandQueue, command)`: receives commands from a specific command queue
    ```c
    #define xmtk_send_command(commandQueue, command) tk_snd_mbf(commandQueue, &command, sizeof(command), TMO_POL);
    #define xmtk_receive_command(commandQueue, command) tk_rcv_mbf(commandQueue, &command, TMO_FEVR);
    ```
- A template file is made as `task_template.c`.
- A flowchart of the structure of a task is as follows:
    ```mermaid
        stateDiagram-v2
            Requester --> CommandQueue
            CommandQueue --> switch(command.cmd) : xmtk_receive_command(command)
            state switch(command.cmd) {
                direction LR
                cmd0: cmd0->task_cmd0()
                cmd1: cmd01->task_cmd1()
                cmd2: cmd2->task_cmd2()
            }
    ```
- Communication between tasks are as follows, where the `requester` can be any code, such as an interrupt handler, `execute_cmd`, and so on. Different tasks have their own `CommandQueue`:
    ```mermaid
        stateDiagram-v2
            re1: requester
            re2: requester
            re3: requester
            re1 --> CommandQueue0: send_command
            re2 --> CommandQueue1: send_command
            re3 --> CommandQueue2: send_command
            CommandQueue0 --> switch(command0.cmd) : receive_command
            CommandQueue1 --> switch(command1.cmd) : receive_command
            CommandQueue2 --> switch(command2.cmd) : receive_command

            state switch(command0.cmd) {
                direction LR
                cmd0: execute command0.cmd 
            }
            state switch(command1.cmd) {
                direction LR
                cmd1: execute command1.cmd
            }
            state switch(command2.cmd) {
                direction LR
                cmd2: execute command2.cmd
            }
    ```

## Implementations

### Define the Command
- Each task will receive a command to be executed. The command struct `command_template_t` needs to be defined first.
    ```c
    typedef struct
    {
        cmd_list_template_t cmd;
        char msg[20];
        char *value;
    } command_template_t;
    ```
- Here, the cmd is defined by a command list enumeration `cmd_list_template_t`, which lists the commands that this task needs to execute:
    ```c
    typedef enum
    {
        TASK_TEMPLATE_CMD,
        TASK_TEMPLATE_CMD_1,
    } cmd_list_template_t;
    ```
### Configuration of a task
- A task needs components that must be shared with other tasks. One is the `Task` function, and another is `CommandQueue`.
    ```c
    EXPORT ID CommandQueue_template;
    EXPORT ID Task_template;
    ```
- Define the configuration for the `CommandQueue` and the `Task`.
    ```c
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
    ```
- The executing code of the task. The task is executed only after it has received a command (`xmtk_receive_command`), if there is no command, the task is in a waiting (blocked) state. This gives the CPU resources available to other tasks.
    ```c
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
    ```
- Functions to register the task and start the task
    ```c
        void create_task_template()
        {
            CommandQueue_template = tk_cre_mbf(&cmbf_task_template);
            Task_template = tk_cre_tsk(&ctsk_template);
        }

        void start_task_template()
        {
            tk_sta_tsk(Task_template, 0);
        }
    ```

### Send a command
- A task can send a command to another task to request execution of a command, by sending an instance of the command to the `CommandQueue`. A message can be attached to the command, to provide more details. Below is an example of sending a command to the `CommandQueue_template`, which requires the `Task_template` to execute the command `TEMPLATE_CMD_0`.
    ```c
        command_template_t cmd = 
        {
            .cmd = TEMPLATE_CMD_0, 
            .msg = "send to template", 
            .value = "test"
        };
        xmtk_send_command(CommandQueue_template, cmd);
    ```

## Conclusion
This document explains how the author implemented an event-driven (command-driven) communication between tasks in μT-Kernel RTOS. You can customize the `cmd_list_template_t` and the `Task_template` according to your specific requirements.