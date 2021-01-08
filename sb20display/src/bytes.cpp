#include <bytes.h>

Bytes::Bytes(const Bytes &src) : sz(0), array(nullptr) {
  assign(src.size(), src.bytes());
}

Bytes::Bytes(const uint8_t *src) : sz(0), array(nullptr) {
  assign(src[0], src + 1);
}

Bytes::Bytes(uint8_t sz, const uint8_t *src) : sz(0), array(nullptr) {
  assign(sz, src);
}

Bytes::~Bytes() {
  delete array;
}

void Bytes::assign(uint8_t num, const uint8_t *src) {
  this -> sz = num;
  if (num) {
    array = new uint8_t[num];
    for (int ix = 0; ix < num; ix++) {
      array[ix + 1] = src[ix];
    }
  }
}

const uint8_t * Bytes::bytes() const {
  return array;
}

void Bytes::hex_dump() const {
  if (!array) {
    Serial.print(" (empty)");
  } else {
    Serial.printf("%3d:", size());
    for (int ix = 0; ix < size(); ix++) {
      Serial.printf(" %02x", this -> bytes()[ix]);
    }
  }
}


