#include "Arduino.h"
#include "Preferences.h"

#include "model.h"
#include "service.h"

SB20Model * SB20Model::singleton = nullptr;

SB20Model::SB20Model() {
//  this -> erase_config();
  this -> read_config();
  this -> cur_chainring = 0;
  this -> cur_cog = 0;
  dump();
#ifdef MODEL_DEBUG
  Serial.println("Model created");
#endif
}

SB20Model::~SB20Model() {
}

void SB20Model::erase_config() {
  Preferences prefs;

  prefs.begin("stages-sb20", false);
  prefs.clear();
  prefs.end();
}

void SB20Model::read_config() {
  Preferences prefs;

  prefs.begin("stages-sb20", true);
  config.loaded = 0;
  if (prefs.getBytesLength("num-chainrings") && prefs.getBytesLength("chainrings")) {
    uint8_t num = prefs.getUChar("num-chainrings", 2);
    config.chainrings = new uint8_t[num];
    config.num_chainrings = num;
    prefs.getBytes("chainrings", config.chainrings, num);
    config.loaded++;
  } else {
    config.num_chainrings = 2;
    config.chainrings = new uint8_t[2]{ 34, 50 };
  }

  if (prefs.getBytesLength("num-cogs") && prefs.getBytesLength("cogs")) {
    uint8_t num_cogs = prefs.getUChar("num-cogs", 12);
    config.cogs = new uint8_t[num_cogs + 1];
    config.num_cogs = num_cogs;
    prefs.getBytes("cogs", config.cogs, num_cogs);
    config.loaded += 2;
  } else {
    config.num_cogs = 12;
    config.cogs = new uint8_t[12]{ 33, 28, 24, 21, 19, 17, 15, 14, 13, 12, 11, 10 };
  }

  config.uuid = prefs.getString("sb20-uuid").c_str();

  prefs.end();
}

void SB20Model::write_config() const {
#ifdef MODEL_DEBUG
  Serial.println("SB20Model::write_config");
#endif
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

void SB20Model::dump() const {
#ifdef MODEL_DEBUG
  Serial.printf("Configuration (%d):\n", config.loaded);
  Serial.printf("Chainrings: %d: ", num_chainrings());
  for (int ix = 1; ix <= num_chainrings(); ix++) {
    Serial.printf("%d ", chainring(ix));
  }
  Serial.println();
  Serial.printf("Cogs: %d: ", num_cogs());
  for (int ix = 1; ix <= num_cogs(); ix++) {
    Serial.printf("%d ", cog(ix));
  }
  Serial.println();
  Serial.printf("SB20 uuid: '%s'\n", config.uuid.c_str());
#endif
}

uint8_t SB20Model::chainring(int ix) const {
  return ((ix >= 1) && (ix <= num_chainrings())) ? this -> config.chainrings[ix-1] : 0;
}

uint8_t SB20Model::cog(int ix) const {
  return ((ix >= 1) && (ix <= num_cogs())) ? this -> config.cogs[ix-1] : 0;
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
  for (std::vector<ModelListener *>::iterator it = this -> listeners.begin(); it != this -> listeners.end(); it++) {
    (*it) -> onModelUpdate();
  }
  return true;
}

void SB20Model::gear_change(uint8_t new_chainring, uint8_t new_cog) {
  if ((new_chainring != cur_chainring) || (new_cog != cur_cog)) {
    this -> cur_chainring = new_chainring;
    this -> cur_cog = new_cog;
    for (std::vector<ModelListener *>::iterator it = this -> listeners.begin(); it != this -> listeners.end(); it++) {
      (*it) -> onGearChange();
    }
  }
}

void SB20Model::uuid(const std::string &uuid) {
  this -> config.uuid = uuid;
  write_config();
  for (std::vector<ModelListener *>::iterator it = this -> listeners.begin(); it != this -> listeners.end(); it++) {
    (*it) -> onModelUpdate();
  }
}

void SB20Model::display_message(const char *msg) const {
  for (std::vector<ModelListener *>::const_iterator it = this -> listeners.cbegin(); it != this -> listeners.cend(); it++) {
    (*it) -> onDisplayMessage(msg);
  }
}

void SB20Model::onSetup() {
#ifdef MODEL_DEBUG
  Serial.println("Setup");
#endif
  for (std::vector<ModelListener *>::iterator it = this -> listeners.begin(); it != this -> listeners.end(); it++) {
#ifdef MODEL_DEBUG
    Serial.print("Setting up ");
    Serial.println((*it) -> toString().c_str());
#endif
    (*it) -> onSetup();
  }
}

void SB20Model::onLoop() {
  for (std::vector<ModelListener *>::iterator it = this -> listeners.begin(); it != this -> listeners.end(); it++) {
    (*it) -> onLoop();
  }
}

bool SB20Model::onResponse(Bytes &response) {
  static uint8_t gear_change_prefix[4] = { 0x03, 0x0c, 0x01, 0x00 };
  static Bytes gear_change_prefix_bytes = Bytes(gear_change_prefix);
  if ((response.size() > 5) && response.match(gear_change_prefix_bytes)) {
    this -> gear_change(response[3], response[4]);
    return true;
  } else {
    return false;
  }
}

