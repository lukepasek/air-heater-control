#include <Arduino.h>

// #include <HardwareSerial.h>

#include "color_terminal.h"

#define LED_PIN 2
#define OUT_PIN 18
#define OUT_GND_PIN 19
#define BW_SERIAL_PIN 5
#define BW_SERIAL_IDLE_LEVEL LOW

#define BW_SERLIAL_BPS 250
#define BW_SERLIAL_BITS 12

#define BW_BIT_SAMPLE_DELAY ((500000 / BW_SERLIAL_BPS))
// #define BW_BIT_SAMPLE_DELAY ((250000 / BW_SERLIAL_BPS))

// HardwareSerial SerialBlueWire(1);

hw_timer_t *blueWireSerialTimer = NULL;

volatile int rx_sample_cnt = 0;
volatile uint16_t rx_byte_msbf;
volatile boolean new_rx_char = false;
volatile boolean rx_sample_enable = false;

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
boolean monitor_mode = false;

boolean rx_process();
void print_frame();
void print_byte(int idx, uint16_t value, uint16_t last_value, const char *color);

void led_on(boolean on)
{
  digitalWrite(OUT_PIN, on);
  digitalWrite(LED_PIN, on);
}

void IRAM_ATTR serialPinEdgeISR()
{
  if (digitalRead(BW_SERIAL_PIN))
  {
    timerWrite(blueWireSerialTimer, 0);
    detachInterrupt(BW_SERIAL_PIN);
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

void IRAM_ATTR serialBitTimerISR()
{
  if (rx_sample_enable)
  {
    int rx_pin_state = digitalRead(BW_SERIAL_PIN);
    int rx_sample_ptr = rx_sample_cnt % 6;
    boolean rx_frame_error = false;
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
      rx_sample_enable = false;
      attachInterrupt(BW_SERIAL_PIN, &serialPinEdgeISR, CHANGE);
      new_rx_char = !rx_frame_error;
      return;
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  println_color(COLOR_GREEN, "--==## Diesel Air Heater protocol monitor ##==--");
  println_color(COLOR_GREEN, "Running in monitor mode.");

  pinMode(OUT_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BW_SERIAL_PIN, INPUT);

  blueWireSerialTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(blueWireSerialTimer, &serialBitTimerISR, true);
  timerAlarmWrite(blueWireSerialTimer, BW_BIT_SAMPLE_DELAY, true);
  timerAlarmEnable(blueWireSerialTimer);

  attachInterrupt(BW_SERIAL_PIN, &serialPinEdgeISR, CHANGE);
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
  attachInterrupt(BW_SERIAL_PIN, &serialPinEdgeISR, CHANGE);
}

void loop()
{
  unsigned long now = millis();
  if (now - txrx_ts > 500)
  {
    if (monitor_mode) {
      // if (no_data_cnt++ >= 10)
      // {

      //   Serial.print("Monitor mode: No data detected in ");
      //   Serial.print((now - txrx_ts));
      //   Serial.println("ms!");
      //   no_data_cnt = 0;
      //   // -switching to controller mode ");
      //   // monitor_mode = false;
      // }
      if (rx_frame_ptr)
      {
        Serial.print("@");
        Serial.print(now - txrx_ts);
        Serial.print(":");
        Serial.println(rx_frame_ptr);

        rx_frame_ptr = 0;
      }
    } else {
      uint8_t command;
      if (cmd_cnt%25==5) {
        command = 0xae;
      }
      else
      {
        command = 0xa6;
      }
      send_command(command);
      cmd_cnt++;
      txrx_ts = millis();
      Serial.print("> ");
      rx_frame_buffer[0] = command;
      rx_frame_ptr = 1;
    }
  }

  boolean new_frame = rx_process();
  if (new_frame)
  {
    // txrx_ts = now;
    print_frame();
    rx_frame_ptr = 0;
    no_data_cnt = 0;
  }
}

boolean rx_process()
{
  if (new_rx_char)
  {
    new_rx_char = false;
    txrx_ts = millis();
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
    boolean frame_diff = false;
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
