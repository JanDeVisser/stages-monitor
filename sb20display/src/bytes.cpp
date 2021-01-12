#include <bytes.h>

Bytes::Bytes() : sz(0), array(nullptr) {
}

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
  delete[] array;
}

void Bytes::copy(const Bytes &other) {
  assign(other.size(), other.array);
}

void Bytes::assign(uint8_t num, const uint8_t *src) {
  this -> sz = num;
  delete[] array;
  if (num) {
    array = new uint8_t[num];
    memcpy(array, src, num);
  }
#ifdef BYTES_DEBUG
  Serial.print("Copied Bytes ");
  hex_dump_nl();
#endif
}

const uint8_t * Bytes::bytes() const {
  return array;
}

void Bytes::hex_dump(const uint8_t *data, size_t length) {
  if (!data) {
    Serial.print(" (null)");
  } else if (!length) {
    Serial.print(" (zero length)");
  } else {
    for (int ix = 0; ix < length; ix++) {
      Serial.printf(" %02x", data[ix]);
    }
    Serial.printf(" (%d bytes)", length);
  }
}


