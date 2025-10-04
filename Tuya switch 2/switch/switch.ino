#include "TuyaIoT.h"
#include "Log.h"

// GPIO config
#define LED1_PIN      5    // LED1 output
#define BUTTON1_PIN   21   // Button1 input
#define LED2_PIN      27   // LED2 output
#define BUTTON2_PIN   14   // Button2 input
#define BUTTON_LEVEL  LOW

#define DPID_SWITCH1  1    // switch_1
#define DPID_SWITCH2  2    // switch_2

#define BUTTON_DEBOUNCE_MS  (50u)
#define BUTTON_LONG_MS      (3000u)

// Tuya license
#define TUYA_DEVICE_UUID     "uuide14242184fe07580"
#define TUYA_DEVICE_AUTHKEY  "eRG3EiKX2iJ6CQSXRLoTpTneLDIoKKMg"

void tuyaIoTEventCallback(tuya_event_msg_t *event);
void buttonCheck1(void);
void buttonCheck2(void);

bool led1Status = false;
bool led2Status = false;

void setup() {
  Serial.begin(115200);
  Log.begin();

  pinMode(LED1_PIN, OUTPUT);
  digitalWrite(LED1_PIN, LOW);

  pinMode(LED2_PIN, OUTPUT);
  digitalWrite(LED2_PIN, LOW);

  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);

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

  TuyaIoT.begin("scogwldomvgdal4l", PROJECT_VERSION);
}

void loop() {
  buttonCheck1();
  buttonCheck2();
  delay(10);
}

// === Control LED1 ===
void setLed1(bool on) {
  led1Status = on;
  digitalWrite(LED1_PIN, on ? HIGH : LOW);
  TuyaIoT.write(DPID_SWITCH1, on);
  Serial.print("LED1 ");
  Serial.println(on ? "ON" : "OFF");
}

// === Control LED2 ===
void setLed2(bool on) {
  led2Status = on;
  digitalWrite(LED2_PIN, on ? HIGH : LOW);
  TuyaIoT.write(DPID_SWITCH2, on);
  Serial.print("LED2 ");
  Serial.println(on ? "ON" : "OFF");
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
          bool sw = false;
          TuyaIoT.read(event, DPID_SWITCH1, sw);
          setLed1(sw);
        } else if (dpid == DPID_SWITCH2) {
          bool sw = false;
          TuyaIoT.read(event, DPID_SWITCH2, sw);
          setLed2(sw);
        }
      }
    } break;

    default: break;
  }
}

// === Button1 xử lý ===
void buttonClick1() {
  Serial.println("Button1 clicked - toggle LED1");
  setLed1(!led1Status);
}

void buttonLongPress1() {
  Serial.println("Button1 long press - remove Tuya IoT device");
  TuyaIoT.remove();
}

void buttonCheck1(void) {
  static uint32_t pressMs = 0;
  static uint8_t state = 0;

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
void buttonClick2() {
  Serial.println("Button2 clicked - toggle LED2");
  setLed2(!led2Status);
}

void buttonCheck2(void) {
  static uint32_t pressMs = 0;
  static uint8_t state = 0;

  if (digitalRead(BUTTON2_PIN) == BUTTON_LEVEL) {
    if (state == 0) { pressMs = millis(); state = 1; }
    if (state == 1 && (millis() - pressMs) > BUTTON_DEBOUNCE_MS) state = 2;
    if (state == 2 && (millis() - pressMs) >= BUTTON_LONG_MS) { state = 3; /* có thể thêm chức năng khác nếu muốn */ }
  } else {
    if (state == 2 && (millis() - pressMs) < BUTTON_LONG_MS) buttonClick2();
    state = 0;
  }
}