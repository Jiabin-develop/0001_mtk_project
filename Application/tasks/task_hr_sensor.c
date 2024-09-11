#include "task_hr_sensor.h"

EXPORT ID CommandQueue_hr;
EXPORT ID Task_hr;

IMPORT max30102_t max30102;
IMPORT cyhal_i2c_t mI2C;

pulse_buffer_t pulse_buffer = {.start = -1, .end = -1, .touching = 1};

T_CTSK ctsk_hr = {
    // Task creation information
    .itskpri = 10,
    .stksz = 1024,
    .task = task_hr,
    .tskatr = TA_HLNG | TA_RNG3,
};

T_CMBF cmbf_task_hr = {
    // Message buffer creation information
    .mbfatr = TA_TFIFO | TA_MFIFO,
    .bufsz = 1024,
    .maxmsz = sizeof(command_hr_t),
};

static void hr_sensor_initial();
static void hr_interrupt_process();
static void xmtk_timer_idle_handler();
static void xmtk_timer_idle_reset(uint32_t idle_ms);
static void xmtk_timer_idle_stop();
static void xmtk_create_timer_idle();
static void data_process();

ID idle_timer;
uint32_t idle_ms_no_touching = 10000;
uint32_t idle_ms_screenshot = 30000;

void xmtk_create_task_hr()
{
    CommandQueue_hr = tk_cre_mbf(&cmbf_task_hr);
    Task_hr = tk_cre_tsk(&ctsk_hr);
}

void xmtk_start_task_hr()
{
    tk_sta_tsk(Task_hr, 0);
}

void task_hr(INT stacd, void *exinf)
{
    command_hr_t hr_cmd;
    INT len = 0;
    tm_printf((UB *)"task task_hr on\n");
    hr_sensor_initial();
    xmtk_create_timer_idle();
    xmtk_timer_idle_reset(idle_ms_no_touching);
    while (1)
    {
        tm_printf((UB *)"task task_hr on\n");
        while (1)
        {
            len = xmtk_receive_command(CommandQueue_hr, hr_cmd);
            if (len > 0)
            {
                tm_printf((UB *)"HR: got cmd: %d, msg: %s, value: %s\n", hr_cmd.cmd, hr_cmd.msg, hr_cmd.value);
                switch (hr_cmd.cmd)
                {
                case TASK_HR_INTERRUPT:
                    hr_interrupt_process();
                    break;
                case TASK_HR_READ_FIFO:
                    tm_printf((UB *)"int_hr task: fifo_ready");
                    pulse_buffer.num_new = max30102_read_fifo_N(&max30102, 16, pulse_buffer.pulse_raw, &(pulse_buffer.start), &(pulse_buffer.end), PULSE_RAW_LENGTH);
                    tm_printf((UB *)"HR: pulse buffer, start = %d, end = %d, num = %d, num_update=%d\n", pulse_buffer.start, pulse_buffer.end, pulse_buffer.end < pulse_buffer.start ? PULSE_RAW_LENGTH : (pulse_buffer.end - pulse_buffer.start + 1), pulse_buffer.num_new);
                    data_process();
                    command_lcd_t lcd_update_pulse = {.cmd = TASK_LCD_UPDATE_PULSE};
                    xmtk_send_command(CommandQueue_lcd, lcd_update_pulse);
                    break;
                case TASK_HR_STOP_SENSOR:
                    max30102_shutdown(&max30102, 1);
                    xmtk_timer_idle_reset(idle_ms_screenshot);
                    break;
                case TASK_HR_START_SENSOR:
                    max30102_shutdown(&max30102, 0);
                    xmtk_timer_idle_reset(idle_ms_no_touching);
                    break;
                default:
                    break;
                }
            }
        }
    }
}

static void xmtk_timer_idle_handler()
{
    command_lcd_t cmd_lcd_off = {.cmd = TASK_LCD_OFF};
    xmtk_send_command(CommandQueue_lcd, cmd_lcd_off);
    command_hr_t cmd_hr_stop = {.cmd = TASK_HR_STOP_SENSOR};
    xmtk_send_command(CommandQueue_hr, cmd_hr_stop);
}

