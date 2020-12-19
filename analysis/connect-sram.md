
### Connection dialog

Appears to be mostly meta data exchange.

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
  - `0x0036` `0c 46 be b0 9c 22 48 ff ae 0e c6 ea e1 a2 f4 e5` (Notify)

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

### Setup and shifting dialog

Use case:
- Shifting profile 2x12. Front 50/34 (`0x32`/`0x22`), Rear 10-33 (`0x0a`-`0x21`)
- Mode: Simulation
- Shift down and back up cassette in small ring
- Shift to big ring. Shift down and back up cassette. At some point shifted back to small ring

| Event | Sent | Received | Remarks
|-------|------|----------| -------
|  115 | `08 00` | | Reset?
|  116 | | `08 00` |
|  115 | `0c 00 01` | |
|  123 | | `0c 00 01` |
|  140 | `0a 00 00 00` | |
|  123 | | `0a 01 4a 01 d8 0b 36 8b 20 3e 2c d9 69 03 6e 3d` |
|  145 | `0d 02` | |
|  123 | | `0d 02 d8 0b 31 2e 37 2e 31` |
|  149 | `0d 04` | |
|  151 | | `0d 04 2c d9 31 2e 37 2e 31` |
|  153 | `0e 00` | |
|  155 | | `0e 00 00 00 00 01 01` |
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
|  188 | `0c 03 22 32 21 1c 18 15 13 11 0f 0e 0d 0c 0b 0a` | | Set up shifting profile. `0c 03`? `0x22` and `0x32` chain rings; `0x21` - `0x0a` cogs.
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
| 1128 | | `0c 01 00 01 04 22 00 15 00 00 03` |
| 1147 | | `0c 01 00 01 03 22 00 18 00 00 03` |
| 1173 | | `0c 01 00 01 02 22 00 1c 00 00 03` |
| 1217 | | `0c 01 00 01 01 22 00 21 00 00 03` |
| 1289 | | `0c 01 00 02 01 32 00 21 00 00 03` |
| 1424 | | `0c 01 00 02 02 32 00 1c 00 00 03` |
| 1451 | | `0c 01 00 02 03 32 00 18 00 00 03` |
| 1462 | | `0c 01 00 02 04 32 00 15 00 00 03` |
| 1476 | | `0c 01 00 02 05 32 00 13 00 00 03` |
| 1491 | | `0c 01 00 02 06 32 00 11 00 00 03` |
| 1505 | | `0c 01 00 02 07 32 00 0f 00 00 03` |
| 1519 | | `0c 01 00 02 08 32 00 0e 00 00 03` |
| 1533 | | `0c 01 00 02 09 32 00 0d 00 00 03` |
| 1545 | | `0c 01 00 02 0a 32 00 0c 00 00 03` |
| 1558 | | `0c 01 00 02 0b 32 00 0b 00 00 03` |
| 1575 | | `0c 01 00 02 0c 32 00 0a 00 00 03` |
| 1592 | | `0c 01 00 02 0c 32 00 0a 00 00 03` |
| 1644 | | `0c 01 00 01 0c 22 00 0a 00 00 03` |
| 1713 | | `0c 01 00 01 0b 22 00 0b 00 00 03` |
| 1738 | | `0c 01 00 01 0a 22 00 0c 00 00 03` |
| 1749 | | `0c 01 00 01 09 22 00 0d 00 00 03` |
| 1759 | | `0c 01 00 01 08 22 00 0e 00 00 03` |
| 1774 | | `0c 01 00 01 07 22 00 0f 00 00 03` |
| 1783 | | `0c 01 00 01 06 22 00 11 00 00 03` |
| 1848 | | `0c 01 00 02 06 32 00 11 00 00 03` |
| 1919 | | `0c 01 00 02 05 32 00 13 00 00 03` |
| 1930 | | `0c 01 00 02 04 32 00 15 00 00 03` |
| 1939 | | `0c 01 00 02 03 32 00 18 00 00 03` |
| 1952 | | `0c 01 00 02 02 32 00 1c 00 00 03` |
| 1960 | | `0c 01 00 02 01 32 00 21 00 00 03` |


### Changing shifting profile 

_File:_ `change-zhram-dreamy-shifting.pcapng`

_Use case:_
- App connected
- Shifting profile 2x12. Front 50/34 (`0x32`/`0x22`), Rear 10-33 (`0x0a`-`0x21`)
- Mode: External
- Change to dream drive profile: 50 wide, left buttons jump by 3 gears
- Shift 4x down and back up cassette in small ring
- Shift 3x up and down with left hand

| Event | Sent | Received | Remarks
|-------|------|----------| -------
|   567 | `0b 00 01 02 05 03 04 05 01 02 03 04 01 02 03 04 00` | | |
| 568 | | `0b 00 01 02 05 03 04 05 01 02 03 04 01 02 03 04 00` | |
|  | | `fd 04` | |
|   571 | `0c 00 02` | `0c 01 00 01 01 22 00 21 00 00 03` | |
|   572 | `10 00 01` | `0c 01 00 01 01 22 00 21 00 00 03` | |
| 575 | | `10 00 01` | |
|  | | `fd 04` | |
|  | | `0c 01 00 01 01 22 00 21 00 00 03` | |
|   577 | `0c 00 02` | | |
|   580 | | `0c 01 00 01 01 22 00 21 00 00 03` | |
|   584 | `05 01 c8 00 01` | | `c8` = 200 |
| 585 | | `05 01 c8 00 01` | |
|  | | `fd 04` | |
|   588 | `0c 02 00 02 01 32 1a 01 03 64 00` | `0c 01 00 01 01 22 00 21 00 00 03` | Setting up the dream drive profile. It seems that `02` in bit 3 indicates dream drive; `01` in bit 4 is one chainring, `32` in bit 5 is 50 cogs, `03` is how many gears the left shifter (FD) skips. Unsure about the rest, but the `64` (= 100 decimal) is interesting. Appears to be double the number of gears. |
|   592 | | `0c 02 01 02 01 32 1a 01 03 64 00` | |
|   593 | `0c 00 02` | | |
|   599 | | `0c 01 02 01 01 22 00 21 00 01 03` | |
|   600 | `0c 03 f4 01 4f 02 ab 02 07 03 63 03 bf 03 1b 04 76 04` | | Setting up the dream drive gears. Gears are numbers from 500 (`01 f4`) to 5000 (`13 88`) | 
|  | `0c 03 d2 04 2e 05 8a 05 e6 05 42 06 9d 06 f9 06 55 07 b1 07` | | | 
|  | `0c 03 0d 08 69 08 c4 08 20 09 7c 09 d8 09 34 0a 90 0a eb 0a` | | | 
|  | `0c 03 47 0b a3 0b ff 0b 5b 0c b7 0c 12 0d 6e 0d ca 0d 26 0e` | | | 
|  | `0c 03 82 0e de 0e 39 0f 95 0f f1 0f 4d 10 a9 10 05 11 60 11` | | | 
|  | `0c 03 bc 11 18 12 74 12 d0 12 2c 13 88 13` | | | 
| 611 | `0c 02 0b 02 01 32 1a 01 03 64 00` | | |
| 612 | | `0c 02 0c 02 01 32 1a 01 03 64 00` | |
| 614 | `fd 00` | `0c 01 02 01 01 f4 01 00 00 00 00` | |
| 616 | | `fd 01` | |
| 619 | `03 01 4c 1d 00 00` | `fd 03` | `1d 4c` = 7500|
|  | | `fd 04` | |
|  | | `0c 01 02 01 01 f4 01 00 00 00 00` | |
|  | | `03 01 4c 1d 00 00` | |
|  | | `fd 04` | |
| 620 | `0c 00 02` | | |
| 623 | | `0c 01 02 01 01 f4 01 00 00 00 00` | |
|     | | `0c 01 02 01 01 f4 01 00 00 00 00` | |
|     | | `0c 01 02 01 01 f4 01 00 00 00 00` | |
| 624 | `0c 00 02` | | |
| 627 | | `0c 01 02 01 01 f4 01 00 00 00 00` | |
|  637 | `0c 00 02` | | |
|  640 | | `0c 01 02 01 01 f4 01 00 00 00 00` | |
|  797 | | `0c 01 02 01 02 4f 02 00 00 00 00` | Shift one step down. bit 4 goes to 2. 5 and 6 match the number for the second gear in the `0c 03` setup sequence: `4f 02`|
|  848 | | `0c 01 02 01 03 ab 02 00 00 00 00` | Another step down. Bit 4 goes up to 3. 5 and 6 match  `ab 02`|
|  880 | | `0c 01 02 01 04 07 03 00 00 00 00` | One more.  |
|  987 | | `0c 01 02 01 03 ab 02 00 00 00 00` | Back up |
| 1092 | | `0c 01 02 01 02 4f 02 00 00 00 00` | |
| 1221 | | `0c 01 02 01 01 f4 01 00 00 00 00` | |
| 1343 | | `0c 01 02 01 04 07 03 00 00 00 00` | Shift down 3 steps |
| 1377 | | `0c 01 02 01 07 1b 04 00 00 00 00` | And three more |
| 1430 | | `0c 01 02 01 04 07 03 00 00 00 00` | Thee back up |
| 1468 | | `0c 01 02 01 01 f4 01 00 00 00 00` | Back home |



| Hex Gear | Dec Gear
|----------|----------
| `f4 01` | 500
| `4f 02` | 591
| `ab 02` | 683
| `07 03` | 775
| `63 03` | 867
| `bf 03` | 959
| `1b 04` | 1051
| `76 04` | 1142
| `d2 04` | 1234
| `2e 05` | 1326
| `8a 05` | 1418
| `e6 05` | 1510
| `42 06` | 1602
| `9d 06` | 1693
| `f9 06` | 1785
| `55 07` | 1877
| `b1 07` | 1969
| `0d 08` | 2061
| `69 08` | 2153
| `c4 08` | 2244
| `20 09` | 2336
| `7c 09` | 2428
| `d8 09` | 2520
| `34 0a` | 2612
| `90 0a` | 2704
| `eb 0a` | 2795
| `47 0b` | 2887
| `a3 0b` | 2979
| `ff 0b` | 3071
| `5b 0c` | 3163
| `b7 0c` | 3255
| `12 0d` | 3346
| `6e 0d` | 3438
| `ca 0d` | 3530
| `26 0e` | 3622
| `82 0e` | 3714
| `de 0e` | 3806
| `39 0f` | 3897
| `95 0f` | 3989
| `f1 0f` | 4081
| `4d 10` | 4173
| `a9 10` | 4265
| `05 11` | 4357
| `60 11` | 4448
| `bc 11` | 4540
| `18 12` | 4632
| `74 12` | 4724
| `d0 12` | 4816
| `2c 13` | 4908
| `88 13` | 5000

