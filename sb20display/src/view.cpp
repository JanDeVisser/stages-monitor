#include "view.h"

SB20View * SB20View::singleton = nullptr;

SB20View * SB20View::getView(SB20Model *model) {
  if (!singleton) {
    singleton = new SB20HeltecView(model);
  }
  return singleton;
}

SB20View::SB20View(SB20Model *model) : _model(model) {
  _model -> add_listener(this);
}

/* ----------------------------------------------------------------------- */

SB20HeltecView::SB20HeltecView(SB20Model *model) : SB20View(model) {
  Heltec.display -> flipScreenVertically();
  Heltec.display -> setContrast(255);
  height = Heltec.display -> getHeight();
  width = Heltec.display -> getWidth();
#ifdef VIEW_DEBUG
  Serial.printf("Initializing View: height: %d width %d\n", height, width);
#endif
}

void SB20HeltecView::onRefresh() {
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
//  Heltec.begin(true, false, true);
}

void SB20HeltecView::display() {
#ifdef VIEW_DEBUG
  Serial.println("SB20HeltecView::Display");
#endif
  if (!redisplay) {
    return;
  }
#ifdef VIEW_DEBUG
  Serial.println("Redisplay");
#endif
  Heltec.display -> clear();
  Heltec.display -> setColor(WHITE);

  if (this -> message && this -> message[0]) {
#ifdef VIEW_DEBUG
    Serial.print("Message: ");
    Serial.println(this -> message);
#endif
    Heltec.display -> setFont(ArialMT_Plain_10);
    Heltec.display -> setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display -> drawString(this -> width / 2, 12, this -> message);
  } else if ((model() -> rpm() > 30) && (model() -> rpm() < 200)) {
    Heltec.display -> setFont(ArialMT_Plain_16);
    Heltec.display -> setTextAlignment(TEXT_ALIGN_LEFT);
    char rpm_str[10];
    itoa(model() -> rpm(), rpm_str, 10);
    Heltec.display -> drawString(this -> width / 2, 16, rpm_str);
  }

  uint8_t x_inc = (uint8_t) (width  / (model() -> num_chainrings() + model() -> num_cogs() + 2));
  uint8_t w = (uint8_t) (2 * x_inc / 3);
  uint8_t x = (uint8_t) (x_inc / 2);
  uint8_t h_max = this -> height - 12;
  float factor = ((float) h_max) / ((float) model() -> chainring(model() -> num_chainrings()));
  for (int ix = 1; ix <= model() -> num_chainrings(); ix++) {
    float chainring = model() -> chainring(ix);
    uint8_t h = (uint8_t) roundf(chainring * factor);
    uint8_t y = height - h;
    if (ix == model() -> current_chainring()) {
      Heltec.display -> fillRect(x, y, w, h);
    } else {
      Heltec.display -> drawRect(x, y, w, h);
    }
    x += x_inc;
  }

  x += x_inc;
  factor = ((float) h_max) / (float) model() -> cog(1);
  for (int ix = 1; ix <= model() -> num_cogs(); ix++) {
    float cog = model() -> cog(ix);
    uint8_t h = (uint8_t) roundf(cog * factor);
    uint8_t y = height - h;
    if (ix == model() -> current_cog()) {
      Heltec.display -> fillRect(x, y, w, h);
    } else {
      Heltec.display -> drawRect(x, y, w, h);
    }
    x += x_inc;
  }

  Heltec.display -> display();
  this -> redisplay = false;
}
