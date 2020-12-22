
#include "Arduino.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include <Preferences.h>

#include "heltec.h"

static BLEUUID speed_cadence_service("1816");
static BLEUUID sb20_service("0c46beaf-9c22-48ff-ae0e-c6eae1a2f4e5");
static BLEUUID sb20_status("0c46beb0-9c22-48ff-ae0e-c6eae1a2f4e5");
static BLEUUID sb20_command("0c46beb1-9c22-48ff-ae0e-c6eae1a2f4e5");

/* From https://www.uuidgenerator.net/ */
static BLEUUID config_service("5fc1ff12-e9dc-4aa3-95d9-4dc96949bdc5");
static BLEUUID config_chainrings("3a6ced0d-f13c-4578-9e45-086a69ecdf22");
static BLEUUID config_cogs("3cbcbe91-0212-498e-acb2-ff69c5ff2cfc");

static void notifyCallback(BLERemoteCharacteristic *, uint8_t *, size_t, bool);

class SB20Model;
class SB20View;
class SB20;

static SB20Model *model = nullptr;
static SB20View  *view = nullptr;
static SB20      *sb20 = nullptr;

void hex_dump_raw(uint8_t *data, unsigned int length) {
  if (data == nullptr) {
    Serial.print(" (null)");
  } else if (length == 0) {
    Serial.print(" (empty)");
  } else {
    for (int ix = 0; ix < length; ix++) {
      Serial.printf(" %02x", data[ix]);
    }
  }
}

void hex_dump(uint8_t *data) {
  if (data == nullptr) {
    hex_dump_raw(data, 0);
  } else if (data[0] == 0) {
    hex_dump_raw(data, 0);
  } else {
    hex_dump_raw(data + 1, data[0]);
  }
}


class RingBuffer {

private:
  void         **buffer;
  unsigned int   push_ptr;
  unsigned int   pop_ptr;
  unsigned int   size;

public:
  
  RingBuffer(unsigned int size) {
    this -> size = size;
    this -> push_ptr = 0;
    this -> pop_ptr = 0;
    this -> buffer = new void*[size];
    for (int ix = 0; ix < this -> size; ix++) {
      this -> buffer[ix] = nullptr;
    }
  }

  ~RingBuffer() {
    for (int ix = 0; ix < this -> size; ix++) {
      if (this -> buffer[ix] != nullptr) {
        delete this -> buffer[ix];
      }
    }
    delete this -> buffer;
  }

  void push(void *data) {
    this -> buffer[this -> push_ptr] = data;
    this -> push_ptr = (this -> push_ptr < (this -> size - 1)) ? this -> push_ptr + 1 : 0;
  }

  void * pop() {
    if (this -> empty()) {
      return nullptr;
    }
    void *data = this -> buffer[this -> pop_ptr];
    this -> buffer[this -> pop_ptr] = nullptr;
    this -> pop_ptr = (this -> pop_ptr < (this -> size - 1)) ? this -> pop_ptr + 1 : 0;
    return data;
  }

  bool empty() {
    return this -> push_ptr == this -> pop_ptr;
  }

};


typedef struct _challenge_data {
  uint8_t  challenge[32];
  uint8_t  responses[3][5];
} challenge_data;


class Challenge {
private:
  uint8_t        *_challenge;
  unsigned int    _num;
  uint8_t       **_responses;

public:
  Challenge(challenge_data *data) {
    this -> _challenge = new uint8_t[data -> challenge[0] + 1];
    memcpy(this -> _challenge, data -> challenge, data -> challenge[0] + 1);
    
    for (this -> _num = 0; data -> responses[this -> _num][0]; this -> _num++);
    if (this - _num > 0) {
      this -> _responses = new uint8_t*[this -> _num];
      for (int ix = 0; ix < this -> _num; ix++) {
        this -> _responses[ix] = new uint8_t[data -> responses[ix][0] + 1];
        memcpy(this -> _responses[ix], 
          data -> responses[ix], 
          data -> responses[ix][0] + 1);
      }
    } else {
      this -> _responses = nullptr;
    }
    // this -> dump("Created ");
  }

  void dump(String prefix) {
    Serial.print(prefix);
    Serial.print(" Challenge ");
    hex_dump(this -> _challenge);
    if (this -> _num > 0) {
      Serial.print(", Response(s): ");
      for (int ix = 0; ix < this -> _num; ix++) {
        if (ix > 0) {
          Serial.print(", ");
        }
        hex_dump(this -> _responses[ix]);
      }
    }
    Serial.println();
  }

