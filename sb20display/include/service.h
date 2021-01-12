#ifndef __SERVICE_H__
#define __SERVICE_H__

#include "Arduino.h"
#include "BLECharacteristic.h"
#include "BLEUtils.h"

#include "model.h"

//#define SERVICE_DEBUG

class ConfigService : public BLECharacteristicCallbacks, public ModelListener {
private:
  SB20Model         *model;

  BLEServer         *server = nullptr;
  BLEService        *service = nullptr;
  BLECharacteristic *num_chainrings_char = nullptr;
  BLECharacteristic *chainrings_char = nullptr;
  BLECharacteristic *num_cogs_char = nullptr;
  BLECharacteristic *cogs_char = nullptr;
  BLECharacteristic *uuid_char = nullptr;

  bool               updating = false;

  static ConfigService *singleton;

  void assignCharacteristics();

public:
  /*
   * From https://www.uuidgenerator.net/
   *
   * Used:
   *
   * 5edeede4-50f8-11eb-ae93-0242ac130002
   * 5edef03c-50f8-11eb-ae93-0242ac130002
   * 5edef14a-50f8-11eb-ae93-0242ac130002
   * 5edef488-50f8-11eb-ae93-0242ac130002
   * 5edef578-50f8-11eb-ae93-0242ac130002
   * 5edef654-50f8-11eb-ae93-0242ac130002
   *
   *
   * Available:
   *
   * 5edef730-50f8-11eb-ae93-0242ac130002
   * 5edef802-50f8-11eb-ae93-0242ac130002
   * 5edef924-50f8-11eb-ae93-0242ac130002
   * 5edefa0a-50f8-11eb-ae93-0242ac130002
   */
  constexpr static const char * config_service        = "5edeede4-50f8-11eb-ae93-0242ac130002";
  constexpr static const char * config_num_chainrings = "5edef03c-50f8-11eb-ae93-0242ac130002";
  constexpr static const char * config_chainrings     = "5edef14a-50f8-11eb-ae93-0242ac130002";
  constexpr static const char * config_num_cogs       = "5edef488-50f8-11eb-ae93-0242ac130002";
  constexpr static const char * config_cogs           = "5edef578-50f8-11eb-ae93-0242ac130002";
  constexpr static const char * config_uuid           = "5edef654-50f8-11eb-ae93-0242ac130002";

  static ConfigService * instance() {
    return singleton;
  }

  static ConfigService * getService(SB20Model *model) {
    if (!singleton) {
      singleton = new ConfigService(model);
    }
    return singleton;
  }

  explicit ConfigService(SB20Model *model);
  virtual ~ConfigService();

  void onSetup();
  void onModelUpdate();

	void onRead(BLECharacteristic *);
  void onWrite(BLECharacteristic *);
	void onNotify(BLECharacteristic *);
//	virtual void onStatus(BLECharacteristic *, BLECharacteristicCallbacks::Status, uint32_t);
};

#endif /* __SERVICE_H__ */