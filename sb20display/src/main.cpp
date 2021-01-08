#include "Arduino.h"

#include "model.h"
#include "connector.h"
#include "view.h"

static SB20Model *model = nullptr;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting SB20 Monitor");
  model = SB20Model::getModel();
  SB20View::getView(model);
  SB20Connector::getConnector(model);
  model -> onSetup();
  Serial.println("Started SB20 Monitor");
}

void loop() {
  model -> onLoop();
  delay(100);
}