  ~Challenge() {
    int ix;

    // this -> dump("Destroying");
    delete this -> _challenge;
    for (int ix = 0; ix < this -> _num; ix++) {
      delete this -> _responses[ix];
    }
    delete this -> _responses;
  }

  uint8_t * challenge() {
    return this -> _challenge;
  }

  uint8_t * response(unsigned int ix) {
    return (ix < this -> _num) ? this -> _responses[ix] : nullptr;
  }

  unsigned int responses() {
    return this -> _num;
  }

  bool match(uint8_t *received) {
    // this -> dump("Matching");
    // Serial.print("Against ");
    // hex_dump(received);
    // Serial.println();
    for (int ix = 0; ix < this -> _num; ix++) {
      if (!memcmp(received + 1, this -> _responses[ix] + 1, min(this -> _responses[ix][0], received[0]))) {
        return true;
      }
    }
    return false;
  }
};

uint8_t GEAR_CHANGE_PREFIX[4] = {3, 0x0c, 0x01, 0x00};


class ModelListener {
public:
  virtual void onModelUpdate(SB20Model *model) = 0;
  virtual void onGearChange(SB20Model *model) = 0;
  virtual void onDisplayMessage(const char *msg) = 0;
};

class SB20Model : public BLECharacteristicCallbacks {
private:
  uint8_t           *chainrings;
  uint8_t           *cogs;
  uint8_t            cur_chainring;
  uint8_t            cur_cog;
  ModelListener     *listener;

  BLECharacteristic *chainrings_char;
  BLECharacteristic *cogs_char;

public:
  SB20Model(ModelListener *listener) : listener(listener) { 
    this -> read_config();
    this -> cur_chainring = 0;
    this -> cur_cog = 0;
    // this -> start_ble_service();
  }

  ~SB20Model() {
    delete this -> chainrings;
    delete this -> cogs;
  }

  void set_listener(ModelListener *listener) {
    this -> listener = listener;
  }

