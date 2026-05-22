#include "led_blinky.h"

const char* morseDictionary[] = {
  ".-",     // A
  "-...",   // B
  "-.-.",   // C
  "-..",    // D
  ".",      // E
  "..-.",   // F
  "--.",    // G
  "....",   // H
  "..",     // I
  ".---",   // J
  "-.-",    // K
  ".-..",   // L
  "--",     // M
  "-.",     // N
  "---",    // O
  ".--.",   // P
  "--.-",   // Q
  ".-.",    // R
  "...",    // S
  "-",      // T
  "..-",    // U
  "...-",   // V
  ".--",    // W
  "-..-",   // X
  "-.--",   // Y
  "--..",   // Z
  "-----",  // 0
  ".----",  // 1
  "..---",  // 2
  "...--",  // 3
  "....-",  // 4
  ".....",  // 5
  "-....",  // 6
  "--...",  // 7
  "---..",  // 8
  "----."   // 9
};

#define DOT_DURATION 100      
#define DASH_DURATION 300     
#define GAP_IN_CHAR 100       
#define GAP_BETWEEN_CHAR 300  
#define GAP_BETWEEN_WORD 700  

const char* getMorseCode(char c) {
  if (c >= 'A' && c <= 'Z') {
    return morseDictionary[c - 'A'];
  } else if (c >= 'a' && c <= 'z') {
    return morseDictionary[c - 'a'];
  } else if (c >= '0' && c <= '9') {
    return morseDictionary[26 + (c - '0')];
  }
  return NULL;
}


void blinkMorseChar(const char* morse) {
  for (int i = 0; morse[i] != '\0'; i++) {
    if (morse[i] == '.') {
      digitalWrite(LED_GPIO, HIGH);
      vTaskDelay(pdMS_TO_TICKS(DOT_DURATION));
      digitalWrite(LED_GPIO, LOW);
    } else if (morse[i] == '-') {
      digitalWrite(LED_GPIO, HIGH);
      vTaskDelay(pdMS_TO_TICKS(DASH_DURATION));
      digitalWrite(LED_GPIO, LOW);
    }
    
    // Gap within character (between dots/dashes)
    if (morse[i + 1] != '\0') {
      vTaskDelay(pdMS_TO_TICKS(GAP_IN_CHAR));
    }
  }
}

void led_blinky(void *pvParameters) {
  pinMode(LED_GPIO, OUTPUT);
  digitalWrite(LED_GPIO, LOW);
  
  const char* message = (const char*)pvParameters;
  
  while (1) {
    if (message == NULL) {
      message = "HELLO";
    }
    
    for (int i = 0; message[i] != '\0'; i++) {
      if (message[i] == ' ') {
        vTaskDelay(pdMS_TO_TICKS(GAP_BETWEEN_WORD));
      } else {
        const char* morse = getMorseCode(message[i]);
        if (morse != NULL) {
          blinkMorseChar(morse);
          // Gap between characters
          vTaskDelay(pdMS_TO_TICKS(GAP_BETWEEN_CHAR));
        }
      }
    }
    
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}
