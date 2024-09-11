### Markers:
- 2024/06/22 Start creating the project
- 2024/06/23 Read the documentation [【実習】μT-Kernel 入門 (協力：ルネサス エレクトロニクス )](https://www.tron.org/ja/wp-content/uploads/sites/2/2018/04/TEF071-W003-171121_02.pdf)
- 2024/06/29 
    - Tasks created, achived task communication and synchronization, using command
    - Start building the interface for the LCD 
        - [LCD online store](https://www.amazon.co.jp/gp/product/B0C9X72TPM/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
        - [LCD SSD1306](https://github.com/lexus2k/ssd1306)
        - [LCD clock](https://github.com/ThingPulse/esp8266-oled-ssd1306/blob/master/examples/SSD1306ClockDemo/SSD1306ClockDemo.ino)
        - [LCD simple version](https://github.com/Matiasus/SSD1306)
        - [LCD for MCU](https://github.com/libdriver/ssd1306/tree/master)
        - [LCD for HAL](https://github.com/4ilo/ssd1306-stm32HAL/tree/master)
        - interface implementation
            - Hal I2C `2024/06/30`
- 2024/06/30 Start building the interface for sensors
    - [Heart rate sensor](https://www.amazon.co.jp/gp/product/B0CD73CPNJ/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
    - [sensor IC: max30102](https://github.com/eepj/stm32-max30102)
    - interface implementation
        - GPIO interrupt `2024/07/02`
    - Added the template task implementation
        - Command Queue (implemented by message buffer)
        - Command waiting
- 2024/07/03 
    - Heart rate sensor reads pulse signal from sensor
- 2024/08/18
    - LCD display
    - Signal processing
    - LCD signal drawing
- 2024/08/19
    - LCD information showing, layout tune

        
### trick memo
- tk_snd_mbf 
    - in a interrupt handler:
        ```c
        tk_snd_mbf(mbfid_1, &task1_cmd, sizeof(task1_cmd), TMO_POL);
        ```
    - in a normal task
        ```c
        tk_snd_mbf(mbfid_1, &task1_cmd, sizeof(task1_cmd), TMO_FEVR);
        ```

### Guide can be written
- How to set GPIO interrupt
    - Register interrupt using the cyhal GPIO API 
        - `cyhal_gpio_register_callback`
    - Register interrupt using the μT-Kernel interrupt management system

### References:
- Enviromental setup
    - [μT-Kernel 3.0 BSP2](https://www.tron.org/ja/page-6100/)
    - [μT-Kernel 3.0 BSP2 for ModusToolBox](https://github.com/tron-forum/mtk3_bsp2/blob/main/doc/bsp2_xmc_mtb_jp.md/)
    - [Infineon Modustoolbox](https://www.infineon.com/cms/en/design-support/tools/sdk/modustoolbox-software/)
    - [VS Code](https://code.visualstudio.com/)
    - [Kit XMC72 EVK](https://www.infineon.com/cms/en/product/evaluation-boards/kit_xmc72_evk/)
    - [ModusToolBox Vscode](https://www.infineon.com/dgdl/Infineon-ModusToolbox_3.1_Visual_Studio_Code_User_Guide-UserManual-v01_00-EN.pdf?fileId=8ac78c8c88704c7a0188a18b83824e58)
    - [step by step](https://github.com/Jiabin-develop/knowhow_share_public/blob/main/MTB_MTK/MTB_MTK_VSCODE.md)