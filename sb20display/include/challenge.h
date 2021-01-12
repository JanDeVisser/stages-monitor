#ifndef __CHALLENGE_H__
#define __CHALLENGE_H__

#include <Arduino.h>
#include <deque>
#include <vector>

#include "listeners.h"

//#define CHALLENGE_DEBUG

struct challenge_data {
  uint8_t   challenge[32];
  uint8_t   responses[8][32];
};


class Challenge {
private:
  Bytes              _challenge;
  std::vector<Bytes> _responses;
  void               copy(const Challenge &);

public:
  Challenge();
  Challenge(const Challenge &);
  explicit Challenge(const challenge_data &);
  virtual ~Challenge();

  const Bytes &   challenge() const;
  const Bytes &   response(unsigned int) const;
  unsigned int    responses() const;
  bool            match(Bytes &) const;
  void            dump(const std::string &) const;

  Challenge & operator = (const Challenge &rhs) {
    if (&rhs != this) {
      copy(rhs);
    }
    return *this;
  }

};

class ChallengeDialog : public ResponseListener {
private:
  std::deque<Challenge>     challenges;
  Sender<ResponseListener> *owner;
  Challenge                 waiting_for;
  bool                      is_waiting = false;

public:
  explicit ChallengeDialog(Sender<ResponseListener> *);
  virtual ~ChallengeDialog();

  void add_challenge(const Challenge &);
  void add_challenges(const challenge_data *);

  bool exhausted() {
    return challenges.empty();
  }

  Challenge pop() {
    Challenge ret = challenges.front();
    challenges.pop_front();
    return ret;
  }

  size_t size() {
    return challenges.size();
  }

  std::string toString() {
    return "Challenge Dialog";
  }

  virtual void onDialogStart() { };
  virtual void onChallengeSent(Challenge *) { };
  virtual void onResponseReceived(Challenge *, Bytes &) { };
  virtual void onDialogComplete() { };
};

#endif /* __CHALLENGE_H__ */