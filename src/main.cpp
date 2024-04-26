#include <Arduino.h>
#include "main.h"
#include "color_terminal.h"

#define BW_SERLIAL_BPS 250

volatile int rx_sample_cnt = 0;
volatile uint16_t rx_byte_msbf;
volatile bool new_rx_char = false;
volatile bool rx_sample_enable = false;

volatile int bitTimerCount = 0;

#ifdef ESP32
  #define BW_BIT_SAMPLE_DELAY ((500000 / BW_SERLIAL_BPS))
  #define LED_PIN 8 // C3 super mini - led on pin 8
  #define OUT_PIN 3
  #define BW_SERIAL_PIN 2

  hw_timer_t *SerialBitTimer = NULL;

  void inline initSerialBitTimer() {
    SerialBitTimer = timerBegin(0, 80, true);
    timerAttachInterrupt(SerialBitTimer, &serialBitTimerISR, true);
    timerAlarmWrite(SerialBitTimer, BW_BIT_SAMPLE_DELAY, true);
    timerAlarmEnable(SerialBitTimer);
  }

  void inline restartSerialBitTimer() {
    timerWrite(SerialBitTimer, 0);
  }

  void inline attachSerialPinInterrupt() {
    attachInterrupt(BW_SERIAL_PIN, &serialPinEdgeISR, CHANGE);
  }

  void inline detachSerialPinInterrupt() {
    detachInterrupt(BW_SERIAL_PIN);
  }
#endif

#ifdef NRF52840_PROMICRO
  #define BW_BIT_SAMPLE_DELAY ((500000 / BW_SERLIAL_BPS))
  #define TIMER_INTERRUPT_DEBUG         0
  #define _TIMERINTERRUPT_LOGLEVEL_     3

  #include <Adafruit_TinyUSB.h>
  #include "NRF52TimerInterrupt.h"

  #define LED_PIN LED_BUILTIN // P0.15
  #define OUT_PIN (10) // P0.10
  #define BW_SERIAL_PIN (9) // P0.09

  NRF52Timer ITimer0(NRF_TIMER_4);

  void initSerialBitTimer() {
  }

  void inline stopSerialBitTimer() {
    ITimer0.detachInterrupt();
  }

  void inline restartSerialBitTimer() {
    ITimer0.attachInterruptInterval(BW_BIT_SAMPLE_DELAY, serialBitTimerHandler);
  }

  void inline attachSerialPinEdgeInterrupt() {
    attachInterrupt(BW_SERIAL_PIN, serialPinEdgeHandler, CHANGE);
  }

  void inline detachSerialPinEdgeInterrupt() {
    detachInterrupt(BW_SERIAL_PIN);
  }
#endif

uint8_t rx_frame_buffer[6];
int rx_frame_ptr = 0;
uint8_t tx_frame;

uint16_t last_frame_buffer[8];
int last_frame_ptr = 0;

unsigned long txrx_ts = 0;
unsigned long last_txrx_ts = 0;
unsigned long last_frame_ts = 0;

long cmd_cnt = 0;
int no_data_cnt = 0;
bool monitor_mode = false;

bool rx_process();
void print_frame();
void print_byte(int idx, uint16_t value, uint16_t last_value, const char *color);

void led_on(bool on)
{
  digitalWrite(OUT_PIN, on);
  digitalWrite(LED_PIN, !on);
}

#ifdef ESP32
void IRAM_ATTR serialPinEdgeISR()
{
  serialPinEdgeHandler();
}  

void IRAM_ATTR serialBitTimerISR() 
{
  serialBitTimerHandler();
}
#endif

void serialPinEdgeHandler() {
  if (digitalRead(BW_SERIAL_PIN))
  {
    restartSerialBitTimer();
    detachSerialPinEdgeInterrupt();
    // timerWrite(SerialBitTimer, 0);
    // detachInterrupt(BW_SERIAL_PIN);
    rx_sample_cnt = 0;
    rx_byte_msbf = 0;
    rx_sample_enable = true;
    // led_on(HIGH);
  }
  else
  {
    // if (millis() - txrx_ts >= 100) {
    //   rx_frame_ptr = 0;
    // }
  }
}

