def hexstr(data):
  return "{{ 0x{0:02x}, {1:s} }}".format(len(data), ", ".join(["0x{:02x}".format(b) for b in data]))


def c(chal, *responses):
  if len(responses):
    resp_str = "{{ {0}, {{ 0x00 }} }}".format(", ".join([hexstr(r) for r in responses]))
  else:
    resp_str = "{ { 0x00 } }"
  print("  {{ {0}, {1} }},".format(hexstr(chal), resp_str))

challenges = [
    (b'\x08\0', b'\x08\0'),
    (b'\x0c\0\x01', b'\x0c\0\x01'),
    (b'\x0a\0\0\0', b'\x0a\x01'),
    (b'\x0d\x02', b'\x0d\x02'),
    (b'\x0d\x04', b'\x0d\x04'),
    (b'\x0e\x00', b'\x0e\x00'),
    (b'\x08\0', b'\x08\0'),
    (b'\x0c\0\x01', b'\x0c\0\x01'),
    (b'\x0b\00\04\04\02\03\03\01\01\02\03\04\01\02\03\04\00', b'\x0b\0', b'\x0c\01'),
    (b'\x10\0\x01', b'\x10\0\x01', b'\x0c\01'),
    (b'\x0c\00\02\05\01\xc8\00\01', b'\05\01', b'\x0c\01'),
    (b'\x0c\0\x02', b'\x0c\01'),
    (b'\x0c\02\0\0\02\x0c\x10\0\03\x0e\0', b'\x0c\02', b'\x0c\01'),
    (b'\x0c\0\x02', b'\x0c\01'),
    (b'\x0c\03\x22\x32\x21\x1c\x18\x15\x13\x11\x0f\x0e\x0d\x0c\x0b\x0a',),
    (b'\x0c\x02\x0b\x00\x02\x0c\x10\x00\x03\x0e\x00', b'\x0c\x02', b'\x0c\x01'),
    (b'\xfd\0', b'\xfd\x01', b'\x0c\01'),
    (b'\x03\x01\x4c\x1d\0\0', b'\x03\01', b'\x0c\01'),
    (b'\x0c\0\x02', b'\x0c\01'),
    (b'\x0c\0\x02', b'\x0c\01'),
    (b'\x0c\0\x02', b'\x0c\01'),
]

print("challenge_data _bootstrap_data[{0}] = {{".format(len(challenges) + 1))
for ch in challenges:
    c(*ch)
print("  { { 0x00 }, { { 0x00 } } }")
print("};")

