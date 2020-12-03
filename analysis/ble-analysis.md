#### 0 `CONNECT_REQ`

#### 1 `LL_VERSION_IND` (Link Layer Version Negotiation): 5.0

#### 2 `LL_FEATURE_REQ` (Feature Negotiation)

#### 21 GATT Primary Service Declaration `0x0001-0xffff`

- `0x0001-0x0009` Generic Access Profile `0x1800`
- `0x000a-0x000d` Generic Attribute Profile `0x1801`
- `0x000e-0x001a` Device Information `0x180a`

#### 25 GATT Primary Service Declaration `0x001b-0xffff`

- `0x001b-0x002c` Fitness Machine `0x1826`

#### 32 GATT Primary Service Declaration `0x002d-0xffff`

- `0x002d-0x0033` `0c 46 be 5f 9c 22 48 ff ae 0e c6 ea e1 a2 f4 e5`

#### 34 GATT Primary Service Declaration `0x0034-0xffff`

- `0x0034-0x0039` `0c 46 be af 9c 22 48 ff ae 0e c6 ea e1 a2 f4 e5`

#### 41 GATT Primary Service Declaration `0x003a-0xffff`

- `0x003a-0x0044` Cycling Speed and Cadence `0x1816`
- `0x0045-0xffff` Nordic Semiconductor ASA `0xfe59`

#### 45 Read GATT Characteristic Declaration `0x000a-0x000d`

- Service Generic Attribute Profile `0x1801`
  - `0x000d` Service Changed `0x2a05` (Indicate)

#### 49 Find Information 0x000d

- `0x000d` Client Characteristic Configuration `0x2902`
  
Svc UUID: Generic Attribute Profile Char UUID: Service Changed

FIXME

#### 54 Write Req `0x000d` Indication

#### 60 Read Device Name `0x0001-0x0009`

- `0x0003` Device Name `0x2a00`: Stages Bike 0096

#### 64 Read GATT Characteristic Declaration `0x000e-0x001a`

- Service Device Information `0x180a`
  - `0x0010` Manufacturer Name `0x2a29` (Read)
  - `0x0012` Model Number `0x2a24` (Read)
  - `0x0014` Serial Number `0x2a25` (Read)

#### 72 Read GATT Characteristic Declaration `0x0015-0x001a`

- Service Device Information `0x180a` 
  - `0x0016` Hardware Revision `0x2a27` (Read)
  - `0x0018` Firmware Revision `0x2a26` (Read)
  - `0x001a` Software Revision `0x2a28` (Read)

#### 74 Read GATT Characteristic Declaration `0x0034-0x0039`

- Service `0c 46 be af 9c 22 48 ff ae 0e c6 ea e1 a2 f4 e5`
  - `0x0036` `0c 46 be b0 9c 22 48 ff ae 0e c6 ea e1 a2 f4 e5` (Read)

#### 77 Read GATT Characteristic Declaration `0x0037-0x0039`

- Service `0c 46 be af 9c 22 48 ff ae 0e c6 ea e1 a2 f4 e5`
  - `0x0039` `0c 46 be b1 9c 22 48 ff ae 0e c6 ea e1 a2 f4 e5` (Write w/o response)

#### 81 Read Device Information: Manufacturer Name `0x0010`

- `0x0010` Manufacturer Name `0x2a29`: Stages Cycling

#### 85 Read Device Information: Model Number `0x0010`

- `0x0012` Model Name `0x2a24`: SB20

#### 88 Read Device Information: Serial Number `0x0014`

- `0x0014` Model Name `0x2a25`: A0531200096

#### 97 Read Device Information: Firmware Revision `0x0018`

- `0x0018` Firmware Revision `0x2a26`: 1.1

#### 99 Read Device Information: Software Revision `0x001a`

- `0x001a` Firmware Revision `0x2a28`: 1.10.2+3015

#### 103 Find Information `0x0037`

- `0x0037`
   - Service `0c 46 be af 9c 22 48 ff ae 0e c6 ea e1 a2 f4 e5`
   - Characteristic `0c 46 be b0 9c 22 48 ff ae 0e c6 ea e1 a2 f4 e5`

#### 107 Write Req `0x0037` Notification

#### 115 Write Cmd `0x0039`: `08 00`

#### 116 Handle Value `0x0036`: `08 00`

#### 115 Write Cmd `0x0039`: `0c 00 01`

#### 123 Handle Value `0x0036`: `0c 00 01`

#### 140 Write Cmd `0x0039`: `0a 00 00 00`

#### 123 Handle Value `0x0036`: `0a 01 4a 01 d8 0b 36 8b 20 3e 2c d9 69 03 6e 3d`

#### 145 Write Cmd `0x0039`: `0d 02`

#### 123 Handle Value `0x0036`: `0d 02 d8 0b 31 2e 37 2e 31`

#### 149 Write Cmd `0x0039`: `0d 04`

#### 151 Handle Value `0x0036`: `0d 04 2c d9 31 2e 37 2e 31`

#### 153 Write Cmd `0x0039`: `0e 00`

#### 155 Handle Value `0x0036`: `0e 00 00 00 00 01 01`


| Event | Sent | Received |
|-------|------|----------|
|  156 | `08 00` | |
|  159 | | `08 00` |
|  163 | `0c 00 01` | |
|  164 | | `0c 00 01` |
|  165 | `0b 00 04 04 02 03 03 01 01 02 03 04 01 02 03 04 00` | |
|  167 | | `0b 00 04 04 02 03 03 01 01 02 03 04 01 02 03 04 00 fd 04 0c 01 00 02 01 34 00 21 00 06 00` |
|  168 | `10 00 01` | |
|  171 | | `10 00 01 fd 04 0c 01 00 02 01 34 00 21 00 06 00` |
|  175 | `0c 00 02 05 01 c8 00 01` | | 
|  176 | | `05 01 c8 00 01 fd 04` |
|  177 | `0c 00 02` | `0c 01 00 02 01 34 00 21 00 06 00` |
|  180 | `0c 02 00 00 02 0c 10 00 03 0e 00` | `0c 01 00 02 01 34 00 21 00 06 00` |
|  184 | `0c 02 00 00 02 0c 10 00 03 0e 00` | `0c 01 00 02 01 34 00 21 00 06 00 3a 25 00 02 c1 3e 06 0a 01 00 41 b8 00 96 00 00 00 66 ba 9a af 0a 12 0e 00 04 00 1b 36 00 0c 02 01 00 02 0c 10 00 03 0e 00 31 17 1c`|
|  185 | `0c 00 02` | |
|  187 | | `0c 01 00 02 01 34 00 21 00 00 03` |
|  188 | `0c 03 22 32 21 1c 18 15 13 11 0f 0e 0d 0c 0b 0a` | | |
|  201 | `0c 02 0b 00 02 0c 10 00 03 0e 00` | |
|  202 | | `0c 02 0c 00 02 0c 10 00 03 0e 00` |
|  205 | `fd 00` | `0c 01 00 01 01 22 00 21 00 00 03` |
|  206 | | `fd01` |
|  211 | `03 01 4c 1d 00 00` | `fd 03 fd 04 03 01 4c 1d 00 00 fd 04 fd 04` |
|  214 | `0c 00 02` | `0c 01 00 01 01 22 00 21 00 00 03` |
|  215 | `0c 00 02` | `0c 01 00 01 01 22 00 21 00 00 03 - 0c 01 00 01 01 22 00 21 00 00 03 - 0c 01 00 01 01 22 00 21 00 00 03` |
|  218 | `0c 00 02` | |
|  219 | | `0c 01 00 01 01 22 00 21 00 00 03` |
|  299 | | `0c 01 00 01 02 22 00 1c 00 00 03` |
|  327 | | `0c 01 00 01 03 22 00 18 00 00 03` |
|  353 | | `0c 01 00 01 04 22 00 15 00 00 03` |
|  380 | | `0c 01 00 01 05 22 00 13 00 00 03` |
|  409 | | `0c 01 00 01 06 22 00 11 00 00 03` |
|  439 | | `0c 01 00 01 07 22 00 0f 00 00 03` |
|  471 | | `0c 01 00 01 08 22 00 0e 00 00 03` |
|  496 | | `0c 01 00 01 09 22 00 0d 00 00 03` |
|  591 | | `0c 01 00 01 0a 22 00 0c 00 00 03` |
|  619 | | `0c 01 00 01 0b 22 00 0b 00 00 03` |
|  648 | | `0c 01 00 01 0c 22 00 0a 00 00 03` |
|  722 | | `0c 01 00 01 0c 22 00 0a 00 00 03` |
|  790 | | `0c 01 00 01 0c 22 00 0a 00 00 03` |
|  821 | | `0c 01 00 01 0c 22 00 0a 00 00 03` |
|  951 | | `0c 01 00 01 0b 22 00 0b 00 00 03` |
|  985 | | `0c 01 00 01 09 22 00 0d 00 00 03 - 0c 01 00 01 09 22 00 0d 00 00 03` |
| 1004 | | `0c 01 00 01 08 22 00 0e 00 00 03` |
| 1025 | | `0c 01 00 01 07 22 00 0f 00 00 03` |
| 1061 | | `0c 01 00 01 06 22 00 11 00 00 03` |
| 1098 | | `0c 01 00 01 05 22 00 13 00 00 03` |
| 1128 | | `0c 01 00 01 04 22 00 15 00 00 03`





           0c 01 00 01 05 22 00 13 00 06 00
11-Small | 0c 01 00 01 02 22 00 1c 00 00 03
10-Small | 0c 01 00 01 03 22 00 19 00 00 03
 9-Small | 0c 01 00 01 04 22 00 16 00 00 03
10-Small | 0c 01 00 01 03 22 00 19 00 00 03
11-Small | 0c 01 00 01 02 22 00 1c 00 00 03
12-Small | 0c 01 00 01 01 22 00 20 00 00 03
12-Big   | 0c 01 00 02 01 32 00 20 00 00 03
12-Small | 0c 01 00 01 01 22 00 20 00 00 03




2x12 - 50/34 x 10/33

50 = 0x32
34 = 0x22

10 = 0x0A
33 = 0x21


12       | 0c 01 00 02 01 32 00 21 00 00 03
