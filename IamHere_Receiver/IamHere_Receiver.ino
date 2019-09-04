/*  License
 *  --------------------------------------------------------------------------------
 *  "THE BEER-WARE LICENSE" (Revision 42):
 *  <yenchang.lin@gmail.com>  wrote this program.  As long as you retain this notice
 *  you can do whatever you want with this stuff. If we meet some day, and you think
 *  this stuff is worth it, you can buy me a beer in return.  Andrew Lin
 *  --------------------------------------------------------------------------------
 *
 *  HC-SR501 PIR Motion Detector setup
 *  --Jumper Set         : use H (Repeat Trigger)
 *  --Sensitivity Adjust : Anti C.W to the end (Low, about 3 meters)
 *  --Time-delay  Adjust : Anti C.W to the end (Shorten, 5 sec)
 */

#include <ESP8266WiFi.h>
#include "ThingSpeak.h"
#include "config.h"

WiFiClient wifi_client;

int present_st                  = LEFT;

unsigned long myChannelNumber   = SECRET_CH_ID;
unsigned int  dataFieldNumber   = 1;
const char *myWriteAPIKey       = SECRET_WRITE_APIKEY;
const char *myReadAPIKey        = SECRET_READ_APIKEY;
const char *ssid                = SECRET_SSID;
const char *password            = SECRET_PASS;
int keyIndex                    = 0;    /* your network key index number (needed only for WEP) */

unsigned long data_sendtime     = 0;
run_state_t run_st              = RUN_SETUP;

uint8 do_send                   = 0;

void init_dev_io() {
    Serial.begin(115200);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB
    }
    delay(100);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    ledCtrl(BLINK_INIT);
    Serial.println();
}

void init_net_connect() {
    unsigned int i = 0;

    Serial.println();
    Serial.print("Connecting to: ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        i++;
        delay(500);
        Serial.print(".");

        if (i % 2 == 0)
            ledCtrl(OFF);
        else
            ledCtrl(ON);

        if (i >= ((1 << (sizeof(uint16) * 8)) - 1)) {
            i = 0;
        }
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    ledCtrl(ON);

    ThingSpeak.begin(wifi_client);
}

void setup() {
    run_st = RUN_SETUP;

    init_dev_io();
    init_net_connect();
    ledCtrl(OFF);

    get_data_from_cloud(&present_st);
}

void update_led_indicator(int st)
{
    if (st == PRESENTED) {
        ledCtrl(ON);
    } else if (st == LEFT) {
        ledCtrl(OFF);
    }
}

int get_data_from_cloud(int *fieldData) {
    int httpCode = 0;

    if (((millis() - data_sendtime) < SEND_INTERVAL_MS) && (run_st == RUN_LOOP)) {
#if DEBUG
        Serial.println("warning: ignore data transmit! data push interval must be greater than 15 seconds.");
#else
        Serial.print("<");
#endif
        return 1;
    }
    Serial.println();

    data_sendtime =  millis();

    *fieldData = ThingSpeak.readIntField(myChannelNumber, dataFieldNumber, myReadAPIKey);

    httpCode = ThingSpeak.getLastReadStatus();

    if (httpCode == 200) {
#if DEBUG
        Serial.println("Field Data: " + String(*fieldData));
#else
        show_summary();
        Serial.print(" <<");
        Serial.println(*fieldData);
#endif
        ledCtrl(BLINK_SENT);
        return 0;
    } else {
        Serial.println("Problem reading channel. HTTP error code " + String(httpCode));
        return -1;
    }
}

int send_data_to_cloud(int data) {
    int httpCode;

    if (((millis() - data_sendtime) < SEND_INTERVAL_MS) && (run_st == RUN_LOOP)) {
#if DEBUG
        Serial.println("warning: ignore data transmit! data push interval must be greater than 15 seconds.");
#else
        Serial.print(">");
#endif
        return 1;
    }
    Serial.println();

    data_sendtime =  millis();

    httpCode = ThingSpeak.writeField(myChannelNumber, dataFieldNumber, data, myWriteAPIKey);

    if (httpCode == 200) {
        present_st = LEFT;
        do_send = 0;
#if DEBUG
        Serial.print("Channel write: ");
        Serial.print(data);
        Serial.println(" successful.");
#else
        show_summary();
        Serial.print(" >>");
        Serial.println(data);
#endif
        ledCtrl(BLINK_SENT);
        return 0;
    }
    else {
        Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
        return -1;
    }
}

void show_summary() {
    print_timestamp();

    Serial.print(" data:");
    Serial.print(present_st);
}

int get_button_state() {
    return digitalRead(BUTTON_PIN);
}

void loop() {
    run_st = RUN_LOOP;

    if (get_button_state() == BTN_PRESSED) {
        do_send = 1;
    }

    if (do_send == 1) {
        send_data_to_cloud(LEFT);   /* force set the present state to left, it will update by Sender */
    } else {
        get_data_from_cloud(&present_st);
    }

    update_led_indicator(present_st);

    delay(LOOP_INTERVAL_MS);
}
