#include "Arduino.h"

#include "model.h"
#include "connector.h"
#include "service.h"
#include "view.h"

static SB20Model *model = nullptr;

//#define MAIN_DEBUG

void setup() {
  Heltec.begin(true, false, true);

  Serial.begin(115200);
  delay(2000);
#ifdef MAIN_DEBUG
  Serial.println("Starting SB20 Monitor");
#endif
  model = SB20Model::getModel();
  SB20View::getView(model);
  SB20Connector::getConnector(model);
  ConfigService::getService(model);
  model -> onSetup();
#ifdef MAIN_DEBUG
  Serial.println("Started SB20 Monitor");
#endif
}

void loop() {
#ifdef MAIN_DEBUG
  Serial.println("Loop");
#endif
  model -> onLoop();
  delay(100);
}
