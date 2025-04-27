#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEHIDDevice.h>
#include <BLECharacteristic.h>
#include <HIDTypes.h>
#include <HIDKeyboardTypes.h>

// マクロ定義（エラーにならないやつだけ）
#define USAGE_PAGE(x) 0x05, (x)
#define USAGE(x)      0x09, (x)
#define COLLECTION(x) 0xA1, (x)
#define END_COLLECTION 0xC0
#define REPORT_ID(x)  0x85, (x)
#define REPORT_COUNT(x) 0x95, (x)
#define REPORT_SIZE(x) 0x75, (x)
#define LOGICAL_MINIMUM(x) 0x15, (x)
#define LOGICAL_MAXIMUM(x) 0x25, (x)
#define USAGE_MINIMUM(x) 0x19, (x)
#define USAGE_MAXIMUM(x) 0x29, (x)
// ※INPUTはエラーになるので使わない。0x81を直接使う。

BLEHIDDevice* hid;
BLECharacteristic* inputKeyboard;
BLECharacteristic* inputMouse;
bool deviceConnected = false;

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    BLEDevice::startAdvertising();
  }
};

void setup() {
  Serial.begin(115200);
  BLEDevice::init("ESP32 Combo");

  BLEServer* pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  hid = new BLEHIDDevice(pServer);
  inputKeyboard = hid->inputReport(1); // Report ID 1: Keyboard
  inputMouse = hid->inputReport(2);    // Report ID 2: Mouse

  hid->manufacturer()->setValue("ESP32 Inc.");
  
  hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
  hid->hidInfo(0x00, 0x01);

  const uint8_t reportMap[] = {
    USAGE_PAGE(0x01),       // Generic Desktop
    USAGE(0x06),            // Keyboard
    COLLECTION(0x01),       // Application
      REPORT_ID(0x01),
      USAGE_PAGE(0x07),     // Keyboard/Keypad
      USAGE_MINIMUM(0xE0),
      USAGE_MAXIMUM(0xE7),
      LOGICAL_MINIMUM(0x00),
      LOGICAL_MAXIMUM(0x01),
      REPORT_SIZE(0x01),
      REPORT_COUNT(0x08),
      0x81, 0x02,           // INPUT (Data,Var,Abs)
      REPORT_COUNT(0x01),
      REPORT_SIZE(0x08),
      0x81, 0x01,           // INPUT (Constant)
      REPORT_COUNT(0x06),
      REPORT_SIZE(0x08),
      LOGICAL_MINIMUM(0x00),
      LOGICAL_MAXIMUM(0x65),
      USAGE_MINIMUM(0x00),
      USAGE_MAXIMUM(0x65),
      0x81, 0x00,           // INPUT (Data,Array)
    END_COLLECTION,

    USAGE_PAGE(0x01),       // Generic Desktop
    USAGE(0x02),            // Mouse
    COLLECTION(0x01),       // Application
      REPORT_ID(0x02),
      USAGE(0x01),          // Pointer
      COLLECTION(0x00),     // Physical
        USAGE_PAGE(0x09),   // Buttons
        USAGE_MINIMUM(0x01),
        USAGE_MAXIMUM(0x03),
        LOGICAL_MINIMUM(0x00),
        LOGICAL_MAXIMUM(0x01),
        REPORT_COUNT(0x03),
        REPORT_SIZE(0x01),
        0x81, 0x02,         // INPUT (Data,Var,Abs)
        REPORT_COUNT(0x01),
        REPORT_SIZE(0x05),
        0x81, 0x01,         // INPUT (Constant)
        USAGE_PAGE(0x01),
        USAGE(0x30),        // X
        USAGE(0x31),        // Y
        USAGE(0x38),        // Wheel
        LOGICAL_MINIMUM(0x81),
        LOGICAL_MAXIMUM(0x7F),
        REPORT_SIZE(0x08),
        REPORT_COUNT(0x03),
        0x81, 0x06,         // INPUT (Data,Var,Rel)
      END_COLLECTION,
    END_COLLECTION
  };

  hid->reportMap((uint8_t*)reportMap, sizeof(reportMap));
  hid->startServices();

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setAppearance(HID_KEYBOARD);
  pAdvertising->addServiceUUID(hid->hidService()->getUUID());
  pAdvertising->start();
  
  Serial.println("Waiting for client connection...");
}

void loop() {
  if (deviceConnected) {
    Serial.println("Sending keyboard and mouse report...");

    // --- キーボード： 'h'を押して離す ---
    uint8_t keyReport[8] = {0};
    keyReport[2] = 0x0B; // HID usage ID for 'h'
    inputKeyboard->setValue(keyReport, sizeof(keyReport));
    inputKeyboard->notify();
    delay(50);

    memset(keyReport, 0, sizeof(keyReport));
    inputKeyboard->setValue(keyReport, sizeof(keyReport));
    inputKeyboard->notify();
    delay(500);

    // --- マウス：左クリック ---
    uint8_t mouseReportClick[4] = {0};
    mouseReportClick[0] = 0x01; // 左クリック押し
    mouseReportClick[1] = 0x00; // Xなし
    mouseReportClick[2] = 0x00; // Yなし
    mouseReportClick[3] = 0x00; // ホイールなし
    inputMouse->setValue(mouseReportClick, sizeof(mouseReportClick));
    inputMouse->notify();
    delay(100);

    // 左クリック離す
    memset(mouseReportClick, 0, sizeof(mouseReportClick));
    inputMouse->setValue(mouseReportClick, sizeof(mouseReportClick));
    inputMouse->notify();
    delay(500);

    // --- マウス：ホイールスクロール上 ---
    uint8_t mouseReportWheel[4] = {0};
    mouseReportWheel[0] = 0x00;
    mouseReportWheel[1] = 0x00;
    mouseReportWheel[2] = 0x00;
    mouseReportWheel[3] = 0x01; // 上スクロール
    inputMouse->setValue(mouseReportWheel, sizeof(mouseReportWheel));
    inputMouse->notify();
    delay(500);

    delay(5000);
  }

  delay(100);
}
