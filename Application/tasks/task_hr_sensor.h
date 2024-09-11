#pragma once

#include "../common.h"
typedef enum
{
    TASK_HR_TEST,
    TASK_HR_INTERRUPT,
    TASK_HR_READ_FIFO,
    TASK_HR_START_SENSOR,
    TASK_HR_STOP_SENSOR,
} cmd_list_hr_t;

typedef struct
{
    cmd_list_hr_t cmd;
    char msg[20];
    char *value;
} command_hr_t;

#define PULSE_FS_HZ 50
#define PULSE_RAW_LENGTH 256
#define PULSE_TOUCH_THRESHOLD 18000
typedef struct
{
    uint32_t pulse_raw[PULSE_RAW_LENGTH];
    float pulse_AC[PULSE_RAW_LENGTH];
    float pulse_smooth[PULSE_RAW_LENGTH];
    float pulse_norm[PULSE_RAW_LENGTH];
    int32_t start;
    int32_t end;
    uint32_t num_new;
    uint8_t touching;
    uint8_t current_status; // 0: not touched, 1: touching, 2: touched-measuring, 3: touchedd-get-data
    uint16_t heartbeat;
    uint16_t heartbeat_variation;

} pulse_buffer_t;
extern pulse_buffer_t pulse_buffer;

void task_hr(INT stacd, void *exinf);
void xmtk_create_task_hr();
void xmtk_start_task_hr();
