#include "challenge.h"

/* ----------------------------------------------------------------------- */

Challenge::Challenge(const challenge_data *data) : _challenge(data -> challenge), _responses() {
  for (int ix = 0; data -> responses[ix][0]; ix++) {
    _responses.push_back(Bytes(data -> responses[ix]));
  }
#ifdef CHALLENGE_DEBUG
   this -> dump("Created ");
#endif
}

Challenge::~Challenge() {
}

void Challenge::dump(const std::string &prefix) {
#ifdef CHALLENGE_DEBUG
  Serial.print(prefix);
  Serial.print(" Challenge ");
  _challenge.hex_dump();
  if (this -> _responses.size() > 0) {
    Serial.print(", Response(s): ");
    for (int ix = 0; _responses.size(); ix++) {
      if (ix > 0) {
        Serial.print(", ");
      }
      _responses[ix].hex_dump();
    }
  }
  Serial.println();
#endif
}

const Bytes & Challenge::challenge() {
  return _challenge;
}

const Bytes & Challenge::response(unsigned int ix) {
  return _responses[ix];
}

unsigned int Challenge::responses() {
  return _responses.size();
}

bool Challenge::match(Bytes &received) {
#ifdef CHALLENGE_DEBUG
   this -> dump("Matching");
   Serial.print("Against ");
   hex_dump(received);
   Serial.println();
#endif
  for (int ix = 0; ix < responses(); ix++) {
    if (received == response(ix)) {
      return true;
    }
  }
  return false;
}

/* ----------------------------------------------------------------------- */

ChallengeDialog::ChallengeDialog(ChallengeDialogConnector *connector, ChallengeDialogListener *listener) :
    challenges(), listener(listener), connector(connector), waiting_for(nullptr) {
}

ChallengeDialog::~ChallengeDialog() {
}

bool ChallengeDialog::send_challenge() {
  if (!connector) {
    Serial.println("NO CONNECTOR IN CHALLENGEDIALOG");
    return false;
  }
  if (waiting_for) {
    return false;
  }
  while (!waiting_for) {
    if (challenges.empty()) {
      if (listener) {
        listener -> onDialogComplete();
      }
      return true;
    }
    waiting_for = challenges.front();
    challenges.pop_front();
#ifdef CHALLENGE_DEBUG
     this -> waiting_for -> dump("Sending");
#endif
    this -> connector -> send_challenge(waiting_for -> challenge());
    if (this -> listener) {
      this -> listener -> onChallengeSent(waiting_for);
    }
    if (waiting_for -> responses() == 0) {
      delete this -> waiting_for;
      this -> waiting_for = nullptr;
    }
  }
  return true;
}

void ChallengeDialog::add_challenge(Challenge *challenge) {
  challenges.push_back(challenge);
}

void ChallengeDialog::add_challenges(const challenge_data *data) {
  for (int ix = 0; data[ix].challenge[0]; ix++) {
    add_challenge(new Challenge(data + ix));
  }
}

bool ChallengeDialog::onResponse(Bytes &response) {
  if ((waiting_for != nullptr) && waiting_for -> match(response)) {
    if (listener) {
      listener -> onResponseReceived(waiting_for, response);
    }
    delete waiting_for;
    waiting_for = nullptr;
    this -> send_challenge();
    return true;
  } else {
    return false;
  }
}

void ChallengeDialog::start() {
  if (this -> waiting_for == nullptr) {
    if (this -> listener) {
      this -> listener -> onDialogStart();
    }
    this -> send_challenge();
  }
}
