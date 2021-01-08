#ifndef __CHALLENGE_H__
#define __CHALLENGE_H__

#include <Arduino.h>
#include <deque>
#include <vector>

#include "listeners.h"


struct challenge_data {
  uint8_t   challenge[32];
  uint8_t   responses[32][8];
};


class Challenge {
private:
  Bytes              _challenge;
  std::vector<Bytes> _responses;

protected:
  void dump(const std::string &);

public:
  explicit Challenge(const challenge_data *);
  virtual ~Challenge();

  const Bytes &   challenge();
  const Bytes &   response(unsigned int);
  unsigned int    responses();
  bool            match(Bytes &);
};

class ChallengeDialogListener : public Listener {
public:
  virtual ~ChallengeDialogListener() = default;
  virtual void onDialogStart() { };
  virtual void onChallengeSent(Challenge *) { };
  virtual void onResponseReceived(Challenge *, Bytes &) { };
  virtual void onDialogComplete() { };
};


class ChallengeDialogConnector : public Sender {
public:
  virtual bool send_challenge(const Bytes &) = 0;
};


class ChallengeDialog : public ResponseListener {
private:
  std::deque<Challenge *>   challenges;
  ChallengeDialogListener  *listener;
  ChallengeDialogConnector *connector;
  Challenge                *waiting_for;

  bool send_challenge(void);
  
public:
  ChallengeDialog(ChallengeDialogConnector *, ChallengeDialogListener *);
  virtual ~ChallengeDialog();

  void add_challenge(Challenge *);
  void add_challenges(const challenge_data *);
  bool onResponse(Bytes &);
  void start(void);
  bool done(void) { return this -> waiting_for == nullptr; }
};

#endif /* __CHALLENGE_H__ */