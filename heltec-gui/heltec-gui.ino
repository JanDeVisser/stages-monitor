#include "Arduino.h"
#include "BLEDevice.h"
#include <Preferences.h>

#include "heltec.h"

static BLEUUID speed_cadence_service("1816");
static BLEUUID sb20_service("0c46beaf-9c22-48ff-ae0e-c6eae1a2f4e5");
static BLEUUID sb20_status("0c46beb0-9c22-48ff-ae0e-c6eae1a2f4e5");
static BLEUUID sb20_command("0c46beb1-9c22-48ff-ae0e-c6eae1a2f4e5");

static void notifyCallback(BLERemoteCharacteristic *, uint8_t *, size_t, bool);

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

void display_message(const char *msg) {
  Serial.println(msg);
  Heltec.display -> clear();
  Heltec.display -> setTextAlignment(TEXT_ALIGN_CENTER);
  Heltec.display -> setFont(ArialMT_Plain_16);
  Heltec.display -> drawString(Heltec.display -> getWidth() / 2, (Heltec.display -> getHeight()) / 2 - 8, msg);
  Heltec.display -> display();    
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
    this -> dump("Matching");
    Serial.print("Against ");
    hex_dump(received);
    Serial.println();
    for (int ix = 0; ix < this -> _num; ix++) {
      if (!memcmp(received + 1, this -> _responses[ix] + 1, min(this -> _responses[ix][0], received[0]))) {
        return true;
      }
    }
    return false;
  }
};

uint8_t GEAR_CHANGE_PREFIX[4] = {3, 0x0c, 0x01, 0x00};


class SB20Model {
  uint8_t  num_chainrings;
  uint8_t  num_cogs;
  uint8_t *chainrings;
  uint8_t *cogs;
  uint8_t  cur_chainring;
  uint8_t  cur_cog;
  uint8_t  width;
  uint8_t  height;
  bool     redisplay;
public:
  SB20Model() { 
    this -> num_chainrings = 2;
    this -> num_cogs = 12;
    this -> chainrings = new uint8_t[2]{ 34, 50 };
    this -> cogs = new uint8_t[12]{ 33, 28, 24, 21, 19, 17, 15, 14, 13, 12, 11, 10 };
    this -> cur_chainring = 0;
    this -> cur_cog = 0;
    this -> height = Heltec.display -> getHeight();
    this -> width = Heltec.display -> getWidth();
    this -> redisplay = false;
  }

  ~SB20Model() {
    delete this -> chainrings;
    delete this -> cogs;
  }

  void read_config() {
    Preferences prefs;

    prefs.begin("stages-sb20", false);
    if (prefs.getBytesLength("num-chainrings") && prefs.getBytesLength("chainrings")) {
      this -> num_chainrings = prefs.getUChar("num-chainrings", 2);
      this -> chainrings = new uint8_t[num_chainrings];
      prefs.getBytes("chainrings", this -> chainrings, this -> num_chainrings);
    } else {
      this -> num_chainrings = 2;
      this -> chainrings = new uint8_t[2]{ 34, 50 };
    }
    if (prefs.getBytesLength("num-cogs") && prefs.getBytesLength("cogs")) {
      this -> num_cogs = prefs.getUChar("num-cogs", 12);
      this -> cogs = new uint8_t[num_cogs];
      prefs.getBytes("chainrings", this -> cogs, this -> num_chainrings);
    } else {
      this -> num_cogs = 12;
      this -> cogs = new uint8_t[12]{ 33, 28, 24, 21, 19, 17, 15, 14, 13, 12, 11, 10 };
    }
    prefs.end();
  }

  void status_message(uint8_t *status) {
    if ((status[0] > 5) && !memcmp(status + 1, GEAR_CHANGE_PREFIX + 1, GEAR_CHANGE_PREFIX[0])) {
      this -> cur_chainring = status[4] - 1;
      this -> cur_cog = status[5] - 1;
      this -> redisplay = true;
    }
  }

  void display() {
    if (this -> redisplay) {
      Heltec.display -> clear();
      Heltec.display -> setColor(WHITE);

      uint8_t x_inc = (uint8_t) (width  / (num_chainrings + num_cogs + 2));
      uint8_t w = (uint8_t) (2 * x_inc / 3);
      uint8_t x = (uint8_t) (x_inc / 2);
      uint8_t h_max = this -> height - 1;
      uint8_t y_0 = 0;
      float factor = h_max / chainrings[num_chainrings - 1];
      for (int ix = 0; ix < num_chainrings; ix++) {
        uint8_t chainring = chainrings[ix];
        float h = chainring * factor;
        uint8_t y = (uint8_t) (y_0 + (h_max - h)/2);
        if (ix == cur_chainring) {
          Heltec.display -> fillRect(x, y, w, h);
        } else {
          Heltec.display -> drawRect(x, y, w, h);
        }
        x += x_inc;
      }

      x += x_inc;
      factor = h_max / cogs[0];
      for (int ix = 0; ix < num_cogs; ix++) {
        uint8_t cog = cogs[ix];
        float h = cog * factor;
        uint8_t y = (uint8_t) (y_0 + (h_max - h)/2);
        if (ix == cur_cog) {
          Heltec.display -> fillRect(x, y, w, h);
        } else {
          Heltec.display -> drawRect(x, y, w, h);
        }
        x += x_inc;
      }
      Heltec.display->display();
      this -> redisplay = false;
    }
  }

  void status_message(char *msg) {
    display_message(msg);
  }

};


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

