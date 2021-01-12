#include "connector.h"

SB20Connector * SB20Connector::singleton = nullptr;

static void notifyCallback(BLERemoteCharacteristic *characteristic, uint8_t *data, size_t length, bool isNotify) {
#ifdef CONNECTOR_DEBUG
  Serial.print("Incoming Message: ");
  Bytes::hex_dump_nl(data, length);
#endif
  if (SB20Connector::instance() != nullptr) {
    SB20Connector::instance() -> push_message(data, length);
  }
}

SB20Connector::SB20Connector(SB20Model *model) : 
    msg_queue(),
    model(model),
    blacklist(),
    device(),
    dialogs() {
  model -> add_listener(this);
  this -> add_listener(this);
  this -> add_listener(model);
#ifdef CONNECTOR_DEBUG
  Serial.println("Connector Created");
#endif
}

bool SB20Connector::probe() {
  if (!probe_device) {
    return false;
  }
  this -> display_message("Connecting ...");
#ifdef CONNECTOR_DEBUG
  Serial.print("Probing ");
  Serial.println(probe_device -> getAddress().toString().c_str());
#endif

  if (!client) {
    client = BLEDevice::createClient();
    client -> setClientCallbacks(this);
#ifdef CONNECTOR_DEBUG
    Serial.println("Created client");
#endif
  }

  bool ret = false;
  if (client -> connect(this -> probe_device)) {
#ifdef CONNECTOR_DEBUG
    Serial.println("Connected to server");
#endif
    if (this -> is_sb20()) {
#ifdef CONNECTOR_DEBUG
      Serial.println("Connection initialized");
#endif
      ret = true;
    } else {
#ifdef CONNECTOR_DEBUG
      Serial.println("Could not initialize connection");
#endif
    }
  } else {
#ifdef CONNECTOR_DEBUG
    Serial.println("Unable to connect");
#endif
  }
  if (!ret) {
    if (this -> client -> isConnected()) {
      this->client->disconnect();
    }
    this -> blacklist.insert(this -> probe_device -> getAddress().toString());
  } else {
    this -> device = this -> probe_device -> getAddress().toString();
    this -> model -> uuid(this -> device);
  }
  this -> probe_device = nullptr;
  return ret;
}

bool SB20Connector::is_sb20() {
  bool ret = false;
  if (!(client && client -> isConnected())) {
    Serial.println("SB20Connector::is_sb20() called but not connected");
    return false;
  }

  this -> service = client -> getService(BLEUUID(sb20_service));
  if (!service) {
    this -> error_code = NoSB20Service;
#ifdef CONNECTOR_DEBUG
    Serial.println("Remote does not have the SB20 service");
#endif
    goto error_cleanup;
  }
#ifdef CONNECTOR_DEBUG
  Serial.println("Remote has the SB20 service");
#endif
  command = service -> getCharacteristic(BLEUUID(sb20_command));
  if (!command) {
    this -> error_code = NoCommandChar;
#ifdef CONNECTOR_DEBUG
    Serial.println("Command characteristic not found");
#endif
    goto error_cleanup;
  }
#ifdef CONNECTOR_DEBUG
  Serial.println("Found command characteristic");
#endif

  status = service -> getCharacteristic(BLEUUID(sb20_status));
  if (!status) {
    this -> error_code = NoStatusChar;
#ifdef CONNECTOR_DEBUG
    Serial.println("Status characteristic not found");
#endif
    goto error_cleanup;
  }
#ifdef CONNECTOR_DEBUG
  Serial.println("Found status characteristic");
#endif

  if (status -> canNotify()) {
    status -> registerForNotify(notifyCallback);
  }
#ifdef CONNECTOR_DEBUG
  Serial.println("Connector set up");
#endif
  this -> connect_time = this -> last_contact = this -> last_shift = millis();
  this -> bootstrap();
  ret = true;
  goto exit;

error_cleanup:
  this -> client -> disconnect();
  this -> service = nullptr;

exit:
  return ret;
}

