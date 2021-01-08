#include "view.h"

SB20View * SB20View::singleton = nullptr;

SB20View * SB20View::getView(SB20Model *model) {
  if (!singleton) {
    singleton = new SB20HeltecView(model);
  }
  return singleton;
}

SB20View::SB20View(SB20Model *model) : model(model) {
}

/* ----------------------------------------------------------------------- */

SB20HeltecView::SB20HeltecView(SB20Model *model) : SB20View(model), model(model) {
  Heltec.display -> flipScreenVertically();
  Heltec.display -> setContrast(255);
  this -> height = Heltec.display -> getHeight();
  this -> width = Heltec.display -> getWidth();
}

void SB20HeltecView::onGearChange() {
  this -> redisplay = true;
}

void SB20HeltecView::display_message(const char *msg) {
  this -> message = msg;
  this -> redisplay = true;
}

void SB20HeltecView::erase_message() {
  this -> message = nullptr;
  this -> redisplay = true;
}

void SB20HeltecView::refresh() {
  this -> redisplay = true;
}

void SB20HeltecView::onSetup() {
  Heltec.begin(true, false, true);
}

void SB20HeltecView::display() {
  if (redisplay) {
    return;
  }
  Heltec.display -> clear();
  Heltec.display -> setColor(WHITE);

  if (this -> message && this -> message[0]) {
    Serial.println(this -> message);
    this -> display_time = millis();
    Heltec.display -> setFont(ArialMT_Plain_10);
    Heltec.display -> setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display -> drawString(this -> width / 2, 12, this -> displayed);
  }

  uint8_t x_inc = (uint8_t) (width  / (model -> num_chainrings() + model -> num_cogs() + 2));
  uint8_t w = (uint8_t) (2 * x_inc / 3);
  uint8_t x = (uint8_t) (x_inc / 2);
  uint8_t h_max = this -> height - 12;
  uint8_t y_0 = 0;
  float factor = ((float) (2 * h_max)) / ((float) model -> chainring(model -> num_chainrings()));
  for (int ix = 1; ix <= model -> num_chainrings(); ix++) {
    float chainring = model -> chainring(ix);
    float h = chainring * factor;
    uint8_t y = (uint8_t) ((float) height - h); //(y_0 + (h_max - h)/2);
    if (ix == model -> current_chainring()) {
      Heltec.display -> fillRect(x, y, w, h);
    } else {
      Heltec.display -> drawRect(x, y, w, h);
    }
    x += x_inc;
  }

  x += x_inc;
  factor = ((float) h_max) / (float) model -> cog(1);
  for (int ix = 1; ix <= model -> num_cogs(); ix++) {
    float cog = model -> cog(ix);
    float h = cog * factor;
    uint8_t y = (uint8_t) ((float) height - h); // (y_0 + (h_max - h)/2);
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
