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

int sensor_data                 = 0;
uint16 detect_count             = 0;
uint16 no_detect_count          = 0;
uint16 max_no_detect_count      = 0;

unsigned long start_chktime     = 0;
period_state_t period_st        = CHK_NONE;
uint16 detect_count_in_perid    = 0;

present_state_t present_st      = LEFT;

unsigned long myChannelNumber   = SECRET_CH_ID;
unsigned int  dataFieldNumber   = 1;
const char *myWriteAPIKey       = SECRET_WRITE_APIKEY;
const char *ssid                = SECRET_SSID;
const char *password            = SECRET_PASS;
int keyIndex                    = 0;    /* your network key index number (needed only for WEP) */

unsigned long setup_starttime   = 0;
unsigned long data_sendtime     = 0;
run_state_t run_st              = RUN_SETUP;

void init_dev_io() {
    Serial.begin(115200);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB
    }
    delay(100);

    pinMode(PIROUT_PIN, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    ledCtrl(BLINK_INIT);
    Serial.println();
}

void init_pir_sensor() {
    unsigned long t = 0;

    /*  HC-SR501 requires some time to acclimatize to the infrared energy in the room.
        This takes 60 seconds when the sensor is first powered up. */
    if (PIR_INIT_TIME_MS > (millis() - setup_starttime)) {
        t = PIR_INIT_TIME_MS - (millis() - setup_starttime);
        Serial.print("HC-SR501 initializing...(");
        Serial.print(t / 1000);
        Serial.print("/");
        Serial.print(PIR_INIT_TIME_MS / 1000);
        Serial.println("sec)");
        delay(t);
    }
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
    setup_starttime = millis();

    init_dev_io();
    init_net_connect();
    send_data_to_cloud(LEFT);   /* force set LEFT state in setup */
    init_pir_sensor();
}

int send_data_to_cloud(int data) {
    int httpCode;

    if (((millis() - data_sendtime) < SEND_INTERVAL_MS) && (run_st == RUN_LOOP)) {
        Serial.println("warning: ignore data transmit! data push interval must be greater than 15 seconds.");
        return 1;
    }

    data_sendtime =  millis();

    httpCode = ThingSpeak.writeField(myChannelNumber, dataFieldNumber, data, myWriteAPIKey);

    if (httpCode == 200) {
        Serial.print("Channel write: ");
        Serial.print(data);
        Serial.println(" successful.");
        ledCtrl(BLINK_SENT);
        return 0;
    }
    else {
        Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
        return -1;
    }
}

void update_sensor_data() {
    sensor_data = digitalRead(PIROUT_PIN);

    if (sensor_data == HIGH)
    {
        //Serial.println("ON");
        if (present_st != PRESENTED) {
            ledCtrl(ON);
        }
        detect_count++;

#if CFG_IGNORE_NOISES
        if ((period_st == CHK_NONE) &&
                (detect_count <= IGNORE_NOISES_NUM)) {
            /* ignore noises */
            return;
        }
#endif

        no_detect_count = 0;

        if (period_st == CHK_NONE) {
            period_st = CHK_START;
            start_chktime = millis();
        }

        if (period_st > CHK_START) { /* CHK_IN_PERID, CHK_END */
            detect_count_in_perid++;

            /*  Why use `== THRESHOLD_PRESENT` rather than use `>= THRESHOLD_PRESENT`
             *  You can't push data frequently. Interval must be greater than 15 seconds.
             *  when the condition is triggered, only one data is transmitted.
             */
            if (detect_count_in_perid == THRESHOLD_PRESENT) {
                /* That means the system recognizes that someone really exists there */
#if CFG_SEND_WHEN_CHANGE
                if (present_st != PRESENTED) { /* state change happens */
                    present_st = PRESENTED;
                    send_data_to_cloud(present_st);
                    ledCtrl(ON);
                }
#else
                present_st = PRESENTED;
                send_data_to_cloud(present_st);
                ledCtrl(ON);
#endif
            }
        }
    }
    else
    {
        //Serial.println("OFF");
        if (present_st != PRESENTED) {
            ledCtrl(OFF);
        }
        no_detect_count++;
        detect_count = 0;

        if (no_detect_count == THRESHOLD_LEAVE)
        {
#if CFG_SEND_IN_EACH_PERIOD
            no_detect_count = 0; /* it will trigger the transmit
                                    every time the threshold is reached */
#endif
            /* That means the system recognizes that someone leaves already */
#if CFG_SEND_WHEN_CHANGE
            if (present_st != LEFT) { /* state change happens */
                present_st = LEFT;
                send_data_to_cloud(present_st);
                ledCtrl(OFF);
            }
#else
            present_st = LEFT;
            send_data_to_cloud(present_st);
            ledCtrl(OFF);
#endif
        }

        if (no_detect_count >= ((1 << (sizeof(uint16) * 8)) - 1)) {
            no_detect_count = 0;
            max_no_detect_count = 0;
        }

        if (no_detect_count > max_no_detect_count) {
            max_no_detect_count = no_detect_count;
        }
    }

    if ((period_st == CHK_IN_PERID) &&
            (millis() - start_chktime > CHK_PERIOD_TIME)) {
        period_st = CHK_END;
    }
}

void show_summary() {
    print_timestamp();

    Serial.print("  pv:");
    Serial.print(sensor_data);
    Serial.print(" dc:");
    Serial.print(detect_count);
    Serial.print("\tndc:");
    Serial.print(no_detect_count);
    Serial.print("\tMax(ndc):");
    Serial.print(max_no_detect_count);

#if CFG_SHOW_SEC
    Serial.print("  ");
    Serial.print((max_no_detect_count * LOOP_INTERVAL_MS) / 1000);
    Serial.print("sec");
#endif

    if (present_st == PRESENTED) {
        Serial.print(" ... [O]");
    } else {
        Serial.print(" ... [X]");
    }

    if (period_st == CHK_START)
    {
        Serial.print(" --* (");
        Serial.print(CHK_PERIOD_TIME / 1000);
        Serial.println("sec)");
        period_st = CHK_IN_PERID;
    }
    else if (period_st == CHK_IN_PERID)
    {
        Serial.print("   | ");
        Serial.println(detect_count_in_perid);
    }
    else if (period_st == CHK_END)
    {
        Serial.print(" __x ");
        Serial.println(detect_count_in_perid);
        period_st = CHK_NONE;
        detect_count_in_perid = 0;

#if CFG_SEND_IN_EACH_PERIOD
        //if (present_st == PRESENTED) {
        send_data_to_cloud(present_st);
        //}
#endif
        // reset counter
        //detect_count = 0;
        //no_detect_count = 0;
    }
    else
    {
        Serial.println();
    }
}

void loop() {
    run_st = RUN_LOOP;
    update_sensor_data();
    show_summary();
    delay(LOOP_INTERVAL_MS);
}
