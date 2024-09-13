# μT-Kernel application firmware 
Human body stress checker.  
[日本語版](./2_000_JP_General_firmware_design.md)

## Table of contents
1. [Project Overview](#project-overview)
1. [Hardware](#hardware-overview)
1. [Software](#software-overview)
1. [Design Description](#design-description)
    1. [Directory Structure](#directory-structure)
    1. [Tasks](#tasks)
    1. [Interrupts](#interrupts)
    1. [Peripheral](#peripheral)
1. [Important Implementations](#important-implementations)
    1. [Task Functions](#task-functions)
    1. [Task Creation](#task-creation)
    1. [Task Starting](#task-starting)
    1. [Peripheral Configuration](#peripheral-configuration)
    1. [Interrupt Handlers](#interrupt-handlers)
    1. [Register the Interrupt into μT-Kernel interrupt management](#register-the-interrupt-into-μt-kernel-interrupt-management)
1. [Application main function `app.main.c`](#application-main-function-appmainc)
1. [Knowhow during development](#knowhow-during-development)
    1. [μT-Kernel event-driven implementation](#μt-kernel-event-driven-implementation)
    1. [μT-Kernel interrupt configuration](#μt-kernel-interrupt-configuration)
1. [Project Build](#project-build)
1. [Additional Information](#additional-information)
1. [References](#references)

---
1. ## Project overview
    - A human body stress checker is implemented using μT-Kernel as real-time operating system (RTOS). 
    - The application can measure the heart rate, heart rate variability, and provide corresponding stress level index. The measurement is conducted by finger touch to a pulse sensor. The bio- parameters are measured. The stress check function is a rarely seen in market. In this application, by referring the theory that stress level can be measured by measuring the heart rate variability. The heart rate variability is measured, and a stress level is provided.
    - The Infineon KIT_XMC72_EVK development board is used for the development. 
    - This project fully utilize the μT-Kernel APIs to build the application, which includes most of the important part of a embedded system and μT-Kernel system. 
      - μT-Kernel Multi-tasks running.
      - μT-Kernel Interrupt management.
      - μT-Kernel Task synchronization
      - μT-Kernel Timer management.
      - Serial communication: 
        - I2C communication.
        - UART print.
      - Input peripheral: 
        - IO interrupt.
      - Output peripheral: 
        - LCD display.
    - The implementations considered the actual use cases, and provides necessary information in the LCD display, such as 
        - User touching detection / informing (no measurement)
        - Indicating the measuring status
        - Showing the pulse wave during the measurement
        - Informing if user is not touching the sensor (no measurement)
        - Screenshot of the LCD screen if user button pushed down 
    - The project is going to be open source as an example for quickly start the development of μT-Kernel. The files and directories are well organized by resources, which makes it easy to be expanded with more features. The detail information about the implementation is also appended, which allows to understand the implementation. 
    - The implementation is implemented in event-driven concept. It is for the low-power consumption design purpose. All task execution are performed only when a target event happens. If there is no event request the execution of the task, the task is in a block status, which can allow MCU to be put into a low-power mode. A timer is implemented to turn off the sensor and display if the sensor is not running. By pushing the user button, the measurement can be resumed.


1. ## Hardware Overview
    - [Kit XMC72 EVK](https://www.infineon.com/cms/en/product/evaluation-boards/kit_xmc72_evk/)
        - USER Button 
    - Pulse sensor (e.g., MAX30102)
        - I2C communication
        - Data ready interrupt (FIFO)
    - LCD display (e.g., ssd1306)
        - I2C communication
    - USER Button
        - Interrupt
    - Connection
      ```mermaid
      graph TD;
        subgraph PULSE ["PULSE Sensor: MAX30102"]
            PULSE_SDA["SDA"]
            PULSE_SCL["SCL"]
            PULSE_INT["INT_P"]
            PULSE_GND["GND"]
            PULSE_VCC["VCC_3.3V"]
        end

        subgraph MCU ["MCU: Kit XMC72 EVK"]
            MCU_SDA["SDA"]
            MCU_SCL["SCL"]
            MCU_INT["P10_5"]
            MCU_BTN["P13_6"]
            MCU_VCC["VCC"]
            MCU_GND["GND"]
        end

        subgraph LCD ["LCD Display: SSD1306"]
            LCD_SDA["SDA"]
            LCD_SCL["SCL"]
            LCD_VCC["VCC"]
            LCD_GND["GND"]
        end

        subgraph BTN ["Button"]
            BTN_BTN["GND <-- BTN"]
        end

        MCU_SDA --- LCD_SDA;
        MCU_SCL --- LCD_SCL;

        PULSE_SDA --- MCU_SDA;
        PULSE_SCL --- MCU_SCL ;
        PULSE_VCC --- MCU_VCC;
        PULSE_GND --- MCU_GND;
        PULSE_INT --- MCU_INT;

        MCU_VCC --- LCD_VCC;
        MCU_GND --- LCD_GND;
        
        MCU_BTN --- BTN_BTN;
        MCU_GND --- BTN_BTN;
        
      ```
    - Actual connection
      - zoom out
        - ![alt text](imgs/Device_out.jpg)
      - zoom in
        - LCD contents
          - `1-9`: Stress level (higher value means high stress)
          - `HR`: Heart rate
          - `Va`: Heart rate variability
          - Waveform of the pulse
        - ![alt text](imgs/Device.png)


1. ## Software Overview
    - [μT-Kernel 3.0 BSP2](https://www.tron.org/ja/page-6100/)
    - [μT-Kernel 3.0 BSP2 for ModusToolBox](https://github.com/tron-forum/mtk3_bsp2/blob/main/doc/bsp2_xmc_mtb_jp.md/)
    - [Infineon Modustoolbox](https://www.infineon.com/cms/en/design-support/tools/sdk/modustoolbox-software/)
    - [VS Code](https://code.visualstudio.com/)  
    A step-by-step setup of Modustoolbox development in VScode is uploaded here:
    https://github.com/Jiabin-develop/knowhow_share_public/blob/main/MTB_MTK/MTB_MTK_VSCODE.md


1. ## Design Description
    1. ### Directory structure
        The directories and files are well separated into different directories. The resources are divided into:
        - tasks
        - interrupt
        - peripheral
            - The following peripheral drivers are a clone of the following repository:
                - `HR_sensor`: https://github.com/eepj/stm32-max30102
                - `LCD_ssd1306`:https://github.com/4ilo/ssd1306-stm32HAL/tree/master 
        By including the head file `user_xxx.h`, the resources in that folder can be used.
          ```
            0000_mtk_project/
            ├── Applications/
            │   ├── app.main.c
            │   ├── common.h
            │   ├── tasks/
            │   │   ├── user_task.h
            │   │   └── ...
            │   ├── interrupt/
            │   │   ├── user_interrupt.h
            │   │   └── ...
            │   └── peripheral/
            │       ├── user_peripheral.h
            │       └── ...
            ├── Documents/
            └── ...
          ```
    1. ### Tasks
        Each task is responsible for a hardware/peripheral manipulation. The interaction to a hardware/peripheral can be conducted by sending a command to the CommandQueue of the target task.
        - Task 0: Template for future implementation `task_template`
          - A template for creating a task. Including creating the CommandQueue for communication and Task for command execution.
        - Task 1: Pulse Sensor Measurement `task_hr` 
          - Responsible for reading data from the pulse sensor and calculating heart rate and heart rate variability.
          - After the data reading and calculation finished, it will request `task_lcd` to update the wave and results in the display.
        - Task 2: LCD Display `task_lcd`
          - Responsible for updating the LCD display with the measurement information.
          - The LCD display is synchronized 
        - Task 3: Printing  `task_print`
          - Responsible for print the information thorough the UART terminal. The interrupt can send command to the task for information print.

    1. ### Interrupts
        - Data ready interrupt (FIFO)
          - The data are saved in the FIFO of the pulse sensor. After the FIFO is full, an interrupt will be generated, which will request the MCU to poll the data.
        - USER BTN
          - A button is used to allow user screenshot the measured results.
            - If a measurement is finished, after pushing the button, the LCD will do a screenshot.
            - There will be no screenshot if measurement is ongoing.
            - By pushing the button again, the measurement will resume immediately.

    1. ### Peripheral
        - Pulse sensor
        - LCD display
        - I2C communication

1. ## Important implementations
    Some important implementations are presented in the following parts.
    ### Task Functions
      ```c
      void task_template(INT stacd, void *exinf);
      void task_hr(INT stacd, void *exinf);
      void task_lcd(INT stacd, void *exinf);
      void task_print(INT stacd, void *exinf);
      ```
    ### Task creation
    - The call of μT-Kernel API for task creation is packaged into one create function.
    Some μT-Kernel APIs are abstracted into a simpler function. For these functions, a prefix of **`xmtk`** is added.
      ```c
      void xmtk_create_task_template();
      void xmtk_create_task_hr();
      void xmtk_create_task_lcd();
      void xmtk_create_task_print();
      ```

    ### Task starting
    - The call of μT-Kernel API for starting the task is packaged into one create function.
      ```c
      void xmtk_start_task_template();
      void xmtk_start_task_hr();
      void xmtk_start_task_lcd();
      void xmtk_start_task_print();
      ```

    ### Peripheral Configuration
    - peripheral initialization
      ```c
      // For I2C initialization
      void i2c_init();
      // For IO initialization
      void gpio_init();
      ```

    ### Interrupt Handlers
    - The interrupt handlers are used for register the interrupt into the μT-Kernel interrupt management system.
      ```c
      // Interrupt happens when USER_BTN1 is pushed.
      void xmtk_gpio_interrupt_handler(UINT intno);
      // Interrupt happens when data is ready in the pulse sensor. Data are saved in the FIFO of the sensor.
      void xmtk_hr_interrupt_handler(UINT intno);
      ```

    ### Register the Interrupt into μT-Kernel interrupt management
      - register the Interrupt
        ```c
        void xmtk_register_interrupt();
        ```

  1. ### Application main function `app.main.c`
        ```c
          EXPORT INT usermain(void)
          {
            tm_putstring((UB *)"Start User-main program.\n");

            /* Initialize and register the peripheral resources*/
            xmtk_register_interrupt();
            xmtk_enhance_i2c();

              /* Create tasks */
            xmtk_create_task_template();
            xmtk_create_task_print();
            xmtk_create_task_lcd();
            xmtk_create_task_hr();

            /* Start tasks */
            xmtk_start_task_template();
            xmtk_start_task_print();
            xmtk_start_task_lcd();
            xmtk_start_task_hr();

            tk_slp_tsk(TMO_FEVR);

            return 0;
          }
        ```

1. ## Knowhow during development
    1. ### μT-Kernel event-driven implementation
        - This document explains the implementation of an event-driven (command-driven) task communication in μT-Kernel. The communications and controls of tasks have the following characteristics:
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
              
        #### Implementations
        ##### Define the Command
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
          ##### Configuration of a task
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

            ##### Send a command
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

            - This document explains how the author implemented an event-driven (command-driven) communication between tasks in μT-Kernel RTOS. You can customize the `cmd_list_template_t` and the `Task_template` according to your specific requirements.

    1. ### μT-Kernel interrupt configuration
    - This document provides an overview of configuring and implementing interrupts in the μT-Kernel. It covers basic implementation and some experiences.
        #### About Interrupts 
        - External or internal events (like a hardware signal, a timer overflow, or an I/O operation) from peripherals usually ask the MCU to take some actions. For example, asking the MCU to read data from a sensor if data is available or changing the status if a user pushes a button.
        - To implement functions that rely on the interaction with interrupts, the following components are needed:
            - Initialization of interrupt resources.
            - Interrupt handler.
            - Interrupt configuration.
            - Registering the interrupt handler to the system.

        #### Registering Interrupt Handlers with Hal
        - Below is an example of using the `cyhal` library to implement an interrupt interaction for a user button push.
        - Initialize interrupt resources
            ```c
            cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT, CYBSP_USER_BTN_DRIVE, CYBSP_BTN_OFF);
            ```
        - Define the interrupt handler
            ```c
            static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event)
            {
                // Implement the desired functionality
            }
            ```
        - Configure and register the interrupt handler
            ```c
            cyhal_gpio_callback_data_t gpio_btn_callback_data;
            gpio_btn_callback_data.callback = gpio_interrupt_handler;
            cyhal_gpio_register_callback(CYBSP_USER_BTN, &gpio_btn_callback_data);
            ```
        - Enable the interrupt
            ```c
            cyhal_gpio_enable_event(CYBSP_USER_BTN, CYHAL_GPIO_IRQ_FALL, GPIO_INTERRUPT_PRIORITY, true);
            ```
        - After setting up the above, every time the user pushes the button, the `gpio_interrupt_handler` function will be called.

        #### Registering Interrupt Handlers with μT-Kernel
        - μT-Kernel provides interrupt management functions. However, it is a bit more complex than using the Hal library in the implementation of the `interrupt_handler`. Here are the steps and some points to consider:
        - Initialize interrupt resources (same as Hal)
            ```c
            cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT, CYBSP_USER_BTN_DRIVE, CYBSP_BTN_OFF);
            ```
        - Define the interrupt handler (<strong>different</strong>). Pay attention to the following steps:
            - Clear the interrupt `Cy_GPIO_ClearInterrupt(GPIO_PRT21, 4)`
                - Without this step, the interrupt handler will be continuously executed.
            - (Optional) Check whether the interrupt is from the expected pin.
                - For the `Infineon XMC7200` board, for example, pin21.3 and pin21.4 both generate the interrupt to IO interrupt number `ioss_interrupts_gpio_21_IRQn`. So, you need to determine which pin the interrupt originated from. The function `Cy_GPIO_GetInterruptStatus(GPIO_PRT21, 4)` can be used to check if the interrupt comes from pin21.4.
            - Below is an example of an interrupt handler that sends commands to the `print task` for printing interrupt information if BTN1 is pushed.
                ```c
                void xmtk_gpio_interrupt_handler(UINT intno)
                {
                    Cy_GPIO_ClearInterrupt(GPIO_PRT21, 4);
                    NVIC_ClearPendingIRQ(intno);
                    task_print_cmd_t print_cmd = {.cmd = TASK_PRINT_PRINT, .msg = "BTN1 pushed", .value = "interrupt value to print"};
                    xmtk_send_command(CommandQueue_print, print_cmd);
                }
                ```
            - Configure and register the interrupt handler (<strong>different</strong>)
                - Use the μT-Kernel API `tk_def_int` to register the interrupt and interrupt handler.
                - Be sure to check the interrupt number of the expected interrupt pin. For example, on the `Infineon XMC7200` board, BTN1 is on pin21.4, and the corresponding interrupt number is defined as `ioss_interrupts_gpio_21_IRQn`.
                - Example of interrupt registration using the μT-Kernel API `tk_def_int`:
                    ```c
                    T_DINT t_dint = {
                        .intatr = TA_HLNG,
                        .inthdr = xmtk_gpio_interrupt_handler};
                    tk_def_int(ioss_interrupts_gpio_21_IRQn, &t_dint);
                    ```
        - Enable the interrupt
            ```c
            cyhal_gpio_enable_event(CYBSP_USER_BTN, CYHAL_GPIO_IRQ_FALL, GPIO_INTERRUPT_PRIORITY, true);
            ```

1. ## Project build
    To compile and run the firmware, follow these steps:
    1. Connect the pulse sensor and LCD display to the development board.
    2. Install the required software:
    3. Configure the project settings for your development environment.
    4. Build the project.
    5. Flash the firmware onto the Infineon KIT_XMC72_EVK development board.
    6. Observe the measurement information displayed on the LCD display.
    - The step-by-step clone and build of the project is in [1_010_EN_Clone_Build_Project.md](./1_010_EN_Clone_Build_Project.md)

1. ## Additional information
    - How the stress is measured: https://my.clevelandclinic.org/health/symptoms/21773-heart-rate-variability-hrv

1. ## References
    - Infineon KIT_XMC72_EVK Documentation: https://www.infineon.com/dgdl/Infineon-KIT_XMC72_EVK-V1.0.pdf?fileId=

1. ## Clarifications
    - he specific details of the existing software used, including the name, rights holder, method of acquisition, and its functions in the provided documentation.
        - [Software](#software-overview)
        - - peripheral
            - The following peripheral drivers are a clone of the following repository:
                - `HR_sensor`: https://github.com/eepj/stm32-max30102
                - `LCD_ssd1306`:https://github.com/4ilo/ssd1306-stm32HAL/tree/master 
    - The existing software used in the project will be made available to the organizers free of charge, ensuring it remains accessible until approximately one week after the contest's award ceremony for evaluation purposes.
    - In compliance with the contest rules, we confirmed that all necessary copyright and rights-related processing for the used existing software has been completed.

