#ifndef __MODEL_H__
#define __MODEL_H__

#include "Arduino.h"
#include "BLEDevice.h"
#include "BLEUtils.h"

#include "listeners.h"

struct Configuration {
  uint8_t      num_chainrings;
  uint8_t     *chainrings;
  uint8_t      num_cogs;
  uint8_t     *cogs;
  std::string  uuid;
  
  Configuration & operator = (const Configuration &rhs) {
    if (&rhs != this) {
      num_chainrings = rhs.num_chainrings;
      delete[] chainrings;
      chainrings = new uint8_t[num_chainrings];
      memcpy(chainrings, rhs.chainrings, num_chainrings);
      num_cogs = rhs.num_cogs;
      delete[] cogs;
      cogs = new uint8_t[num_cogs];
      memcpy(cogs, rhs.cogs, num_cogs);
      uuid = rhs.uuid;
    }
    return *this;
  } 
};

class SB20Model : public ResponseListener, public Sender {
private:
  Configuration      config;
  uint8_t            cur_chainring;
  uint8_t            cur_cog;

  static SB20Model  *singleton;

  SB20Model();
  void               read_config();
  void               write_config() const;

public:
  static SB20Model * instance() {
    return singleton;
  }

  static SB20Model * getModel() {
    if (!singleton) {
      singleton = new SB20Model();
    }
    return singleton;
  }

  virtual ~SB20Model();

  uint8_t               num_chainrings() const { return config.num_chainrings; }
  uint8_t               num_cogs() const { return config.num_cogs; }
  uint8_t               chainring(int) const;
  uint8_t               cog(int) const;
  uint8_t               current_chainring() const;
  uint8_t               current_cog() const;
  const std::string &   uuid() const { return config.uuid; }

  void                  gear_change(uint8_t, uint8_t);
  void                  uuid(const std::string &);
  bool                  configuration(const Configuration &);
  const Configuration & configuration() const { return config; }

  void                  display_message(const char *) const;
  void                  onLoop();
  void                  onSetup();

  bool                  onResponse(Bytes &);

  constexpr static uint8_t gear_change_prefix[] = { 3, 0x0c, 0x01, 0x00 };
};

#endif /* __MODEL_H__ */