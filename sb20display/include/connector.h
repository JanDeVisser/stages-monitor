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
  public ModelListener,
  public ChallengeDialogConnector {
private:
  class SB20ConnectorListener : public ChallengeDialogListener {
  private:
    SB20Connector *connector;

  public:
    SB20ConnectorListener(SB20Connector *connector) : connector(connector) {
    }

    virtual ~SB20ConnectorListener() = default;

    void onDialogComplete() {
      connector -> dialog_complete();
      this -> connector -> erase_message();
      this -> notify_complete();
    }

    void notify_complete() { }
  };

  std::deque<Bytes>         msg_queue;
  SB20Model                *model;
  BLEClient                *client;
  BLERemoteService         *service;
  BLEAdvertisedDevice      *probe_device;

  std::set<std::string>     blacklist;
  std::string               device;
  BLERemoteCharacteristic  *command;
  BLERemoteCharacteristic  *status;
  ChallengeDialog          *dialog;
  ChallengeDialogListener  *listener;
  SB20Error                 error_code;
  unsigned long             connect_time;
  unsigned long             last_contact;
  unsigned long             last_shift;

  explicit SB20Connector(SB20Model *);

  bool          send_command(const Bytes &);
  bool          probe();
  bool          bootstrap();
  bool          start_dialog(ChallengeDialogListener *, const challenge_data *);
  void          dialog_complete();

  void display_message(const char *msg) {
    this -> model -> display_message(msg);
  }

  void erase_message() {
    this -> model -> display_message(nullptr);
  }

  static SB20Connector * singleton;

public:
  constexpr static char const * speed_cadence_service = "1816";
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

  bool          send_challenge(const Bytes &);
  bool          push_message(uint8_t *, int);
  bool          ping();

  void          onSetup();
  void          onLoop();
  void          onModelUpdate();
  void          onGearChange();

  void          onConnect(BLEClient *);
  void          onDisconnect(BLEClient *);

  bool          blacklisted(const std::string &);
  void          onResult(BLEAdvertisedDevice);

  constexpr static challenge_data bootstrap_data[] = {
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

  constexpr static challenge_data ping_data[] = { { 0x02, 0x0d, 0x04 }, { { 0x02, 0x0d, 0x04 }, { 0x00 } } };
};

#endif /* __CONNECTOR_H__ */