#pragma once

#include "../../common.h"

/* I2C bus frequency */
#define I2C_FREQ (400000UL)
/* Packet positions */
#define PACKET_SOP_POS (0UL)
#define PACKET_CMD_POS (1UL)
#define PACKET_EOP_POS (2UL)

/* Start and end of packet markers */
#define PACKET_SOP (0x01UL)
#define PACKET_EOP (0x17UL)

/* Packet size */
#define PACKET_SIZE (3UL)

typedef struct
{
    uint8_t add;
    uint8_t data;
} i2c_packet_t;

void i2c_init();
void xmtk_enhance_i2c();

cy_rslt_t i2c_write(cyhal_i2c_t *obj, uint16_t address, uint16_t mem_addr, uint16_t mem_addr_size, const uint8_t *data, uint16_t size, uint32_t timeout);
cy_rslt_t i2c_read(cyhal_i2c_t *obj, uint16_t address, uint16_t mem_addr, uint16_t mem_addr_size, uint8_t *data, uint16_t size, uint32_t timeout);