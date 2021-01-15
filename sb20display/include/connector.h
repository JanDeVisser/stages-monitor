#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__

#include "Arduino.h"
#include "BLEDevice.h"
#include "BLEAdvertisedDevice.h"

#include "deque"
#include "set"
#include "vector"

#include "challenge.h"
#include "model.h"

//#define CONNECTOR_DEBUG

enum SB20Error {
  NoError,
  NoConnection,
  NoSB20Service,
  NoCommandChar,
  NoStatusChar
};


class SB20Connector :
  public BLEClientCallbacks,
  public BLEAdvertisedDeviceCallbacks,
  public Sender<ResponseListener>,
  public ModelListener,
  public ResponseListener {
private:
  std::deque<Bytes>              msg_queue;
  SB20Model                     *model = nullptr;
  BLEClient                     *client = nullptr;
  BLERemoteService              *service = nullptr;
  BLERemoteService              *sc_service = nullptr;
  BLEAdvertisedDevice           *probe_device = nullptr;

  std::set<std::string>          blacklist;
  std::string                    device;
  BLERemoteCharacteristic       *command = nullptr;
  BLERemoteCharacteristic       *status = nullptr;
  BLERemoteCharacteristic       *speed_cadence = nullptr;
  SB20Error                      error_code = NoError;
  unsigned long                  connect_time = 0L;
  unsigned long                  last_contact = millis();
  unsigned long                  last_shift = 0L;

  bool                           is_waiting = false;
  std::deque<ChallengeDialog *>  dialogs;
  ChallengeDialog               *current_dialog = nullptr;
  Challenge                      waiting_for;

  explicit SB20Connector(SB20Model *);

  bool          send_command(const Bytes &);
  bool          probe();
  bool          bootstrap();
  bool          add_dialog(const challenge_data *);

  void display_message(const char *msg) {
    model -> display_message(msg);
  }

  void erase_message() {
    model -> erase_message();
  }

  static SB20Connector * singleton;

public:
  constexpr static char const * speed_cadence_service = "1816";
  constexpr static char const * speed_cadence_measurement = "2a5b";

  constexpr static char const * power_service = "1818";
  constexpr static char const * power_measurement = "2a63";

  constexpr static char const * sb20_service = "0c46beaf-9c22-48ff-ae0e-c6eae1a2f4e5";
  constexpr static char const * sb20_status = "0c46beb0-9c22-48ff-ae0e-c6eae1a2f4e5";
  constexpr static char const * sb20_command = "0c46beb1-9c22-48ff-ae0e-c6eae1a2f4e5";

  static SB20Connector * instance() {
    return singleton;
  }

  static SB20Connector * getConnector(SB20Model *model) {
    if (!singleton) {
      singleton = new SB20Connector(model);
    }
    return singleton;
  }

  virtual ~SB20Connector();

  bool          is_sb20();
  bool          connect(const std::string &);
  std::string   toString();
  bool          is_connected();
  unsigned long connected_since() const;
  SB20Error     get_error_code();

  bool          send_challenge();
  bool          push_message(uint8_t *, int);
  bool          cadence_message(const uint8_t *);
  bool          ping();

  void          onSetup();
  void          onLoop();
  void          onModelUpdate();
  void          onRefresh();

  void          onConnect(BLEClient *);
  void          onDisconnect(BLEClient *);
  bool          onResponse(Bytes &);

  bool          blacklisted(const std::string &);
  void          onResult(BLEAdvertisedDevice);

  static challenge_data bootstrap_data[];
  static challenge_data ping_data[];
};

#endif /* __CONNECTOR_H__ */