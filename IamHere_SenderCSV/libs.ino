/*  License
 *  --------------------------------------------------------------------------------
 *  "THE BEER-WARE LICENSE" (Revision 42):
 *  <yenchang.lin@gmail.com>  wrote this program.  As long as you retain this notice
 *  you can do whatever you want with this stuff. If we meet some day, and you think
 *  this stuff is worth it, you can buy me a beer in return.  Andrew Lin
 *  --------------------------------------------------------------------------------
 */

void ledCtrl(led_state_t st) {
    switch (st)
    {
        case ON:
            digitalWrite(LED_BUILTIN, LOW); /* Active Low LED_BUILTIN */
            break;
        case OFF:
            digitalWrite(LED_BUILTIN, HIGH);
            break;
        case BLINK_INIT:
            digitalWrite(LED_BUILTIN, LOW);
            delay(100);
            digitalWrite(LED_BUILTIN, HIGH);
            delay(100);
            digitalWrite(LED_BUILTIN, LOW);
            delay(100);
            digitalWrite(LED_BUILTIN, HIGH);
            break;
        case BLINK_SENT:
            digitalWrite(LED_BUILTIN, HIGH);
            delay(50);
            digitalWrite(LED_BUILTIN, LOW);
            delay(50);
            digitalWrite(LED_BUILTIN, HIGH);
            delay(50);
            digitalWrite(LED_BUILTIN, LOW);
            break;
    }
}

void print_timestamp() {
    curr_unix_time = (millis() - setup_starttime) / 1000;

    curr_unix_time += ntp_unix_time;

    Serial.print((curr_unix_time  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if (((curr_unix_time % 3600) / 60) < 10) {
        // In the first 10 minutes of each hour, we'll want a leading '0'
        Serial.print('0');
    }
    Serial.print((curr_unix_time  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ((curr_unix_time % 60) < 10) {
        // In the first 10 seconds of each minute, we'll want a leading '0'
        Serial.print('0');
    }
    Serial.print(curr_unix_time % 60); // print the second
}

