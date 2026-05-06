#include <MPU6050.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ==============================
// Hardware
// ==============================
MPU6050 mpu;

// ==============================
// BLE UUIDs
// ==============================
#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHAR_UUID_TX "12345678-1234-1234-1234-1234567890ac"
#define CHAR_UUID_RX "12345678-1234-1234-1234-1234567890ad"

BLEServer* pServer = nullptr;
BLECharacteristic* pTx = nullptr;
BLECharacteristic* pRx = nullptr;

bool deviceConnected = false;
bool streaming = false;

// ==============================
// Timing
// ==============================
unsigned long lastStatusSend = 0;
const unsigned long statusInterval = 1000;

// ==============================
// System State (abstracted)
// ==============================
bool eventDetected = false;
int eventCounter = 0;

// Raw sensor data
int16_t ax, ay, az;
int16_t gx, gy, gz;

// ==============================
// Abstracted Motion Processing
// ==============================
//
// NOTE:
// Core motion interpretation and event classification logic
// has been intentionally abstracted for intellectual property protection.
//
bool evaluateMotionEvent(int16_t ax, int16_t ay, int16_t az,
                         int16_t gx, int16_t gy, int16_t gz)
{
  // Proprietary motion interpretation algorithm (not included)
  return false;
}

// ==============================
// BLE Callbacks
// ==============================
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* server) override {
    deviceConnected = true;
    Serial.println("Wearable: Connected");
  }

  void onDisconnect(BLEServer* server) override {
    deviceConnected = false;
    streaming = false;
    Serial.println("Wearable: Disconnected");
    server->startAdvertising();
  }
};

class RxCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* characteristic) override {

    String cmd = characteristic->getValue();
    if (cmd.length() == 0) return;

    Serial.print("CMD Received: ");
    Serial.println(cmd);

    if (cmd == "HELLO_WEARABLE") {
      if (deviceConnected && pTx) {
        pTx->setValue("<ACK_WEARABLE>");
        pTx->notify();
      }
    }

    else if (cmd == "START_STREAM") {
      streaming = true;
      Serial.println("Streaming ENABLED");
    }

    else if (cmd == "STOP_STREAM") {
      streaming = false;
      Serial.println("Streaming DISABLED");
    }
  }
};

// ==============================
// Setup
// ==============================
void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);
  mpu.initialize();

  Serial.println(mpu.testConnection()
    ? "IMU connected"
    : "IMU failed");

  BLEDevice::init("Wearable");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *service = pServer->createService(SERVICE_UUID);

  pTx = service->createCharacteristic(
    CHAR_UUID_TX,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pTx->addDescriptor(new BLE2902());

  pRx = service->createCharacteristic(
    CHAR_UUID_RX,
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_WRITE_NR
  );
  pRx->setCallbacks(new RxCallbacks());

  service->start();

  BLEAdvertising *adv = pServer->getAdvertising();
  adv->addServiceUUID(SERVICE_UUID);
  adv->start();

  Serial.println("BLE advertising started");
}

// ==============================
// Loop
// ==============================
void loop() {

  // Read IMU (kept for system realism)
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // ==============================
  // Event Detection (ABSTRACTED)
  // ==============================
  if (streaming && deviceConnected) {

    eventDetected = evaluateMotionEvent(ax, ay, az, gx, gy, gz);

    if (eventDetected) {
      pTx->setValue("<EVENT_TRIGGER>");
      pTx->notify();

      eventCounter++;
      eventDetected = false;
    }

    // Periodic status update
    unsigned long now = millis();
    if (now - lastStatusSend >= statusInterval) {
      lastStatusSend = now;

      char buffer[48];
      snprintf(buffer, sizeof(buffer),
               "<STATUS:OK>");

      pTx->setValue(buffer);
      pTx->notify();

      Serial.println("Status sent");
    }
  }

  yield();
}