bool SB20Connector::connect(const std::string &uuid) {
  if (!this -> client) {
    this -> client = BLEDevice::createClient();
    this -> client -> setClientCallbacks((BLEClientCallbacks *) this);
#ifdef CONNECTOR_DEBUG
    Serial.print("Created client for ");
    Serial.println(uuid.c_str());
#endif
  }

  BLEAddress addr(uuid);
  if (client -> connect(addr, BLE_ADDR_TYPE_RANDOM)) {
#ifdef CONNECTOR_DEBUG
    Serial.println("Connected");
#endif
    if (!this -> is_sb20()) {
#ifdef CONNECTOR_DEBUG
      Serial.println("Remote is not an SB20");
#endif
      return false;
    }
#ifdef CONNECTOR_DEBUG
    Serial.println("Remote is an SB20");
#endif
    this -> device = uuid;
    this -> model -> uuid(uuid);
    return true;
  } else {
#ifdef CONNECTOR_DEBUG
    Serial.println("Unable to connect");
#endif
    this -> error_code = NoConnection;
    return false;
  }
}

SB20Connector::~SB20Connector() {
  if (client -> isConnected()) {
    this -> client -> disconnect();
  }
}

std::string SB20Connector::toString() {
  return this -> device;
}

bool SB20Connector::is_connected() {
  return client && client -> isConnected();
}

unsigned long SB20Connector::connected_since() const {
  return this -> connect_time;
}

SB20Error SB20Connector::get_error_code() {
  return this -> error_code;
}

bool SB20Connector::send_command(const Bytes &bytes) {
#ifdef CONNECTOR_DEBUG
  Serial.print("Sending command ");
  bytes.hex_dump_nl();
#endif
  command -> writeValue((uint8_t *) bytes.bytes(), bytes.size());
  this -> last_contact = millis();
  return true;
}

bool SB20Connector::add_dialog(const challenge_data *data) {
  ChallengeDialog *dialog = new ChallengeDialog(this);
  dialog -> add_challenges(data);
  dialogs.push_back(dialog);
  return true;
}

bool SB20Connector::bootstrap() {
#ifdef CONNECTOR_DEBUG
  Serial.println("Starting bootstrap sequence");
#endif
  this -> display_message("Initializing ...");
  bool ret = this -> add_dialog(bootstrap_data);
  return ret;
}

bool SB20Connector::ping() {
  this -> display_message("Ping");
  return this -> add_dialog(ping_data);
}

bool SB20Connector::push_message(uint8_t *data, int length) {
  if (length < 0 || length > 254) {
    return false;
  }
  this -> last_contact = millis();
  Bytes msg((uint8_t) length, data);
//#ifdef CONNECTOR_DEBUG
//  Serial.print("push_message: Incoming message ");
//  msg.hex_dump_nl();
//#endif
  msg_queue.push_back(msg);
  return true;
}

bool SB20Connector::send_challenge() {
  if (is_waiting) {
    return false;
  }
  while (!is_waiting) {
    if (current_dialog) {
      if (current_dialog -> exhausted()) {
        current_dialog -> onDialogComplete();
        delete current_dialog;
        current_dialog = nullptr;
        erase_message();
      } else {
        waiting_for = current_dialog -> pop();
        is_waiting = true;
      }
    }
    if (!is_waiting) {
      if (!dialogs.empty()) {
        current_dialog = dialogs.front();
        dialogs.pop_front();
        if (!current_dialog -> exhausted()) {
          current_dialog -> onDialogStart();
#ifdef CONNECTOR_DEBUG
          Serial.printf("Starting ChallengeDialog with %d challenges\n", current_dialog -> size());
#endif
        }
        continue; // continue the while (!is_waiting) loop. This will pick up the first
                  // challenge in the new dialog
      } else {
        return false;
      }
    }

#ifdef CONNECTOR_DEBUG
    this -> waiting_for.dump("Sending");
#endif
    send_command(waiting_for.challenge());
    current_dialog -> onChallengeSent(&waiting_for);
    if (waiting_for.responses() == 0) {
      is_waiting = false;
    }
  }
  return true;
}

bool SB20Connector::onResponse(Bytes &response) {
  if (is_waiting && waiting_for.match(response)) {
    current_dialog -> onResponseReceived(&waiting_for, response);
    is_waiting = false;
    return true;
  } else {
    return false;
  }
}

void SB20Connector::onSetup() {
  BLEDevice::init("Stages SB20 Monitor");

  if (!this -> model -> uuid().empty()) {
    this -> connect(this -> model -> uuid());
  }
  if (!this -> is_connected()) {
    this->display_message("Scanning");

    BLEScan *scanner = BLEDevice::getScan();
    scanner -> setAdvertisedDeviceCallbacks(this);
    scanner -> setInterval(1349);
    scanner -> setWindow(449);
    scanner -> setActiveScan(true);
    scanner -> start(5, false);
  }
}


void SB20Connector::onLoop() {
#ifdef CONNECTOR_DEBUG
  Serial.println("Connector::onLoop");
#endif
  if (this -> probe_device && !this->probe()) {
#ifdef CONNECTOR_DEBUG
    Serial.println("Restart scan");
#endif
    BLEDevice::getScan()->start(5, true);
    return;
  }
  if (!this -> is_connected()) {
#ifdef CONNECTOR_DEBUG
    Serial.println("Not Connected");
#endif
    return;
  }

  send_challenge();

  unsigned long current = millis();

  // if ((current - this -> last_shift) >= 30 * 60 * 1000) {
  //   this -> client -> disconnect();
  //   return false;
  // }

  while (!msg_queue.empty()) {
    Bytes message = msg_queue.front();
    msg_queue.pop_front();
#ifdef CONNECTOR_DEBUG
    Serial.print("onLoop: Handling message: ");
    message.hex_dump_nl();
#endif

    bool handled = false;
    for (std::vector<ResponseListener *>::iterator it = this -> listeners.begin();
         !handled && it != this -> listeners.end();
         it++) {
      handled = (*it) -> onResponse(message);
    }
#ifdef CONNECTOR_DEBUG
    Serial.print("Message ");
    if (!handled) {
      Serial.print("NOT ");
    }
    Serial.println("handled");
#endif
  }

  if ((current - this -> last_contact) >= 60000) {
#ifdef CONNECTOR_DEBUG
    Serial.printf("Ping: current: %ld Last_contact: %ld diff: %ld\n", current, last_contact, current - last_contact);
#endif
    ping();
  }
}

void SB20Connector::onModelUpdate() {
  // Send new gearing model to SB20.
}

void SB20Connector::onGearChange() {
  this -> last_shift = millis();
}

void SB20Connector::onConnect(BLEClient *ble_client) {
  this -> display_message("Connect");
}

void SB20Connector::onDisconnect(BLEClient *ble_client) {
  this -> display_message("Disconnect");
#ifdef CONNECTOR_DEBUG
  Serial.println("Disconnect");
#endif
  this -> service = nullptr;
}

bool SB20Connector::blacklisted(const std::string &found_device) {
  return (this -> blacklist.find(found_device) != this->blacklist.end());
}

void SB20Connector::onResult(BLEAdvertisedDevice advertisedDevice) {
#ifdef CONNECTOR_DEBUG
  Serial.print("BLE Advertised Device found: ");
  Serial.println(advertisedDevice.getAddress().toString().c_str());
#endif

  if (this -> blacklisted(advertisedDevice.getAddress().toString())) {
#ifdef CONNECTOR_DEBUG
    Serial.println("Blacklisted");
#endif
    return;
  }

  if (advertisedDevice.haveServiceUUID() &&
      advertisedDevice.isAdvertisingService(BLEUUID(speed_cadence_service))) {
    BLEDevice::getScan() -> stop();
    this -> probe_device = new BLEAdvertisedDevice(advertisedDevice);
  } else {
    this -> blacklist.insert(advertisedDevice.getAddress().toString());
  }
}

challenge_data SB20Connector::bootstrap_data[] = {
  { { 0x02, 0x08, 0x00 }, { { 0x02, 0x08, 0x00 }, { 0x00 } } },
  { { 0x03, 0x0c, 0x00, 0x01 }, { { 0x03, 0x0c, 0x00, 0x01 }, { 0x00 } } },
  { { 0x04, 0x0a, 0x00, 0x00, 0x00 }, { { 0x02, 0x0a, 0x01 }, { 0x00 } } },
  { { 0x02, 0x0d, 0x02 }, { { 0x02, 0x0d, 0x02 }, { 0x00 } } },
  { { 0x02, 0x0d, 0x04 }, { { 0x02, 0x0d, 0x04 }, { 0x00 } } },
  { { 0x02, 0x0e, 0x00 }, { { 0x02, 0x0e, 0x00 }, { 0x00 } } },
  { { 0x02, 0x08, 0x00 }, { { 0x02, 0x08, 0x00 }, { 0x00 } } },
  { { 0x03, 0x0c, 0x00, 0x01 }, { { 0x03, 0x0c, 0x00, 0x01 }, { 0x00 } } },
  { { 0x11, 0x0b, 0x00, 0x04, 0x04, 0x02, 0x03, 0x03, 0x01, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x00 }, { { 0x02, 0x0b, 0x00 }, { 0x02, 0x0c, 0x01 }, { 0x00 } } },
  { { 0x03, 0x10, 0x00, 0x01 }, { { 0x03, 0x10, 0x00, 0x01 }, { 0x02, 0x0c, 0x01 }, { 0x00 } } },
  { { 0x08, 0x0c, 0x00, 0x02, 0x05, 0x01, 0xc8, 0x00, 0x01 }, { { 0x02, 0x05, 0x01 }, { 0x02, 0x0c, 0x01 }, { 0x00 } } },
  { { 0x03, 0x0c, 0x00, 0x02 }, { { 0x02, 0x0c, 0x01 }, { 0x00 } } },
  { { 0x0b, 0x0c, 0x02, 0x00, 0x00, 0x02, 0x0c, 0x10, 0x00, 0x03, 0x0e, 0x00 }, { { 0x02, 0x0c, 0x02 }, { 0x02, 0x0c, 0x01 }, { 0x00 } } },
  { { 0x03, 0x0c, 0x00, 0x02 }, { { 0x02, 0x0c, 0x01 }, { 0x00 } } },
  { { 0x10, 0x0c, 0x03, 0x22, 0x32, 0x21, 0x1c, 0x18, 0x15, 0x13, 0x11, 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a }, { { 0x00 } } },
  { { 0x0b, 0x0c, 0x02, 0x0b, 0x00, 0x02, 0x0c, 0x10, 0x00, 0x03, 0x0e, 0x00 }, { { 0x02, 0x0c, 0x02 }, { 0x02, 0x0c, 0x01 }, { 0x00 } } },
  { { 0x02, 0xfd, 0x00 }, { { 0x02, 0xfd, 0x01 }, { 0x02, 0x0c, 0x01 }, { 0x00 } } },
  { { 0x06, 0x03, 0x01, 0x4c, 0x1d, 0x00, 0x00 }, { { 0x02, 0x03, 0x01 }, { 0x02, 0x0c, 0x01 }, { 0x00 } } },
  { { 0x03, 0x0c, 0x00, 0x02 }, { { 0x02, 0x0c, 0x01 }, { 0x00 } } },
  { { 0x03, 0x0c, 0x00, 0x02 }, { { 0x02, 0x0c, 0x01 }, { 0x00 } } },
  { { 0x03, 0x0c, 0x00, 0x02 }, { { 0x02, 0x0c, 0x01 }, { 0x00 } } },
  { { 0x00 }, { { 0x00 } } }
};

challenge_data SB20Connector::ping_data[] = {
  { { 0x02, 0x0d, 0x04 }, { { 0x02, 0x0d, 0x04 }, { 0x00 } } },
  { { 0x00 }, { { 0x00 } } }
};