static void xmtk_timer_idle_reset(uint32_t idle_ms)
{
    tk_sta_alm(idle_timer, idle_ms);
}
static void xmtk_timer_idle_stop()
{
    tk_stp_alm(idle_timer);
}
static void xmtk_create_timer_idle()
{
    T_CALM t_calm =
        {
            .exinf = 0,
            .almatr = TA_HLNG,
            .almhdr = xmtk_timer_idle_handler,
        };
    idle_timer = tk_cre_alm(&t_calm);
}

static void hr_sensor_initial()
{
    // Initiation
    max30102_init(&max30102, &mI2C);
    max30102_reset(&max30102);
    max30102_clear_fifo(&max30102);
    max30102_set_fifo_config(&max30102, max30102_smp_ave_32, 1, 2);
    // Sensor settings
    max30102_set_led_pulse_width(&max30102, max30102_pw_18_bit);
    max30102_set_adc_resolution(&max30102, max30102_adc_16384);
    max30102_set_sampling_rate(&max30102, max30102_sr_50);
    max30102_set_led_current_1(&max30102, 6);
    max30102_set_led_current_2(&max30102, 6);

    // Enter pulse measurement mode
    max30102_set_mode(&max30102, max30102_heart_rate);
    max30102_set_a_full(&max30102, 1);

    uint8_t en_reg[2] = {0};
    max30102_read(&max30102, 0x00, en_reg, 1);
}

void hr_interrupt_process()
{
    uint8_t fifo_int_status = 0;
    max30102_read(&max30102, MAX30102_INTERRUPT_STATUS_1, &fifo_int_status, 1);
    tm_printf((UB *)"int_hr: MAX30102_INTERRUPT_STATUS_1=%d\n", fifo_int_status);

    command_hr_t cmd_hr_fifo = {.cmd = TASK_HR_READ_FIFO, .msg = "fifo interrupt", .value = "interrupt value to print"};
    xmtk_send_command(CommandQueue_hr, cmd_hr_fifo);
}

