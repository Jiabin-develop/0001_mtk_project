#include "user_interrupt.h"

cyhal_gpio_callback_data_t gpio_btn_callback_data;
cyhal_gpio_callback_data_t gpio_hr_callback_data;

#define GPIO_INTERRUPT_PRIORITY (6u)

/*******************************************************************************
 * Function Prototypes
 *******************************************************************************/

void gpio_init()
{
    /* Initialize the user button */
    cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT, CYBSP_USER_BTN_DRIVE, CYBSP_BTN_OFF);
    /* Enable interrupt */
    cyhal_gpio_enable_event(CYBSP_USER_BTN, CYHAL_GPIO_IRQ_FALL, GPIO_INTERRUPT_PRIORITY, true);

    /* Initialize the user button */
    cyhal_gpio_init(P13_6, CYHAL_GPIO_DIR_INPUT, CYBSP_USER_BTN_DRIVE, CYBSP_BTN_OFF);
    /* Enable interrupt */
    cyhal_gpio_enable_event(P13_6, CYHAL_GPIO_IRQ_FALL, GPIO_INTERRUPT_PRIORITY, true);

    /* Initialize the HR sensor interruption */
    cyhal_gpio_init(CYBSP_D8, CYHAL_GPIO_DIR_INPUT, CYBSP_USER_BTN_DRIVE, CYBSP_BTN_OFF);
    /* Enable interrupt */
    cyhal_gpio_enable_event(CYBSP_D8, CYHAL_GPIO_IRQ_FALL, GPIO_INTERRUPT_PRIORITY, true);
}

void xmtk_gpio_interrupt_handler(UINT intno)
{
    tm_printf((UB *)" user BTN1 pushed, 21.3= %d, 21.4=%d\n", Cy_GPIO_GetInterruptStatus(GPIO_PRT21, 3), Cy_GPIO_GetInterruptStatus(GPIO_PRT21, 4));
    Cy_GPIO_ClearInterrupt(GPIO_PRT21, 4);
    NVIC_ClearPendingIRQ(intno);
    tm_printf((UB *)" user BTN1 pushed intno=%d\n", intno);
    cmd_print_t print_cmd = {.cmd = TASK_PRINT_PRINT, .msg = "from btn", .value = "interrupt value to print"};
    xmtk_send_command(CommandQueue_print, print_cmd);
    command_lcd_t lcd_cmd = {.cmd = TASK_LCD_CHANGE_UPDATE, .msg = "from btn", .value = "interrupt value to change lcd update"};
    xmtk_send_command(CommandQueue_lcd, lcd_cmd);
}

uint32_t last_button_pressed_tick = 0;
uint32_t current_button_pressed_tick = 0;
void xmtk_btn_interrupt_handler(UINT intno)
{
    SYSTIM pk_time_current;
    tk_get_otm(&pk_time_current);
    current_button_pressed_tick = pk_time_current.lo;
    tm_printf((UB *)" user BTN pushed\n");
    Cy_GPIO_ClearInterrupt(GPIO_PRT13, 6);
    NVIC_ClearPendingIRQ(intno);
    tm_printf((UB *)" user BTN pushed intno=%d\n", intno);

    // Debounce
    if (
        (current_button_pressed_tick < last_button_pressed_tick) || (current_button_pressed_tick - last_button_pressed_tick) < 200)
    {
        last_button_pressed_tick = current_button_pressed_tick;
        tm_printf((UB *)" user BTN: debounce: current_tick =%d, last_tick=%d\n", current_button_pressed_tick, last_button_pressed_tick);
        return;
    }

    tm_printf((UB *)" user BTN: works: current_tick =%d, last_tick=%d\n", current_button_pressed_tick, last_button_pressed_tick);

    cmd_print_t print_cmd = {.cmd = TASK_PRINT_PRINT, .msg = "from btn", .value = "interrupt value to print"};
    xmtk_send_command(CommandQueue_print, print_cmd);

    command_lcd_t cmd_lcd_on = {.cmd = TASK_LCD_ON};
    xmtk_send_command(CommandQueue_lcd, cmd_lcd_on);

    command_lcd_t cmd_lcd_update_change = {.cmd = TASK_LCD_CHANGE_UPDATE};
    xmtk_send_command(CommandQueue_lcd, cmd_lcd_update_change);

    command_hr_t cmd_hr_on = {.cmd = TASK_HR_START_SENSOR};
    xmtk_send_command(CommandQueue_hr, cmd_hr_on);

    last_button_pressed_tick = current_button_pressed_tick;
}

void xmtk_hr_interrupt_handler(UINT intno)
{
    Cy_GPIO_ClearInterrupt(GPIO_PRT10, 5);
    NVIC_ClearPendingIRQ(intno);
    tm_printf((UB *)" gpio_interrupt_handler\n");
    command_hr_t hr_cmd = {.cmd = TASK_HR_INTERRUPT, .msg = "HR interrupt", .value = "interrupt value to print"};
    xmtk_send_command(CommandQueue_hr, hr_cmd);
}

void xmtk_register_interrupt()
{
    T_DINT t_dint = {
        .intatr = TA_HLNG,
        .inthdr = xmtk_gpio_interrupt_handler};
    tk_def_int(ioss_interrupts_gpio_21_IRQn, &t_dint);

    T_DINT t_dint_btn = {
        .intatr = TA_HLNG,
        .inthdr = xmtk_btn_interrupt_handler};
    tk_def_int(ioss_interrupts_gpio_13_IRQn, &t_dint_btn);

    T_DINT t_dint_hr = {
        .intatr = TA_HLNG,
        .inthdr = xmtk_hr_interrupt_handler};
    tk_def_int(ioss_interrupts_gpio_10_IRQn, &t_dint_hr);
}
