//
// Created by jan on 2021-01-06.
//

#ifndef __BYTES_H__
#define __BYTES_H__

#include <Arduino.h>

//#define BYTES_DEBUG

class Bytes {
private:
  uint8_t  sz;
  uint8_t *array;

  void assign(uint8_t, const uint8_t *);
  void copy(const Bytes &);
public:
  Bytes();
  Bytes(uint8_t, const uint8_t *);
  explicit Bytes(const uint8_t *);
  Bytes(const Bytes &);
  virtual ~Bytes();

  unsigned int    size() const { return sz; }
  const uint8_t * bytes() const;

  Bytes & operator = (const Bytes &rhs) {
    if (&rhs != this) {
      copy(rhs);
    }
    return *this;
  }

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

  bool match(const Bytes &other) const {

    // If array and sz are both set, we are valid:
    if (array && sz) {

      // If they are valid too
      if (other.array && other.sz) {

        // Compare up to the shortest of the two:
        int ret = memcmp(array, other.array, min((int) sz, (int) other.sz));

        // If we're equal up to the shortest length:
        return !ret;
      } else {

        // We are defined and they aren't, so we're larger:
        return 1;
      }

    } else {

      // We're not defined. If they are, they are larger. Else we're the same:
      return (other.array && other.sz) ? -1 : 0;
    }
  }

  int cmp(const uint8_t *other, int num) const {

    // If array and sz are both set, we are valid:
    if (array && sz) {

      // If they are valid too
      if (other && num) {

        // Compare up to the shortest of the two:
        int ret = memcmp(array, other, min((int) sz, num));

        // If we're equal up to the shortest length:
        if (!ret) {

          // Return 0 if the lengths are the same, and the diff of the lengths otherwise:
          return sz - num;
        } else {
          // If they differ, return the result of memcmp:
          return ret;
        }

      } else {

        // We are defined and they aren't, so we're larger:
        return 1;
      }

    } else {

      // We're not defined. If they are, they are larger. Else we're the same:
      return (other && num) ? -1 : 0;
    }
  }

  int cmp(const Bytes &other) const {
    return cmp(other.bytes(), (int) other.size());
  }

  friend bool operator == (const Bytes &lhs, const Bytes &rhs) {
    bool ret = !lhs.cmp(rhs);
#ifdef BYTES_DEBUG
    Serial.print("Comparing ");
    lhs.hex_dump();
    Serial.print(" and ");
    rhs.hex_dump();
    Serial.printf(". Result: %d\n", ret);
#endif
    return ret;
  }

  void hex_dump() const {
    hex_dump(array, sz);
  }

  void hex_dump_nl() const {
    hex_dump_nl(array, sz);
  }

  static void hex_dump(const uint8_t *, size_t);
  static void hex_dump_nl(const uint8_t *data, size_t length) {
    hex_dump(data, length);
    Serial.println();
  }
};

#endif /* __BYTES_H__ */
