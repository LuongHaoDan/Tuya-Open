#include "TuyaIoT.h"
#include "Log.h"


// GPIO config
#define LED_PIN      5    // LED output
#define BUTTON_PIN   21   // Button input
#define BUTTON_LEVEL LOW

#define DPID_SWITCH  1    // Theo Tuya Cloud (switch_1)

#define BUTTON_DEBOUNCE_MS  (50u)
#define BUTTON_LONG_MS      (3000u)

// Tuya license (cần thay bằng license của bạn)
#define TUYA_DEVICE_UUID     "uuide14242184fe07580"
#define TUYA_DEVICE_AUTHKEY  "eRG3EiKX2iJ6CQSXRLoTpTneLDIoKKMg"

void tuyaIoTEventCallback(tuya_event_msg_t *event);
void buttonCheck(void);

bool ledStatus = false;  // Trạng thái LED

void setup() {
  Serial.begin(115200);
  Log.begin();

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

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

  // PROJECT_VERSION lấy từ appConfig.json
  TuyaIoT.begin("tkjozobua4vu3qgm", PROJECT_VERSION);
}

void loop() {
  buttonCheck();
  delay(10);
}

// === Control LED ===
void setLed(bool on) {
  ledStatus = on;
  digitalWrite(LED_PIN, on ? HIGH : LOW);
  TuyaIoT.write(DPID_SWITCH, on); // gửi trạng thái lên cloud
  if (on) {
    Serial.println("LED ON");
  } else {
    Serial.println("LED OFF");
  }
}

// === Callback khi nhận event từ Tuya Cloud/App ===
void tuyaIoTEventCallback(tuya_event_msg_t *event) {
  tuya_event_id_t event_id = TuyaIoT.eventGetId(event);
  switch (event_id) {
    case TUYA_EVENT_DP_RECEIVE_OBJ: {
      uint16_t dpNum = TuyaIoT.eventGetDpNum(event);
      for (uint16_t i = 0; i < dpNum; i++) {
        uint8_t dpid = TuyaIoT.eventGetDpId(event, i);
        if (dpid == DPID_SWITCH) {
          bool sw = false;
          TuyaIoT.read(event, DPID_SWITCH, sw);
          setLed(sw);
        }
      }
    } break;

    default: break;
  }
}

// === Xử lý button ===
void buttonClick() {
  Serial.println("Button clicked - toggle LED");
  setLed(!ledStatus);  // đảo trạng thái LED
}

void buttonLongPressStart() {
  Serial.println("Long press - remove Tuya IoT device");
  TuyaIoT.remove();
}

void buttonCheck(void) {
  static uint32_t pressMs = 0;
  static uint8_t state = 0;

  if (digitalRead(BUTTON_PIN) == BUTTON_LEVEL) {
    if (state == 0) {
      pressMs = millis();
      state = 1;
    }
    if (state == 1 && (millis() - pressMs) > BUTTON_DEBOUNCE_MS) {
      state = 2;
    }
    if (state == 2 && (millis() - pressMs) >= BUTTON_LONG_MS) {
      state = 3;
      buttonLongPressStart();
    }
  } else {
    if (state == 2) {
      if ((millis() - pressMs) < BUTTON_LONG_MS) {
        buttonClick();
      } else {
        buttonLongPressStart();
      }
    }
    state = 0;
  }
}
