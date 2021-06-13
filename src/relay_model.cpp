
// 继电器部分
#include <Arduino.h>
#include "gpio.h"

#include "esp_utils.c"

// BTN DEF
#define BTN 0
#define LED 2

// STATUS DEF
#define Close 1
#define Open 0

/*
var BTNTimer = 0;
function powerBtn(delay) {
  if (delay > 0) {
    console.log(`power press. delay: ${delay}`);
    clearTimeout(BTNTimer);
    digitalWrite(BTN, Open);
    digitalWrite(LED, 1);
    BTNTimer = setTimeout(() => {
      digitalWrite(BTN, Close);
      digitalWrite(LED, 0);
    }, delay);
  }
}

// service
function handle(msg) {
  if (msg == 'short') {
    powerBtn(shortDelay);
  } else if (msg == 'long') {
    powerBtn(6000);
  } else {
    var delay = Number(msg);
    powerBtn(delay);
  }
}
*/

static uint8 RELAY_IF_EXEC = 0;

void power_btn(int delay)
{
    if (RELAY_IF_EXEC)
    {
        return;
    }
    Serial.printf("power press. delay: %d", delay);
    RELAY_IF_EXEC = 1;
    digitalWrite(BTN, Open);
    digitalWrite(LED, 1);
    service_sleep(delay); /* 延时 */
    digitalWrite(BTN, Close);
    digitalWrite(LED, 0);
    RELAY_IF_EXEC = 0;
}