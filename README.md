# Diesel air heater control

<img src="https://github.com/lukepasek/diesel-air-heater-control/assets/6756387/f79806ff-4568-4286-bd64-a9b7440f8464" width="50%">

## New controller protocol

<img src="https://github.com/lukepasek/diesel-air-heater-control/assets/6756387/4ec178e6-bfd3-4cf4-b9d5-ae88bbd995fa" width="30%">

## Sample of protocol decode

```
    8164: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
     998: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
    1000: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
    1001: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
    1001: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
    1001: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
    1000: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
    1001: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
    1001: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
    1000: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
    1001: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
    1000: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
    1001: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
    1002: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
    1000: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
    1001: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
    1001: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
     560: 0: 10100010 0xa2 | 0: 01100000 0x60 1: 01000010 0x42 | on/off toggle | on, H3
     440: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 01000010 0x42 | manual mode | on, H3
    1001: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 01000010 0x42 | manual mode | on, H3
    1002: 0: 10100110 0xa6 | 0: 01101000 0x68 1: 01000010 0x42 | manual mode | on, fan, H3
    1001: 0: 10100110 0xa6 | 0: 01101000 0x68 1: 01000010 0x42 | manual mode | on, fan, H3
    1000: 0: 10100110 0xa6 | 0: 01101000 0x68 1: 01000010 0x42 | manual mode | on, fan, H3
    1001: 0: 10100110 0xa6 | 0: 01101000 0x68 1: 01000010 0x42 | manual mode | on, fan, H3
    1001: 0: 10100110 0xa6 | 0: 01101000 0x68 1: 01000010 0x42 | manual mode | on, fan, H3
    1001: 0: 10100110 0xa6 | 0: 01101000 0x68 1: 01000010 0x42 | manual mode | on, fan, H3
    1000: 0: 10100110 0xa6 | 0: 01101000 0x68 1: 01000010 0x42 | manual mode | on, fan, H3
    1001: 0: 10100110 0xa6 | 0: 01101000 0x68 1: 01000010 0x42 | manual mode | on, fan, H3
    1000: 0: 10100110 0xa6 | 0: 01101001 0x69 1: 01000010 0x42 | manual mode | on, glow plug, fan, H3
    1002: 0: 10100110 0xa6 | 0: 01101001 0x69 1: 01000010 0x42 | manual mode | on, glow plug, fan, H3
    1001: 0: 10100110 0xa6 | 0: 01101001 0x69 1: 01000010 0x42 | manual mode | on, glow plug, fan, H3
    1000: 0: 10100110 0xa6 | 0: 01101001 0x69 1: 01000010 0x42 | manual mode | on, glow plug, fan, H3
    1001: 0: 10100110 0xa6 | 0: 01101001 0x69 1: 01000010 0x42 | manual mode | on, glow plug, fan, H3
    1001: 0: 10100110 0xa6 | 0: 01101001 0x69 1: 01000010 0x42 | manual mode | on, glow plug, fan, H3
    1001: 0: 10100110 0xa6 | 0: 01101001 0x69 1: 01000010 0x42 | manual mode | on, glow plug, fan, H3
    1000: 0: 10100110 0xa6 | 0: 01101001 0x69 1: 01000010 0x42 | manual mode | on, glow plug, fan, H3
    1000: 0: 10100110 0xa6 | 0: 01101001 0x69 1: 01000010 0x42 | manual mode | on, glow plug, fan, H3
     986: 0: 10100010 0xa2 | 0: 01100001 0x61 1: 11000000 0xc0 | on/off toggle | shutdown, glow plug, H1
     351: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
     665: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
    1001: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
```
