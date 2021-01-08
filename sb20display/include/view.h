
#include "heltec.h"
#include "model.h"

class SB20View : public ModelListener {
private:
  static SB20View  *singleton;
  SB20Model        *model;

protected:
  explicit SB20View(SB20Model *);

public:
  static SB20View * instance() {
    return singleton;
  }

  static SB20View * getView(SB20Model *);

  virtual ~SB20View() { };

  virtual void display() = 0;

  virtual void refresh() { };

  virtual void display_message(const char *msg) { };

  void onLoop() {
    this -> display();
  }

  virtual void onDisplayMessage(const char *msg) {
    Serial.println(msg);
    this -> display_message(msg);
  }

  virtual void erase_message() { };
};


class SB20HeltecView : public SB20View {
private:
  SB20Model     *model;
  uint8_t        width;
  uint8_t        height;
  const char    *message = nullptr;
  const char    *displayed = nullptr;
  bool           redisplay = false;

protected:
  explicit SB20HeltecView(SB20Model *);

public:
  virtual ~SB20HeltecView() { };

  void    onGearChange();
  void    onSetup();

  void    display_message(const char *);
  void    erase_message();
  void    refresh();
  void    display();
};

