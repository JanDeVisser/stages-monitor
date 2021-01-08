#include "Arduino.h"

#include "service.h"

ConfigService::ConfigService(SB20Model *model) : model(model) {
}

ConfigService::~ConfigService() {
}

void ConfigService::onSetup(void) {
  server = BLEDevice::createServer();
  service = server->createService(config_service);
  Serial.println("Server side BLE service created");

  this -> num_chainrings_char = service->createCharacteristic(
    config_num_chainrings,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  this -> num_chainrings_char -> setCallbacks(this);

  this -> chainrings_char = service->createCharacteristic(
    config_chainrings,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  this -> chainrings_char -> setCallbacks(this);

  this -> num_cogs_char = service->createCharacteristic(
    config_num_cogs,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  this -> num_cogs_char -> setCallbacks(this);

  this -> cogs_char = service->createCharacteristic(
    config_cogs,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  this -> cogs_char -> setCallbacks(this);

  this -> uuid_char = service->createCharacteristic(
    config_uuid,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  this -> uuid_char -> setCallbacks(this);

  Serial.println("Created config service and characteristics");

  assignCharacteristics();

  service -> start();
  Serial.println("Server side BLE service started");

  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising -> addServiceUUID(config_service);
  advertising -> setScanResponse(true);
  advertising -> setMinPreferred(0x06);  // functions that help with iPhone connections issue
  advertising -> setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("BLE service advertised");
}

void ConfigService::assignCharacteristics() {
  uint8_t num = model -> num_chainrings();
  this -> num_chainrings_char -> setValue(&num, 1);

  uint8_t *bytes = new uint8_t[model -> num_chainrings()];
  for (int ix = 0; ix < model -> num_chainrings(); ix++) {
    bytes[ix] = model -> chainring(ix+1);
  }
  this -> chainrings_char -> setValue(bytes, model -> num_chainrings());
  delete[] bytes;

  num = model -> num_cogs();
  this -> num_chainrings_char -> setValue(&num, 1);

  bytes = new uint8_t[model -> num_cogs()];
  for (int ix = 0; ix < model -> num_cogs(); ix++) {
    bytes[ix] = model -> cog(ix+1);
  }
  this -> cogs_char -> setValue(bytes, model -> num_cogs());
  delete[] bytes;

  this -> uuid_char -> setValue(model -> uuid());

  Serial.println("Characteristic values assigned");
}

void ConfigService::onModelUpdate() {
  if (!updating) {
    assignCharacteristics();
  }
}

void ConfigService::onRead(BLECharacteristic* characteristic) {
  Serial.print("Read of char ");
  Serial.println(characteristic -> toString().c_str());
}

void ConfigService::onWrite(BLECharacteristic* characteristic) {
  if (updating) {
    return;
  }
  updating = true;
  Configuration c = this -> model -> configuration();
  Serial.print("Write of char ");
  if (characteristic == this -> num_chainrings_char) {
    Serial.println("#chainrings");
    c.num_chainrings = *(characteristic -> getData());
  } else if (characteristic == this -> chainrings_char) {
    Serial.println("chainrings");
    delete[] c.chainrings;
    c.chainrings = new uint8_t[c.num_chainrings];
    memcpy(c.chainrings, characteristic->getData(), c.num_chainrings);
  } else if (characteristic == this -> num_cogs_char) {
    Serial.println("#cogs");
    c.num_cogs = *(characteristic -> getData());
  } else if (characteristic == this -> cogs_char) {
    Serial.println("cogs");
    delete[] c.cogs;
    c.cogs = new uint8_t[c.num_cogs];
    memcpy(c.cogs, characteristic -> getData(), c.num_cogs);
  } else if (characteristic == this -> uuid_char) {
    Serial.println("uuid");
    c.uuid = characteristic -> getValue();
  } else {
    Serial.println(characteristic -> toString().c_str());
  }
  this -> model -> configuration(c);
}

void ConfigService::onNotify(BLECharacteristic* characteristic) {
  Serial.print("Notify of char ");
  Serial.println(characteristic -> toString().c_str());
}

//void ConfigService::onStatus(BLECharacteristic* characteristic, BLECharacteristicCallbacks::Status s, uint32_t code) {
//  Serial.print("Status of char ");
//  Serial.print(characteristic -> toString().c_str());
//  Serial.print(": ");
//  switch (s) {
//    case SUCCESS_INDICATE:
//      Serial.print("SUCCESS_INDICATE");
//      break;
//    case SUCCESS_NOTIFY:
//      Serial.print("SUCCESS_NOTIFY");
//      break;
//    case ERROR_INDICATE_DISABLED:
//      Serial.print("ERROR_INDICATE_DISABLED");
//      break;
//    case ERROR_NOTIFY_DISABLED:
//      Serial.print("ERROR_NOTIFY_DISABLED");
//      break;
//    case ERROR_GATT:
//      Serial.print("ERROR_GATT");
//      break;
//    case ERROR_NO_CLIENT:
//      Serial.print("ERROR_NO_CLIENT");
//      break;
//    case ERROR_INDICATE_TIMEOUT:
//      Serial.print("ERROR_INDICATE_TIMEOUT");
//      break;
//    case ERROR_INDICATE_FAILURE:
//      Serial.print("ERROR_INDICATE_FAILURE");
//      break;
//  }
//  Serial.print(", code: ");
//  Serial.println(code);
//}

