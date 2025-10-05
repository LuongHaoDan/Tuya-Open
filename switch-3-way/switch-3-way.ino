
#include "TuyaIoT.h"
#include "Log.h"

// GPIO config
#define LED1_PIN      5     // LED1 output
#define BUTTON1_PIN   21    // Button1 input
#define LED2_PIN      27    // LED2 output
#define BUTTON2_PIN   14    // Button2 input
#define LED3_PIN      23    // LED3 output
#define BUTTON3_PIN   22    // Button3 input
#define BUTTON_LEVEL  LOW

// DP define (theo Tuya template)
#define DPID_SWITCH1     1   // switch_1
#define DPID_SWITCH2     2   // switch_2
#define DPID_SWITCH3     3   // switch_3
#define DPID_COUNTDOWN1  7   // countdown_1
#define DPID_COUNTDOWN2  8   // countdown_2
#define DPID_COUNTDOWN3  9   // countdown_3

#define BUTTON_DEBOUNCE_MS  (50u)
#define BUTTON_LONG_MS      (3000u)

// Tuya license
#define TUYA_DEVICE_UUID     "uuide14242184fe07580"
#define TUYA_DEVICE_AUTHKEY  "eRG3EiKX2iJ6CQSXRLoTpTneLDIoKKMg"

// --- Forward declarations (prototypes) ---
// đảm bảo compiler biết các hàm trước khi sử dụng
void tuyaIoTEventCallback(tuya_event_msg_t *event);
void buttonCheck1(void);
void buttonCheck2(void);
void buttonCheck3(void);
void setLed1(bool on);
void setLed2(bool on);
void setLed3(bool on);

// trạng thái LED
bool led1Status = false;
bool led2Status = false;
bool led3Status = false;

// countdown timer (giây)
uint32_t countdown1 = 0;
uint32_t countdown2 = 0;
uint32_t countdown3 = 0;
uint32_t lastTick = 0;

void setup() {
  Serial.begin(115200);
  Log.begin();

  pinMode(LED1_PIN, OUTPUT); digitalWrite(LED1_PIN, LOW);
  pinMode(LED2_PIN, OUTPUT); digitalWrite(LED2_PIN, LOW);
  pinMode(LED3_PIN, OUTPUT); digitalWrite(LED3_PIN, LOW);

  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);

  TuyaIoT.setEventCallback(tuyaIoTEventCallback);

  // license
  tuya_iot_license_t license;
  int rt = TuyaIoT.readBoardLicense(&license);
  if (OPRT_OK != rt) {
    license.uuid = TUYA_DEVICE_UUID;
    license.authkey = TUYA_DEVICE_AUTHKEY;
    Serial.println("Replace TUYA_DEVICE_UUID and TUYA_DEVICE_AUTHKEY with your real values!");
  }
  TuyaIoT.setLicense(license.uuid, license.authkey);

  TuyaIoT.begin("utkzbw3ywercgp2m", PROJECT_VERSION);

  lastTick = millis();
}

void loop() {
  buttonCheck1();
  buttonCheck2();
  buttonCheck3();

  // xử lý countdown mỗi 1s
  if (millis() - lastTick >= 1000) {
    lastTick = millis();

    if (countdown1 > 0) {
      countdown1--;
      if (countdown1 == 0) {
        // khi countdown về 0 -> tắt (hoặc toggle tuỳ yêu cầu)
        // ở đây mình tắt LED (thông dụng hơn)
        setLed1(false);
        TuyaIoT.write(DPID_COUNTDOWN1, 0);
      } else {
        // report hiện thời gian còn lại nếu cần (tuỳ bạn)
        // TuyaIoT.write(DPID_COUNTDOWN1, countdown1);
      }
    }

    if (countdown2 > 0) {
      countdown2--;
      if (countdown2 == 0) {
        setLed2(false);
        TuyaIoT.write(DPID_COUNTDOWN2, 0);
      }
    }

    if (countdown3 > 0) {
      countdown3--;
      if (countdown3 == 0) {
        setLed3(false);
        TuyaIoT.write(DPID_COUNTDOWN3, 0);
      }
    }
  }

  delay(10);
}

// === Control LED ===
void setLed1(bool on) {
  led1Status = on;
  digitalWrite(LED1_PIN, on ? HIGH : LOW);
  TuyaIoT.write(DPID_SWITCH1, on);
  Serial.print("LED1 "); Serial.println(on ? "ON" : "OFF");
}

void setLed2(bool on) {
  led2Status = on;
  digitalWrite(LED2_PIN, on ? HIGH : LOW);
  TuyaIoT.write(DPID_SWITCH2, on);
  Serial.print("LED2 "); Serial.println(on ? "ON" : "OFF");
}

