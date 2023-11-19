/*
  Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
  Ported to Arduino ESP32 by Evandro Copercini
  updated by chegewara and MoThunderz
*/


//Initializing BLE Libraries
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

//Initializing LCD - I2C Library
#include <LiquidCrystal_I2C.h>

// Initialize all pointers
BLEServer* pServer = NULL;                    // Pointer to the server
BLECharacteristic* pCharacteristic_1 = NULL;  // Pointer to Characteristic 1

BLE2902* pBLE2902_1;  // Pointer to BLE2902 of Characteristic 1


// Some variables to keep track on device connected
bool deviceConnected = false;
bool oldDeviceConnected = false;

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

String inputValue = "";
bool newWrite = false;


// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x3F, lcdColumns, lcdRows);

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
// UUIDs used in this example:
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_1 "beb5483e-36e1-4688-b7f5-ea07361b26a8"  // CHARACTERISTIC UUID FOR WRITING LCD IC2


// Callback function that is called whenever a client is connected or disconnected
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

// Callback function that is called whenever characteristic receive data
class MyCharacteristicsCallBack: public BLECharacteristicCallbacks {

    void onWrite(BLECharacteristic *pCharacteristic_1) {
      newWrite = true;
      std::string rxValue = pCharacteristic_1->getValue();
       inputValue = rxValue.c_str();
       Serial.println(inputValue);
      }
};

void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("ESP32 Microcontroller");

  // Create the BLE Server
  pServer = BLEDevice::createServer();

  //Create Callback for Server
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService* pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic_1 = pService->createCharacteristic(
    CHARACTERISTIC_UUID_1,
    BLECharacteristic::PROPERTY_WRITE);

  //Create Callback for Characteristics
  pCharacteristic_1->setCallbacks(new MyCharacteristicsCallBack());


  // Create a BLE Descriptor
  pBLE2902_1 = new BLE2902();
  pBLE2902_1->setNotifications(true);
  pCharacteristic_1->addDescriptor(pBLE2902_1);

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");

  // initialize LCD
  lcd.init();

  // turn on LCD backlight
  lcd.backlight();

  //Setting cursor index
  lcd.setCursor(0, 0);

}





void loop() {
  if (deviceConnected) {

    //Boolean to avoid loop
    if (newWrite) {

      // print message
      lcd.clear();
      lcd.print(inputValue);

      newWrite = false;
    }
  }
  
  // The code below keeps the connection status uptodate:
  // Disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);                   // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();  // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // Connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}