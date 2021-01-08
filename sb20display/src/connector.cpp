#include "connector.h"

SB20Connector * SB20Connector::singleton = nullptr;

static void notifyCallback(BLERemoteCharacteristic *characteristic, uint8_t *data, size_t length, bool isNotify) {
  if (SB20Connector::instance() != nullptr) {
    SB20Connector::instance() -> push_message(data, length);
  }
}

SB20Connector::SB20Connector(SB20Model *model) : 
    msg_queue(),
    model(model),
    client(nullptr),
    service(nullptr),
    probe_device(nullptr),
    blacklist(),
    device(),
    command(nullptr),
    status(nullptr),
    dialog(nullptr),
    listener(nullptr),
    error_code(NoError),
    connect_time(0L),
    last_contact(0L),
    last_shift(0L) {
  model -> add_listener(this);
  this -> add_listener(model);
}

bool SB20Connector::probe() {
  if (!this -> probe_device) {
    return false;
  }
  this -> display_message("Connecting ...");
  Serial.print("Probing ");
  Serial.println(this -> probe_device -> getAddress().toString().c_str());

  if (!this -> client) {
    this -> client = BLEDevice::createClient();
    Serial.println(" - Created client");
    this -> client -> setClientCallbacks((BLEClientCallbacks *) this);
  }

  bool ret = false;
  if (client -> connect(this -> probe_device)) {
    Serial.println(" - Connected to server");
    if (!this -> is_sb20()) {
      Serial.println(" - Found our service");
      ret = true;
    } else {
      Serial.println("Not an SB20");
    }
  } else {
    Serial.println(" - Unable to connect");
  }
  if (!ret) {
    if (this -> client -> isConnected()) {
      this->client->disconnect();
    }
    this -> blacklist.insert(this -> probe_device -> toString());
  } else {
    this -> device = this -> probe_device -> toString();
    this -> model -> uuid(this -> device);
  }
  this -> probe_device = nullptr;
  return ret;
}

bool SB20Connector::is_sb20() {
  if (client && client -> isConnected()) {
    if (!service) {
      this -> service = client->getService(sb20_service);
      if (service) {
        command = service -> getCharacteristic(sb20_command);
        if (command == nullptr) {
          this -> error_code = NoCommandChar;
          this -> client -> disconnect();
          this -> service = nullptr;
          return false;
        }
        Serial.println(" - Found command characteristic");

        status = service -> getCharacteristic(sb20_status);
        if (status == nullptr) {
          this -> error_code = NoStatusChar;
          this -> client -> disconnect();
          this -> service = nullptr;
          return false;
        }
        Serial.println(" - Found status characteristic");

        if (status -> canNotify()) {
          status -> registerForNotify(notifyCallback);
        }
        this -> connect_time = this -> last_contact = this -> last_shift = millis();
        this -> bootstrap();
      } else {
        error_code = NoSB20Service;
      }
    }
    return this -> service != nullptr;
  } else {
    return false;
  }
}

bool SB20Connector::connect(const std::string &uuid) {
  if (!this -> client) {
    this -> client = BLEDevice::createClient();
    Serial.println(" - Created client");
    this -> client -> setClientCallbacks(this);
  }

  if (client -> connect(BLEAddress(uuid))) {
    Serial.println(" - Connected to server");
    if (!this -> is_sb20()) {
      Serial.println("Not an SB20");
      return false;
    }
    Serial.println(" - Found SB20 service");
    this -> device = uuid;
    this -> model -> uuid(uuid);
  } else {
    Serial.println(" - Unable to connect");
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
  command -> writeValue(bytes.size(), (const uint8_t *) bytes);
  this -> last_contact = millis();
  return true;
}

bool SB20Connector::start_dialog(ChallengeDialogListener *dialog_listener, const challenge_data *data) {
  if (!dialog) {
    if (!dialog_listener) {
      dialog_listener = new SB20ConnectorListener(this);
    }
    this -> listener = dialog_listener;
    this -> dialog = new ChallengeDialog((ChallengeDialogConnector *) this, this -> listener);
    this -> dialog -> add_challenges(data);
    this -> dialog -> start();
    return true;
  } else {
    return false;
  }
}

void SB20Connector::dialog_complete() {
  delete this -> listener;
  this -> listener = nullptr;
  delete this -> dialog;
  this -> dialog = nullptr;
}

bool SB20Connector::bootstrap() {
  this -> display_message("Initializing ...");
  return this -> start_dialog(nullptr, bootstrap_data);
}

bool SB20Connector::ping() {
  this -> display_message("Ping");
  return this -> start_dialog(nullptr, ping_data);
}

bool SB20Connector::send_challenge(const Bytes &challenge) {
  return send_command(challenge);
}

bool SB20Connector::push_message(uint8_t *data, int length) {
  if (length < 0 || length > 254) {
    return false;
  }
  this -> last_contact = millis();
  Bytes *msg = new Bytes((uint8_t) length, data);
#ifdef CONNECTOR_DEBUG
   Serial.print("Incoming status ");
   msg.hex_dump();
   Serial.println();
#endif
  msg_queue.push_back(*msg);
  return true;
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
  if (this -> probe_device && !this->probe()) {
    BLEDevice::getScan()->start(0);
    return;
  }
  if (!this -> is_connected()) {
    return;
  }

  unsigned long  current = millis();

  // if ((current - this -> last_shift) >= 30 * 60 * 1000) {
  //   this -> client -> disconnect();
  //   return false;
  // }

  for (Bytes *message = &msg_queue.front(); message != nullptr; message = &msg_queue.front()) {
    msg_queue.pop_front();
    for (std::vector<Listener *>::iterator it = this -> listeners.begin(); it != this -> listeners.end(); it++) {
      if (((ResponseListener *)(*it)) -> onResponse(*message)) {
        break;
      }
    }
  }

  if ((current - this -> last_contact) >= 60000) {
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
  Serial.println("Disconnect");
  this -> service = nullptr;
}

bool SB20Connector::blacklisted(const std::string &found_device) {
  return (this -> blacklist.find(found_device) != this->blacklist.end());
}

void SB20Connector::onResult(BLEAdvertisedDevice advertisedDevice) {
  Serial.print("BLE Advertised Device found: ");
  Serial.println(advertisedDevice.toString().c_str());

  if (this -> blacklisted(advertisedDevice.toString())) {
    Serial.println("Blacklisted");
    return;
  }

  if (advertisedDevice.haveServiceUUID() &&
      advertisedDevice.isAdvertisingService(BLEUUID(speed_cadence_service))) {
    BLEDevice::getScan() -> stop();
    this -> probe_device = new BLEAdvertisedDevice(advertisedDevice);
  } else {
    this -> blacklist.insert(advertisedDevice.toString());
  }
}