void serialBitTimerHandler() {
  int rx_pin_state = digitalRead(BW_SERIAL_PIN);
  int rx_sample_ptr = rx_sample_cnt % 6;
  bool rx_frame_error = false;
  rx_sample_cnt++;

  // led_on(rx_sample_cnt & 1);

  led_on(HIGH);

  if (rx_sample_ptr == 0)
  {
    rx_frame_error = rx_pin_state != 1;
  }
  else if (rx_sample_ptr == 2)
  {
    rx_byte_msbf = (rx_byte_msbf << 1) | rx_pin_state;
  }
  else if (rx_sample_ptr == 4)
  {
    rx_frame_error = rx_pin_state != 0;
  }
  else
  {
    led_on(LOW);
  }

  if (rx_frame_error || rx_sample_cnt >= ((6 * 8) - 2))
  {
    led_on(LOW);
    stopSerialBitTimer();
    attachSerialPinEdgeInterrupt();
    new_rx_char = !rx_frame_error;      
  }
}

// ----> setup
void setup()
{
  Serial.begin(115200);

  #ifdef NRF52840_PROMICRO
    while (!Serial && millis() < 5000);
    Serial.println("Running on NRF52840_PROMICRO!");
    Serial.print("Led pin: ");
    Serial.println(LED_PIN);
    Serial.print("Heater serial interface IO pin: ");
    Serial.println(BW_SERIAL_PIN);
  #endif

  pinMode(OUT_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BW_SERIAL_PIN, INPUT_PULLUP);

  initSerialBitTimer();

  // delay(3000);
  for (int i = 0; i < 6; i++)
  {
    led_on(HIGH);
    delay(100);
    led_on(LOW);
    delay(100);
  }

  Serial.println();
  println_color(COLOR_GREEN, "--==## Diesel Air Heater protocol controller and monitor ##==--");
  if (monitor_mode) {
    println_color(COLOR_GREEN, "Running in monitor mode.");
  } else {
    println_color(COLOR_YELLOW, "Running in controller mode.");
  }

  restartSerialBitTimer();
  // delay(100000);

  attachSerialPinEdgeInterrupt();
}

void send_command(uint8_t command)
{
  detachInterrupt(BW_SERIAL_PIN);
  pinMode(BW_SERIAL_PIN, OUTPUT);
  digitalWrite(BW_SERIAL_PIN, LOW);
  delay(30);
  for (int i = 7; i >= 0; i--)
  {
    digitalWrite(BW_SERIAL_PIN, HIGH);
    delay(4);
    digitalWrite(BW_SERIAL_PIN, (command >> i) & 0x01);
    delay(4);
    digitalWrite(BW_SERIAL_PIN, LOW);
    delay(4);
  }
  digitalWrite(BW_SERIAL_PIN, HIGH);
  pinMode(BW_SERIAL_PIN, INPUT);

  attachSerialPinEdgeInterrupt();
}

// ----> loop
unsigned long rx_timeout = 0;
int no_rx_cnt = 0;
void loop()
{
  // testSerialBitTimer();
//   Serial.println(bitTimerCount);
//   delay(1000);
// }

// void none() {
  unsigned long now = millis();

  // Serial.print(".");

  // led_on(now % 100 == 0);
  if ((now - rx_timeout) >= 700)
  {
    rx_timeout = now;
    // led_on(LOW);
    if (monitor_mode)
    {
      if ((now - txrx_ts) > 700)
      {
        Serial.print(".");
        if (no_rx_cnt++ >= 20)
        {
          no_rx_cnt = 0;
          Serial.println();
          Serial.print("Monitor mode: No data detected in ");
          Serial.print((now - txrx_ts) / 1000);
          Serial.println("s!");
          Serial.println("Switching to controller mode.");
          monitor_mode = false;
        }
      }

      if (rx_frame_ptr)
      {
        Serial.print("mm@");
        Serial.print(now - txrx_ts);
        Serial.print(":");
        Serial.println(rx_frame_ptr);

        rx_frame_ptr = 0;
      }
    }
    else
    {
      if (digitalRead(BW_SERIAL_PIN))
      {
        uint8_t command;
        if (cmd_cnt % 25 == 5)
        {
          command = 0xae;
        }
        else
        {
          command = 0xa6;
        }
        Serial.print("cm@");
        Serial.print(millis());
        Serial.print(" control frame:");
        Serial.printf(" 0x%02x ", command);
        led_on(HIGH);
        send_command(command);
        led_on(LOW);

        cmd_cnt++;
        txrx_ts = millis();
        Serial.print("> ");
        rx_frame_buffer[0] = command;
        rx_frame_ptr = 1;
      }
    }
  }

  bool new_frame = rx_process();
  if (new_frame)
  {
    // txrx_ts = now;
    print_frame();
    rx_frame_ptr = 0;
    no_data_cnt = 0;
  }
}

bool rx_process()
{
  if (new_rx_char)
  {
    new_rx_char = false;
    txrx_ts = millis();
    // Serial.printf(" 0x%02x\n", rx_byte_msbf);
    rx_frame_buffer[rx_frame_ptr++] = rx_byte_msbf;
    if (rx_frame_ptr >= 3)
    {
      return true;
    }
  }
  return false;
}

void print_byte(int idx, uint16_t value, uint16_t last_value, const char *color)
{
  text_color(color);
  Serial.printf("%d: ", idx);
  for (int b = 7; b >= 0; b--)
  {
    int bv = (value >> b) & 0x01;
    int lbv = (last_value >> b) & 0x01;
    if (bv != lbv)
    {
      text_color("\033[47m");
      Serial.print(bv);
      text_color(color);
    }
    else
    {
      text_color(color);
      Serial.print(bv);
    }
  }
  text_color(color);
  Serial.printf(" 0x%02x ", value);
  text_color(COLOR_NONE);
}

int duplicate_frame_cnt = 0;
void print_frame()
{
  if (rx_frame_ptr == 3)
  {
    bool frame_diff = false;
    unsigned long now = millis();
    // if (frame_ptr != last_frame_ptr)
    // {
    //   // frame_diff = true;
    //   Serial.println("Historic frame length mismatch!");
    // }
    // else
    // {
    for (int i = 0; i < rx_frame_ptr; i++)
    {
      if (rx_frame_buffer[i] != last_frame_buffer[i])
      {
        frame_diff = true;
        break;
      }
    }
    // }

    if (!frame_diff)
    {
      last_frame_ptr = rx_frame_ptr;
      for (int i = 0; i < rx_frame_ptr; i++)
      {
        last_frame_buffer[i] = rx_frame_buffer[i];
      }
      // return;
    }

    Serial.printf("%8lu: ", (now - last_frame_ts));
    last_frame_ts = now;

    for (int i = 0; i < 1; i++)
    {
      print_byte(i, rx_frame_buffer[i], last_frame_buffer[i], frame_diff ? COLOR_BLUE : COLOR_NONE);
    }
    Serial.print("| ");
    for (int i = 1; i < rx_frame_ptr; i++)
    {
      print_byte(i - 1, rx_frame_buffer[i], last_frame_buffer[i], frame_diff ? COLOR_BROWN : COLOR_NONE);
    }

    Serial.print("| ");

    last_frame_ptr = rx_frame_ptr;
    for (int i = 0; i < rx_frame_ptr; i++)
    {
      last_frame_buffer[i] = rx_frame_buffer[i];
    }

    // uint8_t reply[2];
    uint8_t query = rx_frame_buffer[0];
    uint8_t flags0 = rx_frame_buffer[1];
    uint8_t flags1 = rx_frame_buffer[2] >> 6;
    uint8_t value = rx_frame_buffer[2] & 0x3f;

    switch (query)
    {
    case 0x70:
      text_color(COLOR_PURPLE);
      Serial.print("auto. mode ");
      break;
    case 0x71:
      text_color(COLOR_PURPLE);
      Serial.print("mode change 1");
      break;
    case 0xa1:
      text_color(COLOR_PURPLE);
      Serial.print("mode change 2");
      break;
    case 0xa2:
      text_color(COLOR_PURPLE);
      Serial.print("on/off toggle ");
      break;
    case 0xa3:
      text_color(COLOR_PURPLE);
      Serial.print("key up ");
      break;
    case 0xa4:
      text_color(COLOR_PURPLE);
      Serial.print("key down ");
      break;
    case 0xa6:
      text_color(COLOR_PURPLE);
      Serial.print("manual mode ");
      break;
    case 0xab:
      text_color(COLOR_PURPLE);
      Serial.print("get voltage ");
      break;
    case 0xae:
      text_color(COLOR_PURPLE);
      Serial.print("high alt. toggle ");
      break;
    case 0xaf:
      text_color(COLOR_PURPLE);
      Serial.print("get int. temp. ");
      break;
    }
    text_color(COLOR_NONE);

    Serial.print("| ");

    if ((flags0 & 0xf0) == 0x60)
    {
      if (flags0 & 0x02)
      {
        text_color(COLOR_CYAN);
        Serial.print("high alt., ");
        text_color(COLOR_NONE);
      }

      if (flags1 & 0x01)
      {
        if (flags1 & 0x02)
        {
          text_color(COLOR_CYAN);
          Serial.print("shutdown, ");
          text_color(COLOR_NONE);
        }
        else
        {
          text_color(COLOR_CYAN);
          Serial.print("on, ");
          text_color(COLOR_NONE);
        }

        if (flags0 & 0x01)
        {
          text_color(COLOR_CYAN);
          Serial.print("glow plug, ");
          text_color(COLOR_NONE);
        }
        if (flags0 & 0x04)
        {
          text_color(COLOR_CYAN);
          Serial.print("pump, ");
          text_color(COLOR_NONE);
        }
        if (flags0 & 0x08)
        {
          text_color(COLOR_CYAN);
          Serial.print("fan, ");
          text_color(COLOR_NONE);
        }
        text_color(COLOR_CYAN);
        if (value < 6)
        {
          Serial.print("H");
          Serial.print(value + 1);
        }
        else
        {
          Serial.print("automatic mode");
        }
        Serial.print(" ");
        text_color(COLOR_NONE);
      }
      else
      {
        text_color(COLOR_CYAN);
        Serial.print("idle ");
        Serial.print(value);
        Serial.print("V ");
        text_color(COLOR_NONE);
      }
    }
    else if ((flags0 & 0xf0) == 0x80)
    {
      text_color(COLOR_RED);
      Serial.print(" error ");
      Serial.print(value + 1);
      text_color(COLOR_NONE);
    }
    else if ((flags0 & 0xf0) == 0x50)
    {
      text_color(COLOR_GREEN);
      Serial.print("automatic mode report: ");
      Serial.print(value);
      text_color(COLOR_NONE);
    }
    else if ((flags0 & 0xf0) == 0x40)
    {
      text_color(COLOR_GREEN);
      Serial.print(" internal temp. ");
      Serial.print(value);
      text_color(COLOR_NONE);
    }
    else if ((flags0 & 0xf0) == 0xa0)
    {
      text_color(COLOR_GREEN);
      Serial.print(" voltage ");
      Serial.print(value + 1);
      text_color(COLOR_NONE);
    }
    else
    {
      text_color(COLOR_BROWN);
      Serial.print(" unknown report ");
      Serial.print("value: ");
      Serial.print(value);
      text_color(COLOR_NONE);
    }

    // text_color(COLOR_BLUE);
    // if (query[1] == 0x0da)
    // {
    //   text_color(COLOR_NONE);
    //   Serial.print("ping > ");
    // }
    // else if (query[1] == 0x09a)
    // {
    //   Serial.print("on/off toggle > ");
    // }
    // else if (query[1] == 0x2da)
    // {
    //   Serial.print("high alt. toggle > ");
    // }
    // else if (query[1] == 0x2d2)
    // {
    //   Serial.print("prime pump > ");
    // }
    // else if (query[1] == 0x0d2)
    // {
    //   Serial.print("key down > ");
    // }
    // else if (query[1] == 0x09b)
    // {
    //   Serial.print("key up > ");
    // }
    // else if (query[1] == 0x2db)
    // {
    //   Serial.print("internal temp. query > ");
    // }
    // else if (query[1] == 0x29b)
    // {
    //   Serial.print("voltage query > ");
    // }
    // else
    // {
    //   text_color(COLOR_LIGT_PURPLE);
    //   Serial.print("unknown query > ");
    // }
    text_color(COLOR_NONE);

    // }
    // else if (query[0] == 0x0db)
    // {
    //   text_color(COLOR_BLUE);
    //   Serial.print("thermostat mode > ");
    //   text_color(COLOR_NONE);
    // }
    // else
    // {
    //   text_color(COLOR_LIGT_RED);
    //   Serial.print("unknow control > ");
    //   text_color(COLOR_NONE);
    // }

    // if (reply[0] == 0x292)
    // {
    //   text_color(COLOR_LIGT_RED);
    //   Serial.print(" error ");
    //   text_color(COLOR_NONE);
    // }

    // if (reply[0] == 0x0da || reply[0] == 0x292)
    // {

    //   if (reply[2] == 0x092)
    //   {
    //     Serial.print("idle ");
    //     switch (reply[3])
    //     {
    //     case 0x29b:
    //       Serial.print("11V ");
    //       break;
    //     case 0x2d2:
    //       Serial.print("12V ");
    //       break;
    //     case 0x2d3:
    //       Serial.print("13V ");
    //       break;
    //     case 0x2da:
    //       Serial.print("14V ");
    //       break;
    //     case 0x2db:
    //       Serial.print("15V ");
    //       break;
    //     }
    //   }

    //   if (reply[2] & 0x040)
    //   {
    //     Serial.print("on/startup ");
    //   }

    //   if (reply[1] & 0x200)
    //   {
    //     Serial.print("fan ");
    //   }

    //   if (reply[1] & 0x008)
    //   {
    //     Serial.print("high alt. ");
    //   }
    //   if (reply[1] & 0x040)
    //   {
    //     Serial.print("pump ");
    //   }

    //   if (reply[1] & 0x001)
    //   {
    //     Serial.print("glow plug ");
    //   }

    //   if (reply[3] == 0x092)
    //   {
    //     Serial.print("H-1 ");
    //   }
    //   if (reply[3] == 0x093)
    //   {
    //     Serial.print("H-2 ");
    //   }
    //   if (reply[3] == 0x09a)
    //   {
    //     Serial.print("H-3 ");
    //   }
    //   if (reply[3] == 0x09B)
    //   {
    //     Serial.print("H-4 ");
    //   }
    //   if (reply[3] == 0x0d2)
    //   {
    //     Serial.print("H-5 ");
    //   }
    //   if (reply[3] == 0x0d3)
    //   {
    //     Serial.print("H-6 ");
    //   }
    // }
    // else if (reply[0] == 0x0d3)
    // {
    //   text_color(COLOR_CYAN);
    //   Serial.print(" thermostat report");
    //   text_color(COLOR_NONE);
    // }
    // else if (reply[0] == 0x0d2)
    // {
    //   text_color(COLOR_CYAN);
    //   Serial.print(" internal temp. report");
    //   text_color(COLOR_NONE);
    // }
    // else if (reply[0] == 0x29a)
    // {
    //   text_color(COLOR_CYAN);
    //   Serial.print(" voltage report");
    //   text_color(COLOR_NONE);
    // }
    // else
    // {
    //   text_color(COLOR_LIGT_PURPLE);
    //   Serial.print(" unknown report");
    //   text_color(COLOR_NONE);
    // }

    Serial.println();
  }
  else
  {
    Serial.printf("Invalid frame length: %d\n", rx_frame_ptr);
  }
}
