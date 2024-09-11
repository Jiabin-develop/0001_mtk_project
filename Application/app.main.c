/*
 * app.main.c
 *
 *  Created on: Jun 26, 2024
 *      Author: WANGJiabin
 */

#include "string.h"
#include "common.h"


/* usermain関数 */
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
