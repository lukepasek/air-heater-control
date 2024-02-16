# Diesel air heater control

<img src="https://github.com/lukepasek/diesel-air-heater-control/assets/6756387/f79806ff-4568-4286-bd64-a9b7440f8464" width="50%">

## New controller protocol

<img src="https://github.com/lukepasek/diesel-air-heater-control/assets/6756387/4ec178e6-bfd3-4cf4-b9d5-ae88bbd995fa" width="30%">

The controller has blue symbols/numbers.

<img src="https://github.com/lukepasek/diesel-air-heater-control/assets/6756387/60e005c2-2406-4097-aebb-99cc43b83b85" width="30%">

This new controller (and matching main unit in the heater) communicate using a different protocol to the older heater/controller versions.

## Sample of protocol decode

Come data grames already analised and decoded.

```
    8164: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
     998: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
    [ duplicates removed ]
     560: 0: 10100010 0xa2 | 0: 01100000 0x60 1: 01000010 0x42 | on/off toggle | on, H3
     440: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 01000010 0x42 | manual mode | on, H3
    1001: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 01000010 0x42 | manual mode | on, H3
    1002: 0: 10100110 0xa6 | 0: 01101000 0x68 1: 01000010 0x42 | manual mode | on, fan, H3
    1001: 0: 10100110 0xa6 | 0: 01101000 0x68 1: 01000010 0x42 | manual mode | on, fan, H3
    [ duplicates removed ]
    1000: 0: 10100110 0xa6 | 0: 01101001 0x69 1: 01000010 0x42 | manual mode | on, glow plug, fan, H3
    1002: 0: 10100110 0xa6 | 0: 01101001 0x69 1: 01000010 0x42 | manual mode | on, glow plug, fan, H3
    [ duplicates removed ]
     986: 0: 10100010 0xa2 | 0: 01100001 0x61 1: 11000000 0xc0 | on/off toggle | shutdown, glow plug, H1
     351: 0: 10100110 0xa6 | 0: 01100000 0x60 1: 00001011 0x0b | manual mode | idle 11V
```
