#ifndef __MODEL_H__
#define __MODEL_H__

#include "Arduino.h"
#include "BLEDevice.h"
#include "BLEUtils.h"

#include "listeners.h"

//#define MODEL_DEBUG

struct Configuration {
  uint8_t      num_chainrings;
  uint8_t     *chainrings;
  uint8_t      num_cogs;
  uint8_t     *cogs;
  std::string  uuid;
  int          loaded;
  
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

class SB20Model : public ResponseListener, public Sender<ModelListener> {
private:
  Configuration      config;
  uint8_t            cur_chainring;
  uint8_t            cur_cog;
  uint16_t           last_cum_crank_revs = 0;
  uint16_t           last_crank_ev = 0;
  uint16_t           cur_cadence = 0;

  static SB20Model  *singleton;

  SB20Model();
  static void        erase_config();
  void               read_config();
  void               write_config() const;

  void               dump() const;

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
  uint16_t              rpm() const { return cur_cadence; }
  const std::string &   uuid() const { return config.uuid; }

  void                  gear_change(uint8_t, uint8_t);
  void                  update_cadence(uint16_t, uint16_t);
  void                  uuid(const std::string &);
  bool                  configuration(const Configuration &);
  const Configuration & configuration() const { return config; }

  void                  display_message(const char *) const;
  void                  onLoop();
  void                  onSetup();

  bool                  onResponse(Bytes &);

  std::string           toString() {
    return "SB20 Model";
  }

  void erase_message() const {
    display_message(nullptr);
  }
};

#endif /* __MODEL_H__ */