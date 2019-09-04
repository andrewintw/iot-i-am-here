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
    LEFT,
    PRESENTED,
} present_state_t;

typedef enum {
    RUN_SETUP,
    RUN_LOOP,
} run_state_t;

typedef enum {
    BTN_PRESSED,
    BTN_RELEASED,
} button_state_t;

#define LOOP_INTERVAL_MS            (500)
#define BUTTON_PIN                  0               /* in my ESP-01 module, LED_BUILTIN is D2,
                                                       so don't use D2 if you want to use ESP-01 onboard LED */

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
#define SECRET_READ_APIKEY          "ABC"           /* replace ABC with your channel read API Key */
#define SEND_INTERVAL_MS            (15 * 1000)


