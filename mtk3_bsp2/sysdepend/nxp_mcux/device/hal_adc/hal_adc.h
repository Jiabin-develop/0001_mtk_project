/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.0 BSP 2.0
 *
 *    Copyright (C) 2024 by Ken Sakamura.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 *
 *    Released by TRON Forum(http://www.tron.org) at 2024/02.
 *
 *----------------------------------------------------------------------
 */

#ifndef	_DEV_HAL_ADC_H_
#define	_DEV_HAL_ADC_H_
/*
 *	hal_adc.h
 *	A/DC device driver (NXP MCUXpresso)
*/

/*----------------------------------------------------------------------
 * A/DC Device
 */
#define DEV_HAL_ADC1	0

/*----------------------------------------------------------------------
 * Attribute data
 */
#define TDN_HAL_ADC_MODE	(-100)	// A/DC Mode

/*----------------------------------------------------------------------
 * Device driver initialization and registration
 */

IMPORT ER dev_init_hal_adc( UW unit, ADC_Type *base);

#endif	/* _DEV_HAL_ADC_H_ */