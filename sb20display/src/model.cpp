#include "Arduino.h"
#include "Preferences.h"

#include "model.h"
#include "service.h"

SB20Model * SB20Model::singleton = nullptr;

SB20Model::SB20Model() {
  this -> read_config();
  this -> cur_chainring = 0;
  this -> cur_cog = 0;
  this -> add_listener(ConfigService::getService(this));
}

SB20Model::~SB20Model() {
}

void SB20Model::read_config() {
  Preferences prefs;

  prefs.begin("stages-sb20", false);
  if (prefs.getBytesLength("num-chainrings") && prefs.getBytesLength("chainrings")) {
    uint8_t num_chainrings = prefs.getUChar("num-chainrings", 2);
    this -> config.chainrings = new uint8_t[num_chainrings];
    this -> config.num_chainrings = num_chainrings;
    prefs.getBytes("chainrings", this -> config.chainrings, num_chainrings);
  } else {
    this -> config.num_chainrings = 2;
    this -> config.chainrings = new uint8_t[2]{ 34, 50 };
  }

  if (prefs.getBytesLength("num-cogs") && prefs.getBytesLength("cogs")) {
    uint8_t num_cogs = prefs.getUChar("num-cogs", 12);
    this -> config.cogs = new uint8_t[num_cogs + 1];
    this -> config.num_cogs = num_cogs;
    prefs.getBytes("cogs", this -> config.cogs, num_cogs);
  } else {
    this -> config.num_cogs = 12;
    this -> config.cogs = new uint8_t[12]{ 33, 28, 24, 21, 19, 17, 15, 14, 13, 12, 11, 10 };
  }

  this -> config.uuid = prefs.getString("sb20-uuid").c_str();

  prefs.end();
}

void SB20Model::write_config() const {
  Preferences prefs;

  prefs.begin("stages-sb20", false);
  prefs.clear();
  prefs.putUChar("num-chainrings", num_chainrings());
  prefs.putBytes("chainrings", this -> config.chainrings, num_chainrings());
  prefs.putUChar("num-cogs", num_cogs());
  prefs.putBytes("cogs", this -> config.cogs, num_cogs());
  prefs.putString("sb20-uuid", config.uuid.c_str());
  prefs.end();
}

uint8_t SB20Model::chainring(int ix) const {
  return ((ix >= 1) && (ix <= num_chainrings())) ? this -> config.chainrings[ix] : 0;
}

uint8_t SB20Model::cog(int ix) const {
  return ((ix >= 1) && (ix <= num_cogs())) ? this -> config.cogs[ix] : 0;
}

uint8_t SB20Model::current_chainring() const {
  return this -> cur_chainring;
}

uint8_t SB20Model::current_cog() const {
  return this -> cur_cog;
}

bool SB20Model::configuration(const Configuration &newConfig) {
  config = newConfig;
  write_config();
  for (std::vector<Listener *>::iterator it = this -> listeners.begin(); it != this -> listeners.end(); it++) {
    ((ModelListener *)(*it)) -> onModelUpdate();
  }
  return true;
}

void SB20Model::gear_change(uint8_t new_chainring, uint8_t new_cog) {
  if ((new_chainring != cur_chainring) || (new_cog != cur_cog)) {
    this -> cur_chainring = new_chainring;
    this -> cur_cog = new_cog;
    for (std::vector<Listener *>::iterator it = this -> listeners.begin(); it != this -> listeners.end(); it++) {
      ((ModelListener *)(*it)) -> onGearChange();
    }
  }
}

void SB20Model::uuid(const std::string &uuid) {
  this -> config.uuid = uuid;
  write_config();
  for (std::vector<Listener *>::iterator it = this -> listeners.begin(); it != this -> listeners.end(); it++) {
    ((ModelListener *)(*it)) -> onModelUpdate();
  }
}

void SB20Model::display_message(const char *msg) const {
  for (std::vector<Listener *>::const_iterator it = this -> listeners.cbegin(); it != this -> listeners.cend(); it++) {
    ((ModelListener *)(*it)) -> onDisplayMessage(msg);
  }
}

void SB20Model::onSetup() {
  for (std::vector<Listener *>::iterator it = this -> listeners.begin(); it != this -> listeners.end(); it++) {
    ((ModelListener *)(*it)) -> onSetup();
  }
}

void SB20Model::onLoop() {
  for (std::vector<Listener *>::iterator it = this -> listeners.begin(); it != this -> listeners.end(); it++) {
    ((ModelListener *)(*it)) -> onLoop();
  }
}

bool SB20Model::onResponse(Bytes &response) {
  if ((response.size() > 5) && !response.cmp(gear_change_prefix + 1, gear_change_prefix[0])) {
    this -> gear_change(response[4], response[5]);
    return true;
  } else {
    return false;
  }
}

