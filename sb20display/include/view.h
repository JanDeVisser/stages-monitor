#include "heltec.h"
#include "model.h"

//#define VIEW_DEBUG

class SB20View : public ModelListener {
private:
  static SB20View  *singleton;
  SB20Model        *_model;

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

  SB20Model * model() const {
    return _model;
  }

  void onLoop() {
    this -> display();
  }

  virtual void onDisplayMessage(const char *msg) {
#ifdef VIEW_DEBUG
    Serial.print("SB20View::onDisplayMessage ");
    Serial.println(msg);
#endif
    this -> display_message(msg);
  }

  virtual void erase_message() {
#ifdef VIEW_DEBUG
    Serial.println("SB20View::erase_message");
#endif
    this -> display_message(nullptr);
  };
};


class SB20HeltecView : public SB20View {
private:
  uint8_t        width;
  uint8_t        height;
  const char    *message = nullptr;
  bool           redisplay = false;

public:
  explicit SB20HeltecView(SB20Model *);
  virtual ~SB20HeltecView() { };

  void    onGearChange();
  void    onSetup();

  void    display_message(const char *);
  void    erase_message();
  void    refresh();
  void    display();
};

