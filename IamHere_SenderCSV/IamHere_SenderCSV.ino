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
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include "ThingSpeak.h"
#include "config.h"

ESP8266WiFiMulti wifiMulti;
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

unsigned long setup_starttime   = 0;
unsigned long data_sendtime     = 0;
run_state_t run_st              = RUN_SETUP;

boolean connectioWasAlive       = true;
unsigned long chkwifi_time      = 0;
unsigned long ntp_unix_time     = 0;
unsigned long curr_unix_time    = 0;


unsigned int localPort = 2390;      // local port to listen for UDP packets
IPAddress timeServerIP;
const char* ntpServerName = "tock.stdtime.gov.tw";
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
WiFiUDP udp;

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
        Serial.printf("HC-SR501 initializing...(%d/%dsec)\r\n", (t / 1000), (PIR_INIT_TIME_MS / 1000));
        delay(t);
    }
}

void init_wifi_cfg(wifi_sta_config_t *wifi_sta) {
    wifi_sta_config_t *sta = wifi_sta;

    while (*sta->ssid != NULL ) {
        Serial.printf("addAP: %s / %s\r\n", sta->ssid, sta->password);
        wifiMulti.addAP(sta->ssid, sta->password);
        sta++;
    }
}

void init_net_connect() {
    unsigned int i = 0;

    Serial.printf("\r\nLooking for WiFi");
    while (wifiMulti.run() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);

        i++;

        if (i % 2 == 0) {
            ledCtrl(OFF);
        } else {
            ledCtrl(ON);
        }

        if (i >= ((1 << (sizeof(uint16) * 8)) - 1)) {
            i = 0;
        }
    }

    Serial.printf("\r\nWiFi connected to: %s\r\n", WiFi.SSID().c_str());
    Serial.printf("IP address: %s\r\n", WiFi.localIP().toString().c_str());
    Serial.println("Starting UDP");
    udp.begin(localPort);
    Serial.print("Local port: ");
    Serial.println(udp.localPort());
    ledCtrl(ON);
}

void init_cloud_services() {
    ThingSpeak.begin(wifi_client);
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress& address) {
    Serial.println("sending NTP packet...");
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;

    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    udp.beginPacket(address, 123); //NTP requests are to port 123
    udp.write(packetBuffer, NTP_PACKET_SIZE);
    udp.endPacket();
}

void update_ntp_time() {
    WiFi.hostByName(ntpServerName, timeServerIP);
    sendNTPpacket(timeServerIP); // send an NTP packet to a time server
    delay(1000);
    int cb = udp.parsePacket();
    if (!cb) {
        Serial.println("no packet yet");
    } else {
        Serial.print("packet received, length=");
        Serial.println(cb);
        // We've received a packet, read the data from it
        udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

        //the timestamp starts at byte 40 of the received packet and is four bytes,
        // or two words, long. First, esxtract the two words:

        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
        // combine the four bytes (two words) into a long integer
        // this is NTP time (seconds since Jan 1 1900):
        unsigned long secsSince1900 = highWord << 16 | lowWord;
        Serial.print("Seconds since Jan 1 1900 = ");
        Serial.println(secsSince1900);

        // now convert NTP time into everyday time:
        Serial.print("Unix time = ");
        // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
        const unsigned long seventyYears = 2208988800UL;
        // subtract seventy years:
        unsigned long epoch = secsSince1900 - seventyYears;
        // print Unix time:
        epoch += 8 * 60 * 60;
        ntp_unix_time = epoch;
        Serial.println(epoch);
    }
}

void setup() {
    run_st = RUN_SETUP;
    setup_starttime = millis();

    init_dev_io();
    init_wifi_cfg(wifi_sta_cfg);
    init_net_connect();
    init_cloud_services();

    send_data_to_cloud(LEFT);   /* force set LEFT state in setup */
    init_pir_sensor();
    update_ntp_time();
}

int send_data_to_cloud(int data) {
    int httpCode;

    if (connectioWasAlive == false) {
        Serial.println("warning: ignore data transmit! the network is disconnected.");
        return 1;
    }

    if (((millis() - data_sendtime) < SEND_INTERVAL_MS) && (run_st == RUN_LOOP)) {
        Serial.println("warning: ignore data transmit! data push interval must be greater than 15 seconds.");
        return 1;
    }

    data_sendtime =  millis();

    httpCode = ThingSpeak.writeField(myChannelNumber, dataFieldNumber, data, myWriteAPIKey);

    if (httpCode == 200) {
        ledCtrl(BLINK_SENT);
        return 0;
    }
    else {
        Serial.printf("Problem writing to channel. HTTP error code: %d\r\n", httpCode);
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
    Serial.printf("  pv:%d dc:%d\tndc:%d\tMax(ndc):%d", sensor_data, detect_count, no_detect_count, max_no_detect_count);

#if CFG_SHOW_SEC
    Serial.printf("  %dsec", ((max_no_detect_count * LOOP_INTERVAL_MS) / 1000));
#endif

    if (present_st == PRESENTED) {
        Serial.print(" ... [O]");
    } else {
        Serial.print(" ... [X]");
    }

    if (connectioWasAlive == true) {
        Serial.print(" ");
    } else {
        Serial.print("!");
    }

    if (period_st == CHK_START)
    {
        Serial.printf(" --* (%dsec)\r\n", (CHK_PERIOD_TIME / 1000));
        period_st = CHK_IN_PERID;
    }
    else if (period_st == CHK_IN_PERID)
    {
        Serial.printf("   | %d\r\n", detect_count_in_perid);
    }
    else if (period_st == CHK_END)
    {
        Serial.printf(" __x %d\r\n", detect_count_in_perid);
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

void show_summary_csv() {
    print_timestamp();
    Serial.printf(", %d, %d, %d\r\n", curr_unix_time, sensor_data, present_st);


    if (period_st == CHK_START)
    {
        period_st = CHK_IN_PERID;
    }
    else if (period_st == CHK_END)
    {
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
}

void monitor_wifi()
{
    if (((millis() - chkwifi_time) < CHK_WIFI_INTERVAL_MS) && (run_st == RUN_LOOP)) {
        return;
    }

    chkwifi_time =  millis();

    if (wifiMulti.run() != WL_CONNECTED) {
        if (connectioWasAlive == true) {
            connectioWasAlive = false;
        }
    } else if (connectioWasAlive == false) {
        connectioWasAlive = true;
    }
}

void loop() {
    run_st = RUN_LOOP;

    monitor_wifi();
    update_sensor_data();
    show_summary_csv();

    delay(LOOP_INTERVAL_MS);
}
