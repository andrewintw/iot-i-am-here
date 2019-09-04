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

void _print2digit(int t) {
    if (t < 10) Serial.print('0');
    Serial.print(t);
}

void print_timestamp() {
    unsigned long now_ms = millis() / 1000;
    int t_sec = now_ms % 60;
    int t_min = (now_ms / 60) % 60;
    int t_hor = (now_ms / 3600) % 24;
    _print2digit(t_hor); Serial.print(':');
    _print2digit(t_min); Serial.print(':');
    _print2digit(t_sec);
}