static void data_process()
{
    uint32_t updating = 0;
    float dc = 0;

    // Smooth the sampeles, with 10 samples as average.
    for (int i = pulse_buffer.num_new - 1; i >= 0; i--)
    {
        if (pulse_buffer.end - i >= 0)
        {
            updating = pulse_buffer.end - i;
        }
        else
        {
            updating = PULSE_RAW_LENGTH + pulse_buffer.end - i;
        }
        dc = 0;
        for (int j = 0; j < 10; j++)
        {
            uint32_t index_get = 0;
            if (updating >= j)
            {
                index_get = updating - j;
            }
            else
            {
                index_get = updating + PULSE_RAW_LENGTH - j;
            }
            dc += pulse_buffer.pulse_raw[index_get];
        }
        if (dc > 0)
        {
            dc /= 10;
        }
        pulse_buffer.pulse_smooth[updating] = dc;
        if (dc < PULSE_TOUCH_THRESHOLD)
        {
            pulse_buffer.touching = 0;
        }
        else
        {
            pulse_buffer.touching = 1;
        }
    }

    // remove the DC signal, 50 Hz, 50 samples to get the average DC, cut the DC signal of the previous 50 samples
    for (int i = pulse_buffer.num_new - 1; i >= 0; i--)
    {
        if (pulse_buffer.end - i >= 0)
        {
            updating = pulse_buffer.end - i;
        }
        else
        {
            updating = PULSE_RAW_LENGTH + pulse_buffer.end - i;
        }
        // tm_printf((UB *)"updating=%d\n", updating);
        dc = 0;
        for (int j = 0; j < 50; j++)
        {
            uint32_t index_get = 0;
            if (updating >= j)
            {
                index_get = updating - j;
            }
            else
            {
                index_get = updating + PULSE_RAW_LENGTH - j;
            }
            dc += pulse_buffer.pulse_smooth[index_get];
        }
        if (dc > 0)
        {
            dc /= 50;
        }
        pulse_buffer.pulse_AC[updating] = pulse_buffer.pulse_smooth[updating] - dc;
        // tm_printf((UB *)"%d {pulse_ac}%d\n", updating, (int32_t)(pulse_buffer.pulse_AC[updating]));
    }

    // normalization to a certain range
    float max = 0, min = 0;
    float display_max = 13;
    for (int i = pulse_buffer.num_new - 1; i >= 0; i--)
    {
        if (pulse_buffer.end - i >= 0)
        {
            updating = pulse_buffer.end - i;
        }
        else
        {
            updating = PULSE_RAW_LENGTH + pulse_buffer.end - i;
        }
        max = 0;
        min = 0;
        for (int j = 0; j < 70; j++)
        {
            uint32_t index_get = 0;
            if (updating >= j)
            {
                index_get = updating - j;
            }
            else
            {
                index_get = updating + PULSE_RAW_LENGTH - j;
            }
            if (pulse_buffer.pulse_AC[index_get] > max)
            {
                max = pulse_buffer.pulse_AC[index_get];
            }
            if (pulse_buffer.pulse_AC[index_get] < min)
            {
                min = pulse_buffer.pulse_AC[index_get];
            }
        }
        if (max - min > 0)
        {
            pulse_buffer.pulse_norm[updating] = display_max * (pulse_buffer.pulse_AC[updating] - min) / (max - min) + 1;
        }
        else
        {
            pulse_buffer.pulse_norm[updating] = 0;
        }

        // tm_printf((UB *)"%d {pulse_norm}%d\n", updating, (int32_t)(pulse_buffer.pulse_norm[updating]));
    }

    if (pulse_buffer.touching == 0)
    {
        pulse_buffer.heartbeat = 0;
        pulse_buffer.heartbeat_variation = 0;
        for (int i = 0; i < PULSE_RAW_LENGTH; i++)
        {
            pulse_buffer.pulse_raw[i] = 0;
            pulse_buffer.pulse_norm[i] = 15;
            pulse_buffer.pulse_AC[i] = 0;
        }
        pulse_buffer.current_status = 0;
        return;
    }
    xmtk_timer_idle_reset(idle_ms_no_touching);

    float hrs[PULSE_RAW_LENGTH];
    int num_hrs = 0;
    int last_peak = 0;
    for (int i = 0; i < PULSE_RAW_LENGTH - 1; i++)
    {
        int current_index, previous_index;
        current_index = pulse_buffer.end - i >= 0 ? pulse_buffer.end - i : PULSE_RAW_LENGTH + pulse_buffer.end - i;
        previous_index = pulse_buffer.end - i - 1 >= 0 ? pulse_buffer.end - i - 1 : PULSE_RAW_LENGTH + pulse_buffer.end - i - 1;
        // tm_printf((UB *)"ibis: cross=%d\n", (int)(pulse_buffer.pulse_AC[current_index] * pulse_buffer.pulse_AC[previous_index]));
        // if (pulse_buffer.pulse_AC[current_index] < 0 && pulse_buffer.pulse_AC[previous_index] > 0)
        if (pulse_buffer.pulse_norm[current_index] < (display_max - display_max / 4) && pulse_buffer.pulse_norm[previous_index] > (display_max - display_max / 4))
        {
            if (last_peak == 0)
            {
                last_peak = i;
            }
            else
            {
                hrs[num_hrs] = 60.0 * PULSE_FS_HZ / (i - last_peak);
                tm_printf((UB *)"ibis: last=[%d] i=[%d], hr=%d\n", last_peak, i, (int)hrs[num_hrs]);
                num_hrs++;
                last_peak = i;
            }
        }
    }
    // tm_printf((UB *)"ibis: num_hrs =%d\n", num_hrs);
    if (num_hrs < 3)
    {
        return;
    }
    float hrs_sum = hrs[0];
    float hrv_sum = 0;
    for (int i = 0; i < num_hrs - 1; i++)
    {
        hrs_sum += hrs[i];
        hrv_sum += fabs(60 / hrs[i] - 60 / hrs[i + 1]);
        if (fabs(hrs[i] - hrs[i + 1]) > 5)
        {
            return;
        }
    }
    pulse_buffer.current_status = 1;
    pulse_buffer.heartbeat = hrs_sum / (num_hrs);
    pulse_buffer.heartbeat_variation = 1000 * hrv_sum / (num_hrs - 1);

    tm_printf((UB *)"ibis: num_hrs =%d, hr=%d, hrv=%d\n", num_hrs, pulse_buffer.heartbeat, pulse_buffer.heartbeat_variation);
}