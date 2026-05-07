#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include <ESP32Servo.h>

// ==============================
// Actuation Hardware
// ==============================
Servo deployServo;
const int SERVO_PIN = 14;

const int SERVO_LOCKED_POS = 0;
const int SERVO_DEPLOY_POS = 90;

bool deployed = false;

// ==============================
// External Input
// ==============================
const int OVERRIDE_PIN = 23;

// ==============================
// BLE UUIDs (UNCHANGED CONTRACT)
// ==============================
#define WEAR_SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define WEAR_TX_UUID      "12345678-1234-1234-1234-1234567890ac"
#define WEAR_RX_UUID      "12345678-1234-1234-1234-1234567890ad"

BLERemoteCharacteristic* wearTx = nullptr;
BLERemoteCharacteristic* wearRx = nullptr;

BLEClient* wearClient = nullptr;
BLEAdvertisedDevice* wearDev = nullptr;

bool wearConnected = false;

// ==============================
// System Interface Layer
// (intentional abstraction boundary)
// ==============================
volatile bool streamReady = false;
volatile bool fallEvent = false;
volatile bool resetRequest = false;

// ==============================
// Internal safety gating (abstracted)
// ==============================
bool deploymentAllowed = false;

// =====================================================
// BLE SCAN CALLBACK
// =====================================================
class WearScanCB : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice d) override {

    if (d.haveServiceUUID() &&
        d.isAdvertisingService(BLEUUID(WEAR_SERVICE_UUID))) {

      Serial.println("Wearable found");

      if (wearDev) delete wearDev;
      wearDev = new BLEAdvertisedDevice(d);

      BLEDevice::getScan()->stop();
    }
  }
};

// =====================================================
// CLIENT CALLBACK
// =====================================================
class WearClientCallbacks : public BLEClientCallbacks {
  void onDisconnect(BLEClient* client) override {

    wearConnected = false;
    streamReady = false;
    fallEvent = false;
    resetRequest = false;

    wearTx = nullptr;
    wearRx = nullptr;

    Serial.println("Wearable disconnected");
  }
};

// =====================================================
// ACTUATION LAYER
// =====================================================
void deploy() {
  if (!deployed) {
    deployServo.write(SERVO_DEPLOY_POS);
    deployed = true;
    Serial.println("Deploy executed");
  }
}

void resetSystem() {
  deployed = false;
  deployServo.write(SERVO_LOCKED_POS);
  Serial.println("System reset");
}

// =====================================================
// CONNECT
// =====================================================
void connectToWearable() {

  wearClient = BLEDevice::createClient();
  wearClient->setClientCallbacks(new WearClientCallbacks());

  if (!wearClient->connect(wearDev)) {
    wearDev = nullptr;
    return;
  }

  BLERemoteService* svc = wearClient->getService(WEAR_SERVICE_UUID);
  if (!svc) return;

  wearTx = svc->getCharacteristic(WEAR_TX_UUID);
  wearRx = svc->getCharacteristic(WEAR_RX_UUID);

  if (!wearTx || !wearRx) return;

  wearTx->registerForNotify(
    [](BLERemoteCharacteristic*, uint8_t* data, size_t len, bool) {

      String msg;
      for (int i = 0; i < len; i++) msg += (char)data[i];

      Serial.println("RX: " + msg);

      // KEEP PROTOCOL EXACT (NO REDACTION HERE)
      if (msg == "<ACK_WEARABLE>") {
        streamReady = true;
      }

      if (msg == "<FALL_DETECTED>") {
        fallEvent = true;
      }
    }
  );

  wearConnected = true;
}

// ==============================
// INPUT ABSTRACTION LAYER
// (hardware stabilization included)
// ==============================
bool readOverrideEvent() {
  static unsigned long lastTrigger = 0;
  const unsigned long debounceWindow = 700;

  bool pressed = (digitalRead(OVERRIDE_PIN) == LOW);

  if (pressed && (millis() - lastTrigger > debounceWindow)) {
    lastTrigger = millis();
    return true;
  }
  return false;
}

// =====================================================
// SETUP
// =====================================================
void setup() {
  Serial.begin(115200);

  BLEDevice::init("PlatformModule");

  deployServo.attach(SERVO_PIN, 500, 2500);
  deployServo.write(SERVO_LOCKED_POS);

  pinMode(OVERRIDE_PIN, INPUT_PULLUP);
}

// =====================================================
// LOOP
// =====================================================
void loop() {

  if (wearDev) {
    connectToWearable();
    wearDev = nullptr;
  }

  if (wearConnected && streamReady && wearRx) {
    wearRx->writeValue("START_STREAM");
    streamReady = false;
  }

  // ==============================
  // EVENT HANDLING LAYER
  // (original logic intentionally abstracted)
  // ==============================
  deploymentAllowed = !readOverrideEvent();

  if (fallEvent && deploymentAllowed) {
    deploy();
    fallEvent = false;
  }

  if (readOverrideEvent()) {
    resetRequest = true;
  }

  if (resetRequest) {
    if (deployed) resetSystem();
    resetRequest = false;
  }

  yield();
}