void setLed3(bool on) {
  led3Status = on;
  digitalWrite(LED3_PIN, on ? HIGH : LOW);
  TuyaIoT.write(DPID_SWITCH3, on);
  Serial.print("LED3 "); Serial.println(on ? "ON" : "OFF");
}

// === Callback từ Tuya Cloud ===
void tuyaIoTEventCallback(tuya_event_msg_t *event) {
  tuya_event_id_t event_id = TuyaIoT.eventGetId(event);
  switch (event_id) {
    case TUYA_EVENT_DP_RECEIVE_OBJ: {
      uint16_t dpNum = TuyaIoT.eventGetDpNum(event);
      for (uint16_t i = 0; i < dpNum; i++) {
        uint8_t dpid = TuyaIoT.eventGetDpId(event, i);
        if (dpid == DPID_SWITCH1) {
          bool sw; TuyaIoT.read(event, DPID_SWITCH1, sw); setLed1(sw);
        } else if (dpid == DPID_SWITCH2) {
          bool sw; TuyaIoT.read(event, DPID_SWITCH2, sw); setLed2(sw);
        } else if (dpid == DPID_SWITCH3) {
          bool sw; TuyaIoT.read(event, DPID_SWITCH3, sw); setLed3(sw);
        } else if (dpid == DPID_COUNTDOWN1) {
          uint32_t val; TuyaIoT.read(event, DPID_COUNTDOWN1, val);
          countdown1 = val;
          Serial.print("Countdown1 set: ");
          Serial.print(val);
          Serial.println(" sec");
          // khi đặt giá trị countdown trên app, cũng có thể bật LED nếu cần:
          if (val > 0) setLed1(true);
        } else if (dpid == DPID_COUNTDOWN2) {
          uint32_t val; TuyaIoT.read(event, DPID_COUNTDOWN2, val);
          countdown2 = val;
          Serial.print("Countdown2 set: ");
          Serial.print(val);
          Serial.println(" sec");
          if (val > 0) setLed2(true);
        } else if (dpid == DPID_COUNTDOWN3) {
          uint32_t val; TuyaIoT.read(event, DPID_COUNTDOWN3, val);
          countdown3 = val;
          Serial.print("Countdown3 set: ");
          Serial.print(val);
          Serial.println(" sec");
          if (val > 0) setLed3(true);
        }
      }
    } break;
    default: break;
  }
}

// === Button1 xử lý ===
void buttonClick1() { Serial.println("Button1 clicked - toggle LED1"); setLed1(!led1Status); }
void buttonLongPress1() { Serial.println("Button1 long press - remove Tuya IoT device"); TuyaIoT.remove(); }
void buttonCheck1(void) {
  static uint32_t pressMs = 0; static uint8_t state = 0;
  if (digitalRead(BUTTON1_PIN) == BUTTON_LEVEL) {
    if (state == 0) { pressMs = millis(); state = 1; }
    if (state == 1 && (millis() - pressMs) > BUTTON_DEBOUNCE_MS) state = 2;
    if (state == 2 && (millis() - pressMs) >= BUTTON_LONG_MS) { state = 3; buttonLongPress1(); }
  } else {
    if (state == 2 && (millis() - pressMs) < BUTTON_LONG_MS) buttonClick1();
    state = 0;
  }
}

// === Button2 xử lý ===
void buttonClick2() { Serial.println("Button2 clicked - toggle LED2"); setLed2(!led2Status); }
void buttonCheck2(void) {
  static uint32_t pressMs = 0; static uint8_t state = 0;
  if (digitalRead(BUTTON2_PIN) == BUTTON_LEVEL) {
    if (state == 0) { pressMs = millis(); state = 1; }
    if (state == 1 && (millis() - pressMs) > BUTTON_DEBOUNCE_MS) state = 2;
    if (state == 2 && (millis() - pressMs) >= BUTTON_LONG_MS) { state = 3; }
  } else {
    if (state == 2 && (millis() - pressMs) < BUTTON_LONG_MS) buttonClick2();
    state = 0;
  }
}

// === Button3 xử lý ===
void buttonClick3() { Serial.println("Button3 clicked - toggle LED3"); setLed3(!led3Status); }
void buttonCheck3(void) {
  static uint32_t pressMs = 0; static uint8_t state = 0;
  if (digitalRead(BUTTON3_PIN) == BUTTON_LEVEL) {
    if (state == 0) { pressMs = millis(); state = 1; }
    if (state == 1 && (millis() - pressMs) > BUTTON_DEBOUNCE_MS) state = 2;
    if (state == 2 && (millis() - pressMs) >= BUTTON_LONG_MS) { state = 3; }
  } else {
    if (state == 2 && (millis() - pressMs) < BUTTON_LONG_MS) buttonClick3();
    state = 0;
  }
}

