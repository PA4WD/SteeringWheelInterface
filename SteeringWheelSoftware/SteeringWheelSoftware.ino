#include "Average.h" //https://github.com/MajenkoLibraries/Average

#define OUTPUT_RADIO      PB0
#define INPUT_A           A0
#define INPUT_B           A1

#define ONE_TIME          2250UL
#define ZERO_TIME         1120UL
#define REPEAT_TIME       110000UL

#define KENWOOD_ADDRESS   0xb9
#define KENWOOD_SEEK_DOWN 0x0a
#define KENWOOD_SEEK_UP   0x0b
#define KENWOOD_SOURCE    0x13
#define KENWOOD_VOL_UP    0x14
#define KENWOOD_VOL_DOWN  0x15
#define KENWOOD_MUTE      0x16

#define ANALOG_SAMPLES    10

Average<int> analog_average(ANALOG_SAMPLES);

static inline void on() {
  PORTB |= _BV(OUTPUT_RADIO);
}
static inline void off() {
  PORTB &= ~_BV(OUTPUT_RADIO);
}

static void send_leader() {
  on();
  _delay_us(9000);
  off();
  _delay_us(4500);
}
static void send_1() {
  on();
  _delay_us(560);
  off();
  _delay_us(ONE_TIME - 560);
}
static void send_0() {
  on();
  _delay_us(560);
  off();
  _delay_us(ZERO_TIME - 560);
}

static void send_byte(byte val) {
  for (int y = 0; y < 8; y++) { // Iterate though every bit
    if ( val >> y & 0b00000001 ) {
      send_1();
    } else {
      send_0();
    }
  }
}

static void send_key(byte key) {
  send_leader();
  send_byte(KENWOOD_ADDRESS); //normal
  send_byte(~(KENWOOD_ADDRESS)); //invert
  send_byte(key); //normal
  send_byte(~key); //invert
  send_1(); //end transmission
  _delay_us(REPEAT_TIME - 16 * ZERO_TIME - 16 * ONE_TIME); //pause for next transmission
}

int sampleAnalogInput(int input) {
  for (int i = 0; i < ANALOG_SAMPLES; i++) {
    analog_average.push(analogRead(input));
    _delay_us(100);
  }

  // Display the current data set
  //  for (int i = 0; i < ANALOG_SAMPLES; i++) {
  //    Serial.print(analog_average.get(i));
  //    Serial.print(" ");
  //  }
  //  Serial.println();

  return analog_average.mode();
}



void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(OUTPUT_RADIO, OUTPUT);
  Serial.begin(115200);

  Serial.println(F("Ready to send IR signals at pin "));
}

void loop() {
  int rcA = sampleAnalogInput(INPUT_A);
  int rcB = sampleAnalogInput(INPUT_B);
  //Serial.println(rcA);
  //Serial.println(rcB);

  if (rcA < 1000) {
    //Serial.print("rcA ");
    Serial.println(rcA);
    if (rcA < 50) { //Mode button
      send_key(KENWOOD_SOURCE);
      Serial.println(F("Source"));
      delay(1000);
    }
    if (rcA > 300 && rcA < 400) {
      send_key(KENWOOD_SEEK_UP);
      Serial.println(F("Seek up"));
      delay(1000);
    }
    if (rcA > 650 && rcA < 750) {
      send_key(KENWOOD_VOL_UP);
      Serial.println(F("Volume up"));
      delay(1000);
    }
  }
  if (rcB < 1000) {
    //Serial.print("rcB ");
    Serial.println(rcB);
    if (rcB > 300 && rcB < 400) {
      send_key(KENWOOD_SEEK_DOWN);
      Serial.println(F("Seek down"));
      delay(1000);
    }
    if (rcB > 650 && rcB < 750) {
      send_key(KENWOOD_VOL_DOWN);
      Serial.println(F("Volume down"));
      delay(1000);
    }
  }


  //delay(5000);

  while (Serial.available() > 0) {
    char chr = Serial.read();
    if (chr == '+') {
      send_key(KENWOOD_VOL_UP);
      //Serial.println(F("Volume up"));
    }
    if (chr == '-') {
      send_key(KENWOOD_VOL_DOWN);
      //Serial.println(F("Volume down"));
    }
    if (chr == 's') {
      send_key(KENWOOD_SOURCE);
      //Serial.println(F("source"));
    }
    if (chr == 'u') {
      send_key(KENWOOD_SEEK_UP);
      //Serial.println(F("Seek up"));
    }
    if (chr == 'd') {
      send_key(KENWOOD_SEEK_DOWN);
      //Serial.println(F("Seek down"));
    }
  }
}
