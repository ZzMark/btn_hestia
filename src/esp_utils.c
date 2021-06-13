
#include <Arduino.h>
#include <user_interface.h>

// void espSleep(uint64_t seconds)
// {
//     uint64_t USEC = 1000000;
// #ifdef ESP32
//     Serial.printf("Going to sleep %llu seconds\n", seconds);
//     esp_sleep_enable_timer_wakeup(seconds * USEC);
//     esp_deep_sleep_start();
// #elif ESP8266
//     Serial.printf("Going to sleep %llu s. Waking up only if D0 is connected to RST \n", seconds);
//     ESP.deepSleep(seconds * USEC); // 3600e6 = 1 hour in seconds / ESP.deepSleepMax()
// #endif
// }

// 普通的休眠，等待 x ms
// 不会断开各种连接
void service_sleep(uint64_t millis)
{
    if (millis < 500)
    {
        delay(millis);
    }
    else
    {
        uint64_t remain = millis;
        while(remain > 0)
        {
            // 定期喂狗
            system_soft_wdt_feed();
            delay(500 - (remain % 500));
            remain = remain - 500;
        }
    }
}
