# μT-Kernel application firmware
人体の圧力値をリアルタイムで測定できる装置  
[英語版](./1_000_EN_General_firmware_design.md)
Demo video: 

## 目次

1. [プロジェクト概要](#プロジェクト概要)
1. [ハードウェア](#ハードウェア概要)
1. [ソフトウェア概](#ソフトウェア概要)
1. [設計説明](#設計説明)
    1. [ディレクトリ構造](#ディレクトリ構造)
    1. [タスク](#タスク)
    1. [割り込み](#割り込み)
    1. [周辺機器](#周辺機器)
1. [重要な実装](#重要な実装)
    1. [タスク関数](#タスク関数)
    1. [タスクの作成](#タスクの作成)
    1. [タスクの開始](#タスクの開始)
    1. [周辺機器の設定](#周辺機器の設定)
    1. [割り込みハンドラ](#割り込みハンドラ)
    1. [μT-Kernel割り込み管理](#μt-kernel割り込み管理への登録)
1. [アプリケーションメイン関数 `app.main.c`](#アプリケーションメイン関数-appmainc)
1. [開発ノウハウ](#開発ノウハウ)
    1. [μT-Kernelイベント駆動型実装](#μt-kernel-イベント駆動型実装)
    1. [μT-Kernel割り込み設定](#μt-kernel-割り込み設定)
1. [プロジェクトビルド](#プロジェクトビルド)
1. [追加情報](#追加情報)
1. [参考資料](#参考資料)


---
1. ## プロジェクト概要
   - μT-Kernelをリアルタイムオペレーティングシステム（RTOS）として使用し、人体のストレスチェッカーを実装しています。
   - このアプリケーションでは、心拍数、心拍変動を測定し、それに応じたストレスレベル指数を提供します。測定は、パルスセンサーに指を触れることで行われ、生体パラメータが測定されます。ストレスチェック機能は市場ではほとんど見られませんが、このアプリケーションでは心拍変動を測定することでストレスレベルを測定する理論を参照しています。心拍変動を測定し、ストレスレベルを提供します。
   - 開発にはInfineon KIT_XMC72_EVK開発ボードが使用されています。
   - このプロジェクトは、μT-Kernel APIを完全に活用してアプリケーションを構築しており、組み込みシステムおよびμT-Kernelシステムの重要な部分が含まれています。
     - μT-Kernelのマルチタスク実行。
     - μT-Kernelの割り込み管理。
     - μT-Kernelのタスク同期。
     - μT-Kernelのタイマー管理。
     - シリアル通信：
       - I2C通信。
       - UART出力。
     - 入力周辺機器：
       - IO割り込み。
     - 出力周辺機器：
       - LCDディスプレイ。
   - 実際の使用ケースを考慮した実装であり、LCDディスプレイに必要な情報を提供します。例えば：
       - ユーザーがセンサーに触れていることの検出/通知（測定なし）
       - 測定ステータスの表示
       - 測定中のパルス波形の表示
       - ユーザーがセンサーに触れていない場合の通知（測定なし）
       - ユーザーボタンが押された場合のLCD画面のスクリーンショット
   - このプロジェクトは、オープンソースとして公開されます。ファイルやディレクトリはリソースごとに整理されており、機能を簡単に拡張できるようになっています。また、実装に関する詳細な情報も公開しています。
   - 実装はイベント駆動の概念に基づいており、低消費電力設計を目的としています。すべてのタスク実行はターゲットイベントが発生したときにのみ行われます。タスク実行の要求がない場合、タスクはブロック状態となり、MCUを低消費電力モードにすることができます。センサーとディスプレイが動作していない場合に自動的にオフにするタイマーが実装されており、ユーザーボタンを押すことで測定が再開されます。


1. ## ハードウェア概要
   - [Kit XMC72 EVK](https://www.infineon.com/cms/en/product/evaluation-boards/kit_xmc72_evk/)
       - ユーザーボタン
   - パルスセンサー (例: MAX30102)
       - I2C通信
       - データ準備完了割り込み (FIFO)
   - LCDディスプレイ (例: ssd1306)
       - I2C通信
   - ユーザーボタン
       - 割り込み
   - 接続
     ```mermaid
     graph TD;
       subgraph PULSE ["Pulse sensor: MAX30102"]
           PULSE_SDA["SDA"]
           PULSE_SCL["SCL"]
           PULSE_INT["INT_P"]
           PULSE_GND["GND"]
           PULSE_VCC["VCC_3.3V"]
       end

       subgraph MCU ["MCU: Kit XMC72 EVK"]
           MCU_SDA["SDA"]
           MCU_SCL["SCL"]
           MCU_INT["P10_5"]
           MCU_BTN["P13_6"]
           MCU_VCC["VCC"]
           MCU_GND["GND"]
       end

       subgraph LCD ["LCD Display: SSD1306"]
           LCD_SDA["SDA"]
           LCD_SCL["SCL"]
           LCD_VCC["VCC"]
           LCD_GND["GND"]
       end

       subgraph BTN ["ボタン"]
           BTN_BTN["GND <-- ボタン"]
       end

       MCU_SDA --- LCD_SDA;
       MCU_SCL --- LCD_SCL;

       PULSE_SDA --- MCU_SDA;
       PULSE_SCL --- MCU_SCL ;
       PULSE_VCC --- MCU_VCC;
       PULSE_GND --- MCU_GND;
       PULSE_INT --- MCU_INT;

       MCU_VCC --- LCD_VCC;
       MCU_GND --- LCD_GND;

       MCU_BTN --- BTN_BTN;
       MCU_GND --- BTN_BTN;
     ```

   - 実際の接続
     - 全体図
       - ![alt text](imgs/Device_out.jpg)
     - 拡大図
       - LCDの内容
         - `1-9`: ストレスレベル (値が高いほど高ストレスを意味する)
         - `HR`: 心拍数
         - `Va`: 心拍変動
         - 脈波の波形
       - ![alt text](imgs/Device.png)


1. ## ソフトウェア概要
   - [μT-Kernel 3.0 BSP2](https://www.tron.org/ja/page-6100/)
   - [ModusToolBox用 μT-Kernel 3.0 BSP2](https://github.com/tron-forum/mtk3_bsp2/blob/main/doc/bsp2_xmc_mtb_jp.md/)
   - [Infineon Modustoolbox](https://www.infineon.com/cms/en/design-support/tools/sdk/modustoolbox-software/)
   - [VS Code](https://code.visualstudio.com/)  
    - VS CodeでのModustoolbox開発のステップバイステップのセットアップは、以下にアップロードされています：
      - https://github.com/Jiabin-develop/knowhow_share_public/blob/main/MTB_MTK/MTB_MTK_VSCODE.md


1. ## 設計説明
   1. ### ディレクトリ構造
      ディレクトリとファイルは、異なるディレクトリに明確に分けられています。リソースは次のように分割されています (`user_xxx.h`ヘッダーファイルをincludeすると、そのフォルダ内のリソースを使用できます)：
      - tasks（タスク）
      - interrupt（割り込み）
      - peripheral（周辺機器） 
      
      ```
        0000_mtk_project/
        ├── Applications/
        │   ├── app.main.c
        │   ├── common.h
        │   ├── tasks/
        │   │   ├── user_task.h
        │   │   └── ...
        │   ├── interrupt/
        │   │   ├── user_interrupt.h
        │   │   └── ...
        │   └── peripheral/
        │       ├── user_peripheral.h
        │       └── ...
        ├── Documents/
        └── ...
      ```

   1. ### タスク
      各タスクはハードウェアや周辺機器の操作を担当しています。ハードウェアや周辺機器への操作は、ターゲットタスクのCommandQueueにコマンドを送ることで行われます。
      - タスク0: 今後の実装のためのテンプレート `task_template`
        - タスク作成のためのテンプレートです。通信のためのCommandQueueとコマンド実行のためのTaskの作成が含まれます。
      - タスク1: パルスセンサーの測定 `task_hr`
        - パルスセンサーからデータを読み取り、心拍数と心拍変動を計算します。
        - データの読み取りと計算が完了すると、`task_lcd`にディスプレイに波形と結果を更新するように要求します。
      - タスク2: LCDディスプレイ `task_lcd`
        - 測定情報でLCDディスプレイを更新する役割を担います。
        - LCDディスプレイは同期されています。
      - タスク3: 印刷 `task_print`
        - UART端末を通じて情報を印刷する役割を担います。割り込みはタスクに情報を印刷するコマンドを送ることができます。

   1. ### 割り込み
      - データ準備完了割り込み (FIFO)
        - データはパルスセンサーのFIFOに保存されます。FIFOが満杯になると、割り込みが発生し、MCUにデータをポーリングするよう要求します。
      - ユーザーボタン
        - ユーザーが測定結果をスクリーンショットするためにボタンが使用されます。
          - 測定が完了している場合、ボタンを押すとLCDはスクリーンショットを撮ります。
          - 測定が進行中の場合はスクリーンショットは撮られません。
          - ボタンを再度押すと、測定が即座に再開されます。

   1. ### 周辺機器
      - パルスセンサー
      - LCDディスプレイ
      - I2C通信




1. ## 重要な実装
   いくつかの重要な実装は、次のセクションで説明されています。

   ### タスク関数
   ```c
   void task_template(INT stacd, void *exinf);
   void task_hr(INT stacd, void *exinf);
   void task_lcd(INT stacd, void *exinf);
   void task_print(INT stacd, void *exinf);
   ```

   ### タスクの作成
   - タスク作成のためのμT-Kernel APIの呼び出しは、1つの関数にまとめられています。いくつかのμT-Kernel APIは、より簡単な関数に抽象化されています。これらの関数には、接頭辞**`xmtk`**が付加されています。
   ```c
   void xmtk_create_task_template();
   void xmtk_create_task_hr();
   void xmtk_create_task_lcd();
   void xmtk_create_task_print();
   ```

   ### タスクの開始
   - タスクを開始するためのμT-Kernel APIの呼び出しは、1つの関数にまとめられています。
   ```c
   void xmtk_start_task_template();
   void xmtk_start_task_hr();
   void xmtk_start_task_lcd();
   void xmtk_start_task_print();
   ```

   ### 周辺機器の設定
   - 周辺機器の初期化
   ```c
   // I2C初期化用
   void i2c_init();
   // IO初期化用
   void gpio_init();
   ```

   ### 割り込みハンドラ
   - 割り込みハンドラは、μT-Kernel割り込み管理システムに割り込みを登録するために使用されます。
    ```c
    // USER_BTN1が押されたときに割り込みが発生します。
    void xmtk_gpio_interrupt_handler(UINT intno);
    // パルスセンサーでデータが準備できたときに割り込みが発生します。データはセンサーのFIFOに保存されます。
    void xmtk_hr_interrupt_handler(UINT intno);
    ```

   ### μT-Kernel割り込み管理への割り込みの登録
   - 割り込みの登録
   ```c
   void xmtk_register_interrupt();
   ```

1. ### アプリケーションメイン関数 `app.main.c`
   ```c
   EXPORT INT usermain(void)
   {
     tm_putstring((UB *)"Start User-main program.\n");

     /* 周辺機器リソースの初期化と登録 */
     xmtk_register_interrupt();
     xmtk_enhance_i2c();

     /* タスクの作成 */
     xmtk_create_task_template();
     xmtk_create_task_print();
     xmtk_create_task_lcd();
     xmtk_create_task_hr();

     /* タスクの開始 */
     xmtk_start_task_template();
     xmtk_start_task_print();
     xmtk_start_task_lcd();
     xmtk_start_task_hr();

     tk_slp_tsk(TMO_FEVR);

     return 0;
   }
   ```


1. ## 開発ノウハウ
   1. ### μT-Kernel イベント駆動型実装
      - このドキュメントでは、μT-Kernelにおけるイベント駆動型（コマンド駆動型）タスク通信の実装について説明します。タスクの通信と制御には以下の特徴があります：
      - タスクは、他のリソースからコマンド（リクエスト）を受信した後にのみ実行されます。コマンドがない場合、タスクは待機（ブロック）状態 `TMO_FEVR` にあります。これにより、CPUリソースが他のタスクに利用可能になります。
         - タスクにリクエストを処理するよう要求する割り込み
         - 他のタスクからのリクエスト
      - 各タスクには、他のタスクからコマンドを受け取るための独自のコマンドキューがあります。タスクには次のコンポーネントが含まれます：
         - `CommandQueue_xxx`: 他のタスクからコマンドを受け取る
         - `Task_xxx`: タスクの機能
      - コマンドの送信と受信は、それぞれ次の関数で再定義されています：
         - `xmtk_send_command(commandQueue, command)`: 特定のコマンドキューにコマンドを送信
         - `xmtk_receive_command(commandQueue, command)`: 特定のコマンドキューからコマンドを受信
         ```c
         #define xmtk_send_command(commandQueue, command) tk_snd_mbf(commandQueue, &command, sizeof(command), TMO_POL);
         #define xmtk_receive_command(commandQueue, command) tk_rcv_mbf(commandQueue, &command, TMO_FEVR);
         ```
      - テンプレートファイルは `task_template.c` として作成されています。
      - タスクの構造は次のようにフローチャートで表現できます：
         ```mermaid
           stateDiagram-v2
               Requester --> CommandQueue
               CommandQueue --> switch(command.cmd) : xmtk_receive_command(command)
               state switch(command.cmd) {
                   direction LR
                   cmd0: cmd0->task_cmd0()
                   cmd1: cmd01->task_cmd1()
                   cmd2: cmd2->task_cmd2()
               }
         ```
      - タスク間の通信は以下のように行われます。`requester` は割り込みハンドラや `execute_cmd` など、どんなコードでも構いません。異なるタスクにはそれぞれの `CommandQueue` があります：
         ```mermaid
             stateDiagram-v2
                 re1: requester
                 re2: requester
                 re3: requester
                 re1 --> CommandQueue0: send_command
                 re2 --> CommandQueue1: send_command
                 re3 --> CommandQueue2: send_command
                 CommandQueue0 --> switch(command0.cmd) : receive_command
                 CommandQueue1 --> switch(command1.cmd) : receive_command
                 CommandQueue2 --> switch(command2.cmd) : receive_command

                 state switch(command0.cmd) {
                     direction LR
                     cmd0: execute command0.cmd 
                 }
                 state switch(command1.cmd) {
                     direction LR
                     cmd1: execute command1.cmd
                 }
                 state switch(command2.cmd) {
                     direction LR
                     cmd2: execute command2.cmd
                 }
         ```

      #### 実装
        ##### コマンドの定義
        - 各タスクは実行すべきコマンドを受け取ります。最初に、コマンド構造体 `command_template_t` を定義する必要があります。
          ```c
          typedef struct
          {
              cmd_list_template_t cmd;
              char msg[20];
              char *value;
          } command_template_t;
          ```
        - ここで、`cmd` は、タスクが実行すべきコマンドを列挙する `cmd_list_template_t` で定義されています：
          ```c
          typedef enum
          {
              TASK_TEMPLATE_CMD,
              TASK_TEMPLATE_CMD_1,
          } cmd_list_template_t;
          ```

        ##### タスクの構成
        - タスクには、他のタスクと共有すべきコンポーネントが必要です。1つは `Task` 関数、もう1つは `CommandQueue` です。
          ```c
          EXPORT ID CommandQueue_template;
          EXPORT ID Task_template;
          ```
        - `CommandQueue` と `Task` の設定を定義します。
          ```c
          T_CTSK ctsk_template = {
              // タスク作成情報
              .itskpri = 10,
              .stksz = 1024,
              .task = task_template,
              .tskatr = TA_HLNG | TA_RNG3,
          };

          T_CMBF cmbf_task_template = {
              // メッセージバッファ作成情報
              .mbfatr = TA_TFIFO | TA_MFIFO,
              .bufsz = 1024,
              .maxmsz = sizeof(command_template_t),
          };
          ```

        - タスクの実行コード。タスクは、コマンドを受信した後にのみ実行されます（`xmtk_receive_command`）。コマンドがない場合、タスクは待機（ブロック）状態にあり、他のタスクがCPUリソースを利用できるようになります。
          ```c
          void task_template(INT stacd, void *exinf)
          {
              INT len = 0;
              tm_printf((UB *)"task task_template on\n");
              while (1)
              {
                  len = xmtk_receive_command(CommandQueue_template, template_cmd);
                  if (len > 0)
                  {
                      tm_printf((UB *)"template: got cmd: %d, msg: %s, value: %s\n", template_cmd.cmd, template_cmd.msg, template_cmd.value);
                      switch (template_cmd.cmd)
                      {
                      case TEMPLATE_CMD_0:
                          // 実装
                          break;
                      case TEMPLATE_CMD_1:
                          // 実装
                          break;
                      default:
                          break;
                      }
                  }
              }
          }
          ```

        - タスクを登録し、タスクを開始するための関数
          ```c
              void create_task_template()
              {
                  CommandQueue_template = tk_cre_mbf(&cmbf_task_template);
                  Task_template = tk_cre_tsk(&ctsk_template);
              }

              void start_task_template()
              {
                  tk_sta_tsk(Task_template, 0);
              }
          ```

        ##### コマンドを送信する
        - タスクは、別のタスクにコマンドを送信してコマンドの実行を要求できます。コマンドのインスタンスを `CommandQueue` に送信します。コマンドにはメッセージを添付して、詳細を提供することができます。以下は、`CommandQueue_template` にコマンドを送信する例であり、`Task_template` にコマンド `TEMPLATE_CMD_0` を実行するよう要求しています。
          ```c
              command_template_t cmd = 
              {
                  .cmd = TEMPLATE_CMD_0, 
                  .msg = "send to template", 
                  .value = "test"
              };
              xmtk_send_command(CommandQueue_template, cmd);
          ```

          - このドキュメントでは、μT-Kernel RTOSにおけるイベント駆動型（コマンド駆動型）タスク間通信の実装方法について説明しています。`cmd_list_template_t` や `Task_template` は、特定の要件に応じてカスタマイズできます。


   1. ### μT-Kernel 割り込み設定
   - このドキュメントでは、μT-Kernelにおける割り込みの設定と実装の概要を説明します。基本的な実装といくつかの経験をカバーしています。
   
      #### 割り込みについて
      - 外部または内部のイベント（ハードウェア信号、タイマーオーバーフロー、I/O操作など）によって、MCUに対してアクションを要求することが一般的です。例えば、センサーからデータが利用可能な場合にMCUがデータを読み取るか、ユーザーがボタンを押した際にステータスを変更するなどです。
      - 割り込みに依存する機能を実装するためには、次のコンポーネントが必要です：
        - 割り込みリソースの初期化
        - 割り込みハンドラ
        - 割り込みの設定
        - 割り込みハンドラをシステムに登録すること

      #### Halでの割り込みハンドラの登録
      - 以下は、`cyhal`ライブラリを使用して、ユーザーボタンの押下に対する割り込みインタラクションを実装する例です。
      
      - 割り込みリソースの初期化
          ```c
          cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT, CYBSP_USER_BTN_DRIVE, CYBSP_BTN_OFF);
          ```

      - 割り込みハンドラの定義
          ```c
          static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event)
          {
              // 必要な機能を実装
          }
          ```

      - 割り込みハンドラの設定と登録
          ```c
          cyhal_gpio_callback_data_t gpio_btn_callback_data;
          gpio_btn_callback_data.callback = gpio_interrupt_handler;
          cyhal_gpio_register_callback(CYBSP_USER_BTN, &gpio_btn_callback_data);
          ```

      - 割り込みの有効化
          ```c
          cyhal_gpio_enable_event(CYBSP_USER_BTN, CYHAL_GPIO_IRQ_FALL, GPIO_INTERRUPT_PRIORITY, true);
          ```

      - 上記の設定が完了すると、ユーザーがボタンを押すたびに `gpio_interrupt_handler` 関数が呼び出されます。

      #### μT-Kernelでの割り込みハンドラの登録
      - μT-Kernelは割り込み管理機能を提供します。しかし、Halライブラリを使う場合よりも複雑です。以下のステップといくつかのポイントを考慮する必要があります。

      - 割り込みリソースの初期化（Halと同様）
          ```c
          cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT, CYBSP_USER_BTN_DRIVE, CYBSP_BTN_OFF);
          ```

      - 割り込みハンドラの定義（<strong>異なる点</strong>）。次のステップに注意してください：
          - 割り込みをクリアするために `Cy_GPIO_ClearInterrupt(GPIO_PRT21, 4)` を使用します。
            - このステップを省略すると、割り込みハンドラが連続して実行され続けます。
          - （オプション）割り込みが期待されるピンから来ているかどうかを確認します。
            - 例えば、`Infineon XMC7200` ボードの場合、pin21.3とpin21.4の両方が `ioss_interrupts_gpio_21_IRQn` に割り込みを生成するため、割り込みがどのピンから発生したかを確認する必要があります。`Cy_GPIO_GetInterruptStatus(GPIO_PRT21, 4)` 関数を使用して、割り込みがpin21.4から来ているかどうかを確認できます。
          - 以下は、BTN1が押された際に、`printタスク` に割り込み情報を印刷するコマンドを送信する割り込みハンドラの例です。
            ```c
            void xmtk_gpio_interrupt_handler(UINT intno)
            {
                Cy_GPIO_ClearInterrupt(GPIO_PRT21, 4);
                NVIC_ClearPendingIRQ(intno);
                task_print_cmd_t print_cmd = {.cmd = TASK_PRINT_PRINT, .msg = "BTN1 pushed", .value = "interrupt value to print"};
                xmtk_send_command(CommandQueue_print, print_cmd);
            }
            ```

      - 割り込みハンドラの設定と登録（<strong>異なる点</strong>）
          - μT-Kernel API `tk_def_int` を使用して、割り込みと割り込みハンドラを登録します。
          - 期待される割り込みピンの割り込み番号を確認する必要があります。例えば、`Infineon XMC7200` ボードの場合、BTN1はpin21.4にあり、対応する割り込み番号は `ioss_interrupts_gpio_21_IRQn` です。
          - μT-Kernel API `tk_def_int` を使用した割り込み登録の例：
            ```c
            T_DINT t_dint = {
                .intatr = TA_HLNG,
                .inthdr = xmtk_gpio_interrupt_handler};
            tk_def_int(ioss_interrupts_gpio_21_IRQn, &t_dint);
            ```

      - 割り込みの有効化
          ```c
          cyhal_gpio_enable_event(CYBSP_USER_BTN, CYHAL_GPIO_IRQ_FALL, GPIO_INTERRUPT_PRIORITY, true);
          ```


1. ## プロジェクトビルド
    ファームウェアをコンパイルして実行するには、次の手順に従ってください：
    1. パルスセンサーとLCDディスプレイを開発ボードに接続します。
    2. 必要なソフトウェアをインストールします。
    3. 開発環境のプロジェクト設定を構成します。
    4. プロジェクトをビルドします。
    5. Infineon KIT_XMC72_EVK開発ボードにファームウェアを書き込みます。
    6. LCDディスプレイに表示される測定情報を確認します。
    - プロジェクトのクローンおよびビルドのステップバイステップの手順は、[1_010_EN_Clone_Build_Project.md](./1_010_EN_Clone_Build_Project.md)に記載されています。



1. ## 追加情報
   - ストレスがどのように測定されるかについて: [心拍変動 (HRV)](https://my.clevelandclinic.org/health/symptoms/21773-heart-rate-variability-hrv)
   - Driver libraries used in this project:
    - [LCD for HAL](https://github.com/4ilo/ssd1306-stm32HAL/tree/master)
    - [sensor IC: max30102](https://github.com/eepj/stm32-max30102)



1. ## 参考資料
   - Infineon KIT_XMC72_EVK ドキュメント: [KIT_XMC72_EVKのドキュメント](https://www.infineon.com/dgdl/Infineon-KIT_XMC72_EVK-V1.0.pdf?fileId=)

