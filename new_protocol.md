

### first response after power up
```
 65 6c |  6c 24 24 24
```

### normal response frame (off)

```
 65 6c |  6c 24 24 6d (14V)
 65 6c |  6c 24 24 65 (11V)
```

### pump prime mode

 |  65 6c |  6c 24 24 6d
 |  65 6c |  6c 24 24 6d
 |  65 6c |  6c 24 24 6d
 |  65 2d |  6c 24 24 6d
 |  65 2d |  6c 2c 24 6d
 |  65 2d |  6c 2c 24 b7
 |  65 6c |  6c 2c 24 6d
 |  65 6c |  6c 2c 24 6d <- pump on


### on request/response frame

```

65 64 |  6c 24 9c |
65 6c |  6c 24 2c 64
```

### normal response while starting/pre heat
6c 25 2c 64

### normal response while starting/pump on
6c 2d 2c 64


### off request frame

```
65 64
```

### normal response  while cool down
```
6c 24 2d 24
```

cool down: 6c 25 2d 24 -> off: 6c 24 24 6d


Mountain mode on:
65 6c |  6c 64 24 6d -> delay -> |  65 6c |  6c 64 24

-----------------------------------

first frame after power on (0V ???)
 65  6c  |  6c  24  24  24  (24:00100100)


off/idle (11V)
65  6c  |  6c  24  24  65  (65:01100101) 1011

off/idle (12V)
65  6c  |  6c  24  24  2d  (2d:00101101)  1100

off/idle (13V)
65  6c  |  6c  24  24  2d

off/idle (14V)
65  6c  |  6c  24  24  6d

on request (on ack)
65  64  |  6c  24  2c  64

startup (preheat)
65  6c  |  6c  25  2c  64

startup (pump on - low)
65  6c  |  6c  2d  2c  64

startup (pump on - high)
65  6c  |  6c  2d  2c  2c

off request while starting (off ack)
65  64  |  6c  2c  2d  24

cool-down
65  6c  |  6c  25  2d  24

off/idle
65  6c  |  6c  24  24  65


mountain mode on
65  6d  |  6c  64  24  2d                 (64:01100100)

mountain mode off
65  6d  |  6c  24  24  2d  (6c:01101100)  (24:00100100)


manual pump on
65  2d  |  6c  24  24  2d
65  6c  |  6c  24  24  2d
65  2d  |  6c  2c  24  2d
65  6c  |  6c  2c  24  2d


prog/temp up
65  2c  |  6c  25  2c  24  (24:00100100)
65  2c  |  6c  25  2c  6c  (6c:01101100)
65  2c  |  6c  25  2c  2c  (2c:00101100)

error 2 (over voltage)
65  6c  |  25  24  2c  e4
65  6c  |  25  24  24  64

error 4 (pump disconnected)
65  6c  |  25  24  24  64  (25:00100101)

error 6 (fan stuck)
65  6c  |  25  24  24  2c

start error/restart
65  6c  |  6c  25  2c  2c

error 8 - cool down (start fail/no fuel)
65  6c  |  25  25  2d  6c
idle with error 8
65  6c  |  25  24  24  6c



 36  26  |  34  24  24  26
 a6  a6  |  a6  24  24  a6
 36  26  |  36  b4  34  36
 a6  a6  |  a6  24  24  a6
 36  26  |  34  24  24  26
 a6  a6  |  a6  24  24  a4
 36  26  |  36  b4  34  36
 a6  a6  |  a6  24  24  a6
 36  26  |  34  24  24  26
 a6  a6  |  a6  24  24  a6
 36  26  |  36  b4  34  36
 a6  a6  |  a6  24  24  a6
 36  26  |  34  24  24  26
 a6  a6  |  a6  24  24  a6
 36  26  |  36  b4  34  36




 glow plug on startup
 a6 36 | 36 a4 34 26 | glow plug

 glow plug on cool-down
 a6 36 | 36 a4 b4 24 | glow plug



10100110 a6 00100110 26 | 00110110 36 10100100 a4 00110100 34 00110100 34 * power toggle glow plug  (10V)


[byte:1] b4 pump on
[byte:1] a4 pump off

[byte:1] 0x10 pump on
[byte:1] a4 pump off

start-up
    376184: 10100110 a6 00110110 36 | 00110110 36 10100100 a4 00110100 34 00100110 26 | on fan/glow plug?
cool-down
    274125: 10100110 a6 00110110 36 | 00110110 36 10100100 a4 10110100 b4 00100100 24 | on fan/glow plug?

error 2
   4120388: 10100110 a6 00110110 36 | 10100100 a4 00100100 24 00100100 24 00100110 26 |

11V
    426215: 10100110 a6 00110110 36 | 00110110 36 00100100 24 00100100 24 10100110 a6 | off
12V
   4103374: 10100110 a6 00110110 36 | 00110110 36 00100100 24 00100100 24 10110100 b4 | off
13V
   4111380: 10100110 a6 00110110 36 | 00110110 36 00100100 24 00100100 24 10110110 b6 | off
14V
   4119387: 10100110 a6 00110110 36 | 00110110 36 00100100 24 00100100 24 00100100 24 | on