class SB20 : public BLEClientCallbacks {
private:
  RingBuffer               challenges;
  RingBuffer               status_queue;
  Challenge               *_waiting_for;
  SB20Model                model;
  BLEClient               *client;
  BLEAdvertisedDevice     *device;
  BLERemoteCharacteristic *command;
  BLERemoteCharacteristic *status;
  SB20Error                error_code;
  unsigned long            connect_time;

public:
  SB20(BLEAdvertisedDevice *device) : challenges(64), status_queue(16), model() {
    this -> _waiting_for = nullptr;
    this -> device = device;
    this -> client = nullptr;
    this -> error_code = NoError;
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
      delete client;
      this -> client = nullptr;
      return false;
    }
    Serial.println(" - Found our service");

    command = service -> getCharacteristic(sb20_command);
    if (command == nullptr) {
      this -> error_code = NoCommandChar;
      this -> client -> disconnect();
      delete client;
      this -> client = nullptr;
      return false;
    }
    Serial.println(" - Found command characteristic");

    status = service->getCharacteristic(sb20_status);
    if (status == nullptr) {
      this -> error_code = NoStatusChar;
      this -> client -> disconnect();
      delete client;
      this -> client = nullptr;
      return false;
    }
    Serial.println(" - Found status characteristic");

    if (status -> canNotify()) {
      status -> registerForNotify(notifyCallback); // FIXME error handling
    }
    this -> connect_time = millis();

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

  void status_message(char *msg) {
    this -> model.status_message(msg);
  }

  bool send_command(uint8_t *cmd) {
    command -> writeValue(cmd + 1, cmd[0]);
  }

  bool add_challenge(challenge_data *challenge) {
    Challenge *c = new Challenge(challenge);
    c -> dump("Adding challenge");
    this -> challenges.push(c);
    return true;
  }

  bool bootstrap() {
    this -> status_message("Initializing ...");
    for (int ix = 0; _bootstrap_data[ix].challenge[0]; ix++) {
      add_challenge(&_bootstrap_data[ix]);
    }
    while (!this -> challenges.empty() || this -> _waiting_for != nullptr) {
      this -> handle_incoming();
    }
  }

  bool send_challenge() {
    if (this -> _waiting_for != nullptr) {
      return false;
    }
    this -> _waiting_for = (Challenge *) this -> challenges.pop();
    if (this -> _waiting_for != nullptr) {
      this -> _waiting_for -> dump("Sending");
      this -> send_command(this -> _waiting_for -> challenge());
      if (this -> _waiting_for -> responses() == 0) {
        delete this -> _waiting_for;
        this -> _waiting_for = nullptr;
      }
    }
    return true;
  }

  bool push_status(uint8_t *data, int length) {
    if (length < 0 || length > 255) {
      return false;
    }
    // if (this -> status_queue[this -> queue_ptr] != nullptr) {
    //   return false;
    // }
    uint8_t *status = new uint8_t[length + 1];
    if (status == nullptr) {
      return false;
    }
    status[0] = (uint8_t) length;
    memcpy(status + 1, data, length);
    Serial.print("Pushing status ");
    hex_dump(status);
    this -> status_queue.push(status);
    Serial.println(" -- done");
    return true;
  }

  bool handle_incoming() {
    uint8_t *status;
    while ((status = (uint8_t *) this -> status_queue.pop()) != nullptr) {
      if (this -> _waiting_for != nullptr) {
        if (this -> _waiting_for -> match(status)) {
          delete this -> _waiting_for;
          this -> _waiting_for = nullptr;
        }
      } else {
        this -> model.status_message(status);
      }
    }
    this -> send_challenge();
    this -> model.display();
  }

  void onConnect(BLEClient *client) {
    Serial.println("onConnect");
  }

  void onDisconnect(BLEClient *client) {
    this -> client = nullptr;
    this -> status_message("Disconnected");
  }
};

static SB20 *sb20 = nullptr;

static void notifyCallback(BLERemoteCharacteristic *status, uint8_t *data, size_t length, bool isNotify) {
  Serial.print("Notification for ");
  Serial.print(status->getUUID().toString().c_str());
  Serial.print(": ");
  hex_dump_raw(data, length);
  Serial.print(" [");
  Serial.print(length);
  Serial.println("]");

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
    if (sb20) {
      delete sb20;
    }
    sb20 = nullptr;
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(speed_cadence_service)) {
      BLEDevice::getScan()->stop();
      BLEAdvertisedDevice *device = new BLEAdvertisedDevice(advertisedDevice);
      sb20 = new SB20(device);
    } else {
      add_to_blacklist(advertisedDevice.toString().c_str());
    }
  }
};


void setup() {
  Heltec.begin(true /* Enable display */, true /* Disable LoRa */, true /* Enable Serial */);
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");

  Heltec.display->flipScreenVertically();
  Heltec.display->setContrast(255);
  Heltec.display->clear();
  display_message("Scanning");

  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan *scanner = BLEDevice::getScan();
  scanner->setAdvertisedDeviceCallbacks(new FindSB20Callback());
  scanner->setInterval(1349);
  scanner->setWindow(449);
  scanner->setActiveScan(true);
  scanner->start(5, false);
}


void loop() {
  if (sb20 != nullptr) {
    if (!sb20 -> is_connected()) {
      sb20 -> connect();
      switch (sb20 -> get_error_code()) {
        case NoSB20Service:
          add_to_blacklist(sb20 -> toString().c_str());
          sb20 -> status_message("Scanning");
          BLEDevice::getScan() -> start(0);
          break;
        case NoStatusChar:
        case NoCommandChar:
          sb20 -> status_message("ERG Mode");
          delay(4000);
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
    sb20 -> status_message("Scanning");
    BLEDevice::getScan() -> start(0);
  }
  delay(1000);
}
