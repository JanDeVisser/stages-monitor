//
// Created by jan on 2021-01-06.
//

#ifndef __BYTES_H__
#define __BYTES_H__

#include <Arduino.h>

class Bytes {
private:
  uint8_t  sz;
  uint8_t *array;

  void assign(uint8_t, const uint8_t *);
public:
  Bytes(uint8_t, const uint8_t *);
  explicit Bytes(const uint8_t *);
  Bytes(const Bytes &);
  virtual ~Bytes();

  unsigned int    size() const { return sz; }
  const uint8_t * bytes() const;

  explicit operator const uint8_t *() const {
    return bytes();
  }

  uint8_t operator[] (int ix) const {
    if (array && sz) {
      if (ix >= size()) {
        Serial.println("Bytes: Index out of bound");
        return array[sz - 1];
      } else {
        return array[ix];
      }
    } else {
      return 0;
    }
  }

  int cmp(const uint8_t *other, int num) const {
    if (array && sz) {
      if (other && num) {
        int ret = memcmp((uint8_t *) this, other, min((int) sz, num));
        if (!ret) {
          ret = sz - num;
        }
        return ret;
      } else {
        return 1;
      }
    } else {
      return (other && num) ? -1 : 0;
    }
  }

  int cmp(const Bytes &other) const {
    return cmp(other.bytes(), (int) other.size());
  }

  friend bool operator == (const Bytes &lhs, const Bytes &rhs) {
    return !lhs.cmp(rhs);
  }

  void hex_dump() const;
};

#endif /* __BYTES_H__ */
