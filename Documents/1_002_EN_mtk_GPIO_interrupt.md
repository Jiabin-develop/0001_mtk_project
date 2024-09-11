# Interrupt Configuration and Implementation in μT-Kernel

## Introduction
This document provides an overview of configuring and implementing interrupts in the μT-Kernel. It covers basic implementation and some experiences.

## Table of Contents
- [About Interrupts](#section-1)
- [Registering Interrupt Handlers with Hal](#section-2)
- [Registering Interrupt Handlers with μT-Kernel](#section-3)

## About Interrupts <a name="section-1"></a>
- External or internal events (like a hardware signal, a timer overflow, or an I/O operation) from peripherals usually ask the MCU to take some actions. For example, asking the MCU to read data from a sensor if data is available or changing the status if a user pushes a button.
- To implement functions that rely on the interaction with interrupts, the following components are needed:
    - Initialization of interrupt resources.
    - Interrupt handler.
    - Interrupt configuration.
    - Registering the interrupt handler to the system.

## Registering Interrupt Handlers with Hal<a name="section-2"></a>
Below is an example of using the `cyhal` library to implement an interrupt interaction for a user button push.
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
After setting up the above, every time the user pushes the button, the `gpio_interrupt_handler` function will be called.

## Registering Interrupt Handlers with μT-Kernel<a name="section-3"></a>
μT-Kernel provides interrupt management functions. However, it is a bit more complex than using the Hal library in the implementation of the `interrupt_handler`. Here are the steps and some points to consider:
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