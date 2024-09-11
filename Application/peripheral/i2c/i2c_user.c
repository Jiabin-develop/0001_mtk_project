#include "i2c_user.h"

EXPORT cyhal_i2c_t mI2C;
EXPORT ID semid;

void i2c_init()
{
    cy_rslt_t result;
    /*Configure clock settings for KIT_XMC72_EVK */
    cyhal_clock_t clock_fll, clock_hf, clock_peri;
    result = cyhal_clock_reserve(&clock_hf, &CYHAL_CLOCK_HF[0]);
    result = cyhal_clock_reserve(&clock_fll, &CYHAL_CLOCK_FLL);
    if (result == CY_RSLT_SUCCESS)
    {
        result = cyhal_clock_set_source(&clock_hf, &clock_fll);
    }
    /* Set divider to 1 for Peripheral Clock */
    result = cyhal_clock_reserve(&clock_peri, CYHAL_CLOCK_PERI);
    if (result == CY_RSLT_SUCCESS)
    {
        result = cyhal_clock_set_divider(&clock_peri, 1);
    }

    cyhal_i2c_cfg_t mI2C_cfg =
        {
            .is_slave = false,
            .address = 0,
            .frequencyhal_hz = I2C_FREQ,
        };

    /* Init I2C master */
    result = cyhal_i2c_init(&mI2C, CYBSP_I2C_SDA, CYBSP_I2C_SCL, NULL);
    /* Configure I2C Master */
    result = cyhal_i2c_configure(&mI2C, &mI2C_cfg);
}

void xmtk_enhance_i2c()
{
    T_CSEM t_csem = {
        .sematr = TA_TFIFO | TA_FIRST,
        .isemcnt = 1,
        .maxsem = 1,
    };
    semid = tk_cre_sem(&t_csem);
}

cy_rslt_t i2c_write(cyhal_i2c_t *obj, uint16_t address, uint16_t mem_addr, uint16_t mem_addr_size, const uint8_t *data, uint16_t size, uint32_t timeout)
{
    cy_rslt_t cy_rst;
    tk_wai_sem(semid, 1, TMO_FEVR);
    cy_rst = cyhal_i2c_master_mem_write(obj, address, mem_addr, mem_addr_size, data, size, timeout);
    tk_sig_sem(semid, 1);
    return cy_rst;
}
cy_rslt_t i2c_read(cyhal_i2c_t *obj, uint16_t address, uint16_t mem_addr, uint16_t mem_addr_size, uint8_t *data, uint16_t size, uint32_t timeout)
{
    cy_rslt_t cy_rst;
    tk_wai_sem(semid, 1, TMO_FEVR);
    cy_rst = cyhal_i2c_master_mem_read(obj, address, mem_addr, mem_addr_size, data, size, timeout);
    tk_sig_sem(semid, 1);
    return cy_rst;
}
