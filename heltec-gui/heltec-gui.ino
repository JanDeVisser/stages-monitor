
#include "Arduino.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include <Preferences.h>


#include "model.h"
#include "challenge.h"
#include "connector.h"

static SB20Model *model = nullptr;





static char **uuid_blacklist;
static int    uuid_blacklist_size = 0;
static int    uuid_blacklist_ptr = 0;

void add_to_blacklist(const char *uuid) {
  if (uuid_blacklist_ptr >= uuid_blacklist_size) {
    int new_sz = (uuid_blacklist_size > 0) ? 2 * uuid_blacklist_size : 10;
    char **new_blacklist = (char **) calloc(sizeof(char *), new_sz); // FIXME OOM
    if (uuid_blacklist_size > 0) {
      memcpy(new_blacklist, uuid_blacklist, uuid_blacklist_size * sizeof(char *));
    }
    uuid_blacklist_size = new_sz;
    uuid_blacklist = new_blacklist;
  }
  char *uuid_copy = (char *) malloc(strlen(uuid)); // FIXME OOM
  strcpy(uuid_copy, uuid);
  uuid_blacklist[uuid_blacklist_ptr++] = uuid_copy;
}

bool blacklisted(const char *uuid) {
  for (int ix = 0; ix < uuid_blacklist_ptr; ix++) {
    if (!strcmp(uuid_blacklist[ix], uuid)) {
      return true;
    }
  }
  return false;
}

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class FindSB20Callback : public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    if (blacklisted(advertisedDevice.toString().c_str())) {
      Serial.println("Blacklisted");
      return;
    }

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(speed_cadence_service)) {
      BLEDevice::getScan() -> stop();
      BLEAdvertisedDevice *device = new BLEAdvertisedDevice(advertisedDevice);
      sb20 = new SB20(device, model, view);
    } else {
      add_to_blacklist(advertisedDevice.toString().c_str());
    }
  }
};


void setup() {
  model = new SB20Model();
  SB20View *view = new SB20HeltecView(model);
  SB20Connector *connector = SB20Connector::getConnector(model);
  model -> onSetup();

  /* ----- CONNECTOR onSetup ------ */
  Heltec.begin(true, false, true);
  Serial.begin(115200);
  Serial.println("Starting SB20 Monitor");

  BLEDevice::init("Stages SB20 Monitor");

  view -> display_message("Scanning");

  BLEScan *scanner = BLEDevice::getScan();
  scanner -> setAdvertisedDeviceCallbacks(new FindSB20Callback());
  scanner -> setInterval(1349);
  scanner -> setWindow(449);
  scanner -> setActiveScan(true);
  scanner -> start(5, false);
}


void loop() {
  model -> onLoop();
  delay(100);


  view -> display();
  if (sb20 != nullptr) {
    if (!sb20 -> is_connected()) {
      sb20 -> connect();
      switch (sb20 -> get_error_code()) {
        case NoSB20Service:
          add_to_blacklist(sb20 -> toString().c_str());
          if (sb20) {
            delete sb20;
          }
          sb20 = nullptr;
          view -> display_message("Scanning");
          BLEDevice::getScan() -> start(0);
          break;
        case NoStatusChar:
        case NoCommandChar:
          view -> display_message("ERG Mode");
          if (sb20) {
            delete sb20;
          }
          sb20 = nullptr;
          delay(3750);
          BLEDevice::getScan() -> start(0);
          break;
        case NoError:
          sb20 -> bootstrap();
          break;
      }
    } else {
      sb20 -> handle_incoming();
    }
  } else {
    view -> display_message("Scanning");
    BLEDevice::getScan() -> start(0);
  }
}