  void start_ble_service() {
    BLEServer *server = BLEDevice::createServer();
    BLEService *service = server -> createService(config_service);
    Serial.println("Server side BLE service created");

    this -> chainrings_char = service -> createCharacteristic(
      config_chainrings,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    this -> chainrings_char -> setValue(this -> chainrings, this -> chainrings[0] + 1);
    this -> chainrings_char -> setCallbacks(this);
    Serial.println("Chainrings characteristic created");

    this -> cogs_char = service -> createCharacteristic(
      config_cogs,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    this -> cogs_char -> setValue(this -> cogs, this -> cogs[0] + 1);
    this -> cogs_char -> setCallbacks(this);
    Serial.println("Cogs characteristic created");

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

	void onRead(BLECharacteristic* characteristic) {
    Serial.print("Read of char ");
    Serial.println(characteristic -> toString().c_str());
  }

	void onWrite(BLECharacteristic* characteristic) {
    Serial.print("Write of char ");
    if (characteristic == this -> chainrings_char) {
      Serial.println("Chainrings");
    } else if (characteristic == this -> cogs_char) {
      Serial.println("Cogs");
    } else {
      Serial.println(characteristic -> toString().c_str());
    }
    Serial.print("Value is now ");

    size_t sz = characteristic -> getDataLength();
    uint8_t *chainrings = characteristic -> getData();
    hex_dump_raw(chainrings, sz);

    Serial.println();
  }

	void onNotify(BLECharacteristic* characteristic) {
    Serial.print("Notify of char ");
    Serial.println(characteristic -> toString().c_str());
  }

	virtual void onStatus(BLECharacteristic* characteristic, Status s, uint32_t code) {
    Serial.print("Status of char ");
    Serial.print(characteristic -> toString().c_str());
    Serial.print(": ");
    switch (s) {
      case SUCCESS_INDICATE:
        Serial.print("SUCCESS_INDICATE");
        break;
	    case SUCCESS_NOTIFY:
        Serial.print("SUCCESS_NOTIFY");
        break;
	    case ERROR_INDICATE_DISABLED:
        Serial.print("ERROR_INDICATE_DISABLED");
        break;
	    case ERROR_NOTIFY_DISABLED:
        Serial.print("ERROR_NOTIFY_DISABLED");
        break;
	    case ERROR_GATT:
        Serial.print("ERROR_GATT");
        break;
	    case ERROR_NO_CLIENT:
        Serial.print("ERROR_NO_CLIENT");
        break;
	    case ERROR_INDICATE_TIMEOUT:
        Serial.print("ERROR_INDICATE_TIMEOUT");
        break;
	    case ERROR_INDICATE_FAILURE:
        Serial.print("ERROR_INDICATE_FAILURE");
        break;
    }
    Serial.print(", code: ");
    Serial.println(code);
  }

  void read_config() {
    Preferences prefs;

    prefs.begin("stages-sb20", false);
    if (prefs.getBytesLength("num-chainrings") && prefs.getBytesLength("chainrings")) {
      uint8_t num_chainrings = prefs.getUChar("num-chainrings", 2);
      this -> chainrings = new uint8_t[num_chainrings + 1];
      this -> chainrings[0] = num_chainrings;
      prefs.getBytes("chainrings", &this -> chainrings[1], num_chainrings);
    } else {
      this -> chainrings = new uint8_t[3]{ 2, 34, 50 };
    }

    if (prefs.getBytesLength("num-cogs") && prefs.getBytesLength("cogs")) {
      uint8_t num_cogs = prefs.getUChar("num-cogs", 12);
      this -> cogs = new uint8_t[num_cogs + 1];
      this -> cogs[0] = num_cogs;
      prefs.getBytes("cogs", &this -> cogs[1], num_cogs);
    } else {
      this -> cogs = new uint8_t[13]{ 12, 33, 28, 24, 21, 19, 17, 15, 14, 13, 12, 11, 10 };
    }
    prefs.end();
  }

  uint8_t num_chainrings() {
    return this -> chainrings[0];
  }

  uint8_t num_cogs() {
    return this -> cogs[0];
  }

  uint8_t chainring(int ix) {
    return ((ix >= 1) && (ix <= num_chainrings())) ? this -> chainrings[ix] : 0;
  }

  uint8_t cog(int ix) {
    return ((ix >= 1) && (ix <= num_cogs())) ? this -> cogs[ix] : 0;
  }

  uint8_t current_chainring() {
    return this -> cur_chainring;
  }

  uint8_t current_cog() {
    return this -> cur_cog;
  }

  void gear_change(uint8_t new_chainring, uint8_t new_cog) {
    if ((new_chainring != cur_chainring) || (new_cog != cur_cog)) {
      this -> cur_chainring = new_chainring;
      this -> cur_cog = new_cog;
      if (this -> listener) {
        this -> listener -> onGearChange(this);
      }
    }
  }

  void display_message(const char *msg) {
    if (this -> listener) {
      this -> listener -> onDisplayMessage(msg);
    }
  }

};


class SB20View {
public:
  virtual void display() = 0;

  virtual void refresh() = 0;

  virtual void onGearChange(SB20Model *) = 0;

  virtual void display_message(const char *msg) = 0;

  virtual void onDisplayMessage(const char *msg) {
    Serial.println(msg);
    this -> display_message(msg);
  }

  virtual void erase_message() = 0;
};


class SB20HeltecView : public SB20View {
private:
  SB20Model  *model;
  uint8_t     width;
  uint8_t     height;
  const char *message = nullptr;
  const char *displayed = nullptr;
  bool        redisplay;

public:

  SB20HeltecView(SB20Model *model) : model(model), message(nullptr), redisplay(false) {
    Heltec.display -> flipScreenVertically();
    Heltec.display -> setContrast(255);
    this -> height = Heltec.display -> getHeight();
    this -> width = Heltec.display -> getWidth();
  }

  void onGearChange(SB20Model *model) {
    this -> redisplay = true;
    // this -> display();
  }

  void display_message(const char *msg) {
    this -> message = msg;
    // this -> display();
  }

  void erase_message() {
    this -> message = nullptr;
  }

  void refresh() {
    this -> redisplay = true;
  }

  void display() {
    if ((this -> message != this -> displayed) || (this -> redisplay)) {
      Heltec.display -> clear();
      Heltec.display -> setColor(WHITE);

      if ((this -> message != nullptr) && strcmp(this -> message, "")) {
        Serial.println(this -> message);
        Heltec.display -> clear();
        Heltec.display -> setFont(ArialMT_Plain_10);
        Heltec.display -> setTextAlignment(TEXT_ALIGN_LEFT);
        Heltec.display -> drawString(this -> width / 2, 12, this -> message);
      }
      this -> displayed = this -> message;

      uint8_t x_inc = (uint8_t) (width  / (model -> num_chainrings() + model -> num_cogs() + 2));
      uint8_t w = (uint8_t) (2 * x_inc / 3);
      uint8_t x = (uint8_t) (x_inc / 2);
      uint8_t h_max = this -> height - 12;
      uint8_t y_0 = 0;
      float factor = h_max / (model -> chainring(model -> num_chainrings() / 2));
      for (int ix = 1; ix <= model -> num_chainrings(); ix++) {
        uint8_t chainring = model -> chainring(ix);
        float h = chainring * factor;
        uint8_t y = (uint8_t) (height - h); //(y_0 + (h_max - h)/2);
        if (ix == model -> current_chainring()) {
          Heltec.display -> fillRect(x, y, w, h);
        } else {
          Heltec.display -> drawRect(x, y, w, h);
        }
        x += x_inc;
      }

      x += x_inc;
      factor = h_max / model -> cog(1);
      for (int ix = 1; ix <= model -> num_cogs(); ix++) {
        uint8_t cog = model -> cog(ix);
        float h = cog * factor;
        uint8_t y = (uint8_t) (height - h); // (y_0 + (h_max - h)/2);
        if (ix == model -> current_cog()) {
          Heltec.display -> fillRect(x, y, w, h);
        } else {
          Heltec.display -> drawRect(x, y, w, h);
        }
        x += x_inc;
      }
      Heltec.display -> display();
      this -> redisplay = false;
    }
  }

};

challenge_data _ping = { { 0x02, 0x0d, 0x04 }, { { 0x02, 0x0d, 0x04 }, { 0x00 } } };

challenge_data _bootstrap_data[22] = {
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

enum SB20Error {
  NoError,
  NoSB20Service,
  NoCommandChar,
  NoStatusChar
};

class SB20 : public BLEClientCallbacks, public ModelListener {
private:
  RingBuffer               challenges;
  RingBuffer               status_queue;
  Challenge               *_waiting_for;
  SB20Model               *model;
  SB20View                *view;
  BLEClient               *client;
  BLEAdvertisedDevice     *device;
  BLERemoteCharacteristic *command;
  BLERemoteCharacteristic *status;
  bool                     bootstrapped;
  SB20Error                error_code;
  unsigned long            connect_time;
  unsigned long            last_contact;
  unsigned long            last_shift;

public:
  SB20(BLEAdvertisedDevice *device, SB20Model *model, SB20View *view) : 
      challenges(64), 
      status_queue(16),
      device(device),
      model(model),
      client(nullptr),
      error_code(NoError),
      _waiting_for(nullptr),
      bootstrapped(false),
      view(view) {
    model -> set_listener(this);
  }

  bool connect() {
    Serial.print("Connecting to ");
    Serial.println(this -> device -> getAddress().toString().c_str());

    this -> client = BLEDevice::createClient();
    Serial.println(" - Created client");
    this -> client -> setClientCallbacks(this);

    // Connect to the remove BLE Server.
    client -> connect(device);
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* service = client -> getService(sb20_service);
    if (service == nullptr) {
      this -> error_code = NoSB20Service;
      this -> client -> disconnect();
      return false;
    }
    Serial.println(" - Found our service");

    command = service -> getCharacteristic(sb20_command);
    if (command == nullptr) {
      this -> error_code = NoCommandChar;
      this -> client -> disconnect();
      return false;
    }
    Serial.println(" - Found command characteristic");

    status = service->getCharacteristic(sb20_status);
    if (status == nullptr) {
      this -> error_code = NoStatusChar;
      this -> client -> disconnect();
      return false;
    }
    Serial.println(" - Found status characteristic");

    if (status -> canNotify()) {
      status -> registerForNotify(notifyCallback);
    }
    this -> connect_time = this -> last_contact = this -> last_shift = millis();

    return true;
  }

  ~SB20() {
    if (this -> client != nullptr) {
      this -> client -> disconnect();
      this -> client = nullptr;
    }
    if (this -> _waiting_for != nullptr) {
      delete this -> _waiting_for;
      this -> _waiting_for = nullptr;
    }
  }

  std::string toString() {
    return this -> device -> toString();
  }

  bool is_connected() {
    return this -> client != nullptr;
  }

  unsigned long connected_since() {
    return this -> connect_time;
  }

  SB20Error get_error_code() {
    return this -> error_code;
  }

  bool send_command(uint8_t *cmd) {
    command -> writeValue(cmd + 1, cmd[0]);
    this -> last_contact = millis();
  }

  bool add_challenge(challenge_data *challenge) {
    Challenge *c = new Challenge(challenge);
    // c -> dump("Adding challenge");
    this -> challenges.push(c);
    return true;
  }

  bool bootstrap() {
    if (!this -> bootstrapped) {
      this -> display_message("Initializing ...");
      for (int ix = 0; _bootstrap_data[ix].challenge[0]; ix++) {
        add_challenge(&_bootstrap_data[ix]);
      }
      while (!this -> challenges.empty() || this -> _waiting_for != nullptr) {
        this -> handle_incoming();
      }
      this -> bootstrapped = true;
      this -> view -> erase_message();
      Serial.println("Bootstrapped");
    }
    this -> view -> refresh();
  }

  bool send_challenge() {
    if (this -> _waiting_for != nullptr) {
      return false;
    }
    this -> _waiting_for = (Challenge *) this -> challenges.pop();
    if (this -> _waiting_for != nullptr) {
      // this -> _waiting_for -> dump("Sending");
      this -> send_command(this -> _waiting_for -> challenge());
      if (this -> _waiting_for -> responses() == 0) {
        delete this -> _waiting_for;
        this -> _waiting_for = nullptr;
      }
    }
    return true;
  }

  bool push_status(uint8_t *data, int length) {
    if (length < 0 || length > 254) {
      return false;
    }
    this -> last_contact = millis();
    // if (this -> status_queue[this -> queue_ptr] != nullptr) {
    //   return false;
    // }
    uint8_t *status = new uint8_t[length + 1];
    if (status == nullptr) {
      return false;
    }
    status[0] = (uint8_t) length;
    memcpy(status + 1, data, length);
    // Serial.print("Incoming status ");
    // hex_dump(status);
    // Serial.println();
    this -> status_queue.push(status);
    return true;
  }

  bool handle_incoming() {
    uint8_t       *status;
    unsigned long  current = millis();

    // if ((current - this -> last_shift) >= 30 * 60 * 1000) {
    //   this -> client -> disconnect();
    //   return false;
    // }

    if ((current - this -> last_contact) >= 60000) {
      this -> display_message("Ping!");
      this -> add_challenge(&_ping);
    }

    while ((status = (uint8_t *) this -> status_queue.pop()) != nullptr) {
      if (this -> _waiting_for != nullptr) {
        if (this -> _waiting_for -> match(status)) {
          delete this -> _waiting_for;
          this -> _waiting_for = nullptr;
        }
      } else if ((status[0] > 5) && !memcmp(status + 1, GEAR_CHANGE_PREFIX + 1, GEAR_CHANGE_PREFIX[0])) {
        this -> last_shift = current;
        this -> model -> gear_change(status[4], status[5]);
      }
      if (this -> challenges.empty() && (this -> _waiting_for == nullptr)) {
        this -> view -> erase_message();
      }
    }
    this -> send_challenge();
    return true;
  }

  void onModelUpdate(SB20Model *model) {
    //
  }

  void onGearChange(SB20Model *model) {
    if (this -> view) {
      this -> view -> onGearChange(model);
    }
  }
  
  void onDisplayMessage(const char *msg) {
    this -> display_message(msg);
  }

  void display_message(const char *msg) {
    if (this -> view) {
      this -> view -> onDisplayMessage(msg);
    }
  }

  void onConnect(BLEClient *client) {
    this -> display_message("Connect");
  }

  void onDisconnect(BLEClient *client) {
//    this -> client = nullptr;
//    delete this -> client;
//    this -> display_message("Disconnect");
    Serial.println("Disconnect");
  }
};

static void notifyCallback(BLERemoteCharacteristic *status, uint8_t *data, size_t length, bool isNotify) {
  if (sb20 != nullptr) {
    sb20 -> push_status(data, length);
  }

}


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


void start_server() {
  model = new SB20Model(nullptr);
  view = new SB20HeltecView(model);
}


void setup() {
  Heltec.begin(true, false, true);
  Serial.begin(115200);
  Serial.println("Starting SB20 Monitor");

  BLEDevice::init("Stages SB20 Monitor");

  start_server();

  view -> display_message("Scanning");

  BLEScan *scanner = BLEDevice::getScan();
  scanner -> setAdvertisedDeviceCallbacks(new FindSB20Callback());
  scanner -> setInterval(1349);
  scanner -> setWindow(449);
  scanner -> setActiveScan(true);
  scanner -> start(5, false);
}


void loop() {
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
  delay(100);
}
