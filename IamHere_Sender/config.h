/*  License
 *  --------------------------------------------------------------------------------
 *  "THE BEER-WARE LICENSE" (Revision 42):
 *  <yenchang.lin@gmail.com>  wrote this program.  As long as you retain this notice
 *  you can do whatever you want with this stuff. If we meet some day, and you think
 *  this stuff is worth it, you can buy me a beer in return.  Andrew Lin
 *  --------------------------------------------------------------------------------
 */

typedef enum {
    OFF,
    ON,
    BLINK_INIT,
    BLINK_SENT,
} led_state_t;

typedef enum {
    CHK_NONE,
    CHK_START,
    CHK_IN_PERID,
    CHK_END,
} period_state_t;

typedef enum {
    LEFT,
    PRESENTED,
} present_state_t;

typedef enum {
    RUN_SETUP,
    RUN_LOOP,
} run_state_t;

#define LOOP_INTERVAL_MS            (1 * 1000)
#define PIROUT_PIN                  0               /* in my ESP-01 module, LED_BUILTIN is D2,
                                                       so don't use D2 if you want to use ESP-01 onboard LED */
#define PIR_INIT_TIME_MS            (60 * 1000)     /* HC-SR501 requires some time to acclimatize to the infrared energy in the room.
                                                       This takes 60 seconds when the sensor is first powered up. */

/* =================================================================
 * According to your usage situation adjust the following parameters
 * =================================================================
 */
#define CHK_PERIOD_TIME             (60 * 1000)
#define THRESHOLD_PRESENT           15              /* detected counts in the detection period */
#define THRESHOLD_LEAVE             120             /* detected counts in the detection period */
#define CFG_IGNORE_NOISES           1
#define IGNORE_NOISES_NUM           2               /* for ignore noises */

/* ===============================================================
 * According to your WiFi settings adjust the following parameters
 * ===============================================================
 */
#define SECRET_SSID                 "MySSID"        /* replace MySSID with your WiFi network name */
#define SECRET_PASS                 "MyPassword"    /* replace MyPassword with your WiFi password */

/* ============================================================================
 * According to your Cloud(ThingSpeak) settings adjust the following parameters
 * ============================================================================
 */
#define SECRET_CH_ID                000000          /* replace 000000 with your channel number */
#define SECRET_WRITE_APIKEY         "XYZ"           /* replace XYZ with your channel write API Key */
#define CFG_SEND_WHEN_CHANGE        0
#define CFG_SEND_IN_EACH_PERIOD     1
#define SEND_INTERVAL_MS            (15 * 1000)


