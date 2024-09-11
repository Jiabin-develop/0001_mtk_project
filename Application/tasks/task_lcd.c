#include "task_lcd.h"
#include <math.h>

EXPORT ID CommandQueue_lcd;
EXPORT ID Task_lcd;

IMPORT cyhal_i2c_t mI2C;
uint8_t counter = 0;
char str_show[18];
bool updating = true;
bool lcd_on = true;

T_CTSK ctsk_lcd = {
    // Task creation information
    .itskpri = 9,
    .stksz = 1024,
    .task = task_lcd,
    .tskatr = TA_HLNG | TA_RNG3,
};

T_CMBF cmbf_task_lcd = {
    // Message buffer creation information
    .mbfatr = TA_TFIFO | TA_MFIFO,
    .bufsz = 1024,
    .maxmsz = sizeof(command_lcd_t),
};

static void lcd_draw_pulse();
static void lcd_initial();
static void lcd_turn_off();
static void lcd_turn_on();

void xmtk_create_task_lcd()
{
    CommandQueue_lcd = tk_cre_mbf(&cmbf_task_lcd);
    Task_lcd = tk_cre_tsk(&ctsk_lcd);
}

void xmtk_start_task_lcd()
{
    tk_sta_tsk(Task_lcd, 0);
}

void task_lcd(INT stacd, void *exinf)
{
    command_lcd_t lcd_cmd;
    INT len = 0;
    tm_printf((UB *)"task task_lcd on\n");
    lcd_initial();
    while (1)
    {
        len = xmtk_receive_command(CommandQueue_lcd, lcd_cmd);
        if (len > 0)
        {
            tm_printf((UB *)"HR: got cmd: %d, msg: %s, value: %s\n", lcd_cmd.cmd, lcd_cmd.msg, lcd_cmd.value);
            switch (lcd_cmd.cmd)
            {
            case TASK_LCD_TEST:
                // draw_hr(atoi(task_lcd_cmd.value));
                break;
            case TASK_LCD_UPDATE_PULSE:
                lcd_draw_pulse();
                break;
            case TASK_LCD_CHANGE_UPDATE:
                if (updating == false)
                {
                    updating = true;
                    command_hr_t start_sensor = {.cmd = TASK_HR_START_SENSOR};
                    xmtk_send_command(CommandQueue_hr, start_sensor);
                }
                else if (pulse_buffer.heartbeat > 0)
                {
                    updating = false;
                    lcd_draw_pulse();
                    command_hr_t stop_sensor = {.cmd = TASK_HR_STOP_SENSOR};
                    xmtk_send_command(CommandQueue_hr, stop_sensor);
                }
                break;
            case TASK_LCD_ON:
                lcd_turn_on();
                break;
            case TASK_LCD_OFF:
                lcd_turn_off();
                break;
            default:
                break;
            }
        }
    }
}

void lcd_initial()
{
    uint8_t result_oled_init;
    /* example ss1306 application Init I2C */
    result_oled_init = ssd1306_Init(&mI2C);
    tm_printf((UB *)"oled_init() results : %d.\n", result_oled_init);
}
void lcd_turn_off()
{
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen(&mI2C);
    lcd_on = false;
    return;
}

void lcd_turn_on()
{
    lcd_on = true;
    return;
}

void lcd_draw_pulse()
{
    if (lcd_on == false)
    {
        lcd_turn_off();
        return;
    }

    counter++;
    if (updating == true)
    {
        ssd1306_Fill(Black);
        ssd1306_SetCursor(0, 0);
        ssd1306_WriteString(" Stress Kit - MTK ", Font_7x10, White);
        ssd1306_SetCursor(0, 53);
        for (int i = 0; i < 128; i++)
        {
            ssd1306_DrawPixel(i, 11, White);
        }
        sprintf(str_show, "------<J.W.>-----");
        ssd1306_WriteString(str_show, Font_7x10, White);
    }

    if (updating == false)
    {
        ssd1306_SetCursor(0, 0);
        ssd1306_WriteString(" Stress Kit - MTK ", Font_7x10, White);
        ssd1306_SetCursor(0, 53);
        for (int i = 0; i < 128; i++)
        {
            ssd1306_DrawPixel(i, 11, White);
        }
        sprintf(str_show, "screenshot-<J.W.>");
        ssd1306_WriteString(str_show, Font_7x10, White);
        ssd1306_UpdateScreen(&mI2C);
        return;
    }

    if (pulse_buffer.touching == 0)
    {
        ssd1306_SetCursor(0, 22);
        sprintf(str_show, "Info:");
        ssd1306_WriteString(str_show, Font_7x10, White);
        ssd1306_SetCursor((counter * 4) % 40 < 20 ? (counter * 4) % 40 : 20 - ((counter * 4) % 40 - 20), 34);
        sprintf(str_show, "--Not touching");
        ssd1306_WriteString(str_show, Font_7x10, White);
        ssd1306_UpdateScreen(&mI2C);
        return;
    }

    if (pulse_buffer.current_status == 0)
    {
        uint32_t shift_size = 40;
        ssd1306_SetCursor((counter * 4) % (2 * shift_size) < shift_size ? (counter * 4) % (2 * shift_size) : shift_size - ((counter * 4) % (2 * shift_size) - shift_size), 15);
        sprintf(str_show, "Waiting...");
        ssd1306_WriteString(str_show, Font_7x10, White);
    }
    else
    {
        int score = 0;
        if (pulse_buffer.heartbeat_variation >= 60)
        {
            score = 0;
        }
        else
        {
            score = (1 - pulse_buffer.heartbeat_variation / 60.0) * 9;
        }
        ssd1306_SetCursor(5, 14);
        sprintf(str_show, "%1d", score);
        ssd1306_WriteString(str_show, Font_16x26, White);

        ssd1306_SetCursor(27, 14);
        sprintf(str_show, "HR");
        ssd1306_WriteString(str_show, Font_7x10, White);

        ssd1306_SetCursor(41, 15);
        sprintf(str_show, "%-3d", (int)(pulse_buffer.heartbeat));
        ssd1306_WriteString(str_show, Font_11x18, White);

        ssd1306_SetCursor(76, 14);
        sprintf(str_show, "Va");
        ssd1306_WriteString(str_show, Font_7x10, White);

        ssd1306_SetCursor(91, 15);
        sprintf(str_show, "%-3d", (int)(pulse_buffer.heartbeat_variation));
        ssd1306_WriteString(str_show, Font_11x18, White);
    }

    for (int i = 0; i < 128; i++)
    {
        for (int j = 0; j < 14; j++)
        {
            if (fabs(i - pulse_buffer.end / 2) <= 1)
            {
                ssd1306_DrawPixel(i, 52 - 16, Black);
                ssd1306_DrawPixel(i, 52 - (15 - j), Black);
                // ssd1306_DrawPixel(i, 52 - 1, Black);
                //  ssd1306_DrawPixel(i, 64 - 1, Black);
            }
            else if (j <= pulse_buffer.pulse_norm[i * 2])
            {

                ssd1306_DrawPixel(i, 52 - (15 - j), Black);
                // ssd1306_DrawPixel(i, 62 - (15 - j), White);
            }
            else
            {
                ssd1306_DrawPixel(i, 52 - (15 - j), White);
            }
        }
    }

    ssd1306_UpdateScreen(&mI2C);
    return;
}
