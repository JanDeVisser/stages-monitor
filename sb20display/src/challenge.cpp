#include "challenge.h"

/* ----------------------------------------------------------------------- */

Challenge::Challenge() : _challenge(), _responses() {
}

Challenge::Challenge(const challenge_data &data) : _challenge(data.challenge), _responses() {
  int ix = 0;
  for (ix = 0; data.responses[ix][0]; ix++) {
    _responses.push_back(Bytes(data.responses[ix]));
  }
#ifdef FALLBACK
  uint8_t fallback[4] = {0x03, 0xfe, 0x00, 0x00};
  fallback[2] = (uint8_t) _challenge.size();
  _responses.push_back(Bytes(fallback));
#endif
#ifdef CHALLENGE_DEBUG
   this -> dump("Created ");
#endif
}

Challenge::Challenge(const Challenge &other) : _responses() {
  copy(other);
}

Challenge::~Challenge() {
}

void Challenge::copy(const Challenge &other) {
  _responses.clear();
  _challenge = other._challenge;
  for (int ix = 0; ix < other.responses(); ix++) {
    _responses.push_back(other.response(ix));
  }
#ifdef CHALLENGE_DEBUG
  this -> dump("Copied ");
#endif
}

void Challenge::dump(const std::string &prefix) const {
#ifdef CHALLENGE_DEBUG
  Serial.print(prefix.c_str());
  Serial.print(" Challenge ");
  _challenge.hex_dump();
  if (!_responses.empty()) {
    Serial.printf(", %d Response(s): ", _responses.size());
    for (int ix = 0; ix < _responses.size(); ix++) {
      if (ix > 0) {
        Serial.print(", ");
      }
      _responses[ix].hex_dump();
    }
  } else {
    Serial.print(", No Responses");
  }
  Serial.println();
#endif
}

const Bytes & Challenge::challenge() const {
  return _challenge;
}

const Bytes & Challenge::response(unsigned int ix) const {
  return _responses[ix];
}

unsigned int Challenge::responses() const {
  return _responses.size();
}

bool Challenge::match(Bytes &received) const {
#ifdef CHALLENGE_DEBUG
   this -> dump("Matching");
   Serial.print("Against ");
   received.hex_dump_nl();
#endif
  for (int ix = 0; ix < responses(); ix++) {
    if (received.match(response(ix))) {
      return true;
    }
  }
  return false;
}

/* ----------------------------------------------------------------------- */

ChallengeDialog::ChallengeDialog(Sender<ResponseListener> *owner) : challenges(), owner(owner) {
}

ChallengeDialog::~ChallengeDialog() {
}

void ChallengeDialog::add_challenge(const Challenge &challenge) {
  challenges.push_back(challenge);
}

void ChallengeDialog::add_challenges(const challenge_data *data) {
  for (int ix = 0; data[ix].challenge[0]; ix++) {
    add_challenge(Challenge(data[ix]));
  }
}
