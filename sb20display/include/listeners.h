//
// Created by jan on 2021-01-06.
//

#ifndef __LISTENERS_H__
#define __LISTENERS_H__

#include <bytes.h>

class Listener {
public:
  virtual ~Listener() { };
};

class ModelListener : public Listener {
public:
  virtual ~ModelListener() { };
  virtual void onSetup() { };
  virtual void onLoop() { };
  virtual void onModelUpdate() { };
  virtual void onGearChange() { };
  virtual void onDisplayMessage(const char *msg) { };
};

class ResponseListener : public Listener {
public:
  virtual bool onResponse(Bytes &) { return false; }
};


class Sender {
protected:
  std::vector<Listener *>  listeners;

public:
  virtual ~Sender() { }

  virtual void add_listener(Listener *listener) {
    for (int ix = 0; ix < listeners.size(); ix++) {
      if (listeners[ix] == listener) {
        return;
      }
    }
    listeners.push_back(listener);
  }

  virtual void remove_listener(Listener *listener) {
    for (int ix = 0; ix < listeners.size(); ix++) {
      if (listeners[ix] == listener) {
        listeners.erase(listeners.begin() + ix);
        return;
      }
    }
  }

};

#endif /* __LISTENERS_H__ */