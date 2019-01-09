
//  define ON_ARDUINO to 1 to make work with Arduino Uno
#define ON_ARDUINO 0
#if !ON_ARDUINO
#define USE_TOUCH 1
#define TOUCH_DELTA 300
#endif

#if ON_ARDUINO
#define INPUT0 A0
#define INPUT1 A1
#define INPUT2 A2
#define INPUT3 A3

#define OUTPUT0 2
#define OUTPUT1 3
#define OUTPUT2 4
#define OUTPUT3 5

#define MSG_PIN 13
#else
#define INPUT0 15
#define INPUT1 16
#define INPUT2 17
#define INPUT3 19

#define OUTPUT0 20
#define OUTPUT1 21
#define OUTPUT2 22
#define OUTPUT3 23

#define MSG_PIN 13
#endif


#define VERSION_STR "Teensy for Tormach I/O ID=0\n"
#define DEBOUNCE_MS 10

//  what to actually set the outputs to
bool outputs[4];
//  what to actually send as inputs
bool inputs[4];

//  de-bounce input
bool tInputs[4];
//  time to use for de-bounce
uint32_t tmInputs[4];

#if USE_TOUCH
class TouchPin {
  public:
  TouchPin(uint8_t pin) : pin_(pin), on_(false), low_(800), high_(1300), last_(500) {
  }
  bool read() {
    uint16_t rd = touchRead(pin_);
    last_ = rd;
    if (on_) {
      if (rd < high_) {
        on_ = false;
      } else {
        high_ = (rd > TOUCH_DELTA) ? rd - TOUCH_DELTA : 0;
        if (high_ < low_ + TOUCH_DELTA) {
          high_ = low_ + TOUCH_DELTA;
        }
        if (high_ > low_ * 4) {
          high_ = low_ * 4;
        }
      }
    } else {
      if (rd > low_ + TOUCH_DELTA) {
        on_ = true;
      } else {
        low_ = rd;
        if (low_ > high_ - TOUCH_DELTA) {
          low_ = high_ - TOUCH_DELTA;
        }
        if (low_ > high_ / 2) {
          low_ = high_ / 2;
        }
      }
    }
    return on_;
  }
  char const *fmt() {
    sprintf(buf_, "%d/%d/%d", low_, high_, last_);
    return buf_;
  }
  char buf_[24];
  uint8_t pin_;
  bool on_;
  uint16_t low_;
  uint16_t high_;
  uint16_t last_;
};

TouchPin touchPin0(INPUT0);
TouchPin touchPin1(INPUT1);
TouchPin touchPin2(INPUT2);
TouchPin touchPin3(INPUT3);
#endif

void setup() {
#if ON_ARDUINO
  Serial.begin(38400);
#endif
  // put your setup code here, to run once:
  pinMode(OUTPUT0, OUTPUT);
  digitalWrite(OUTPUT0, LOW);
  pinMode(OUTPUT1, OUTPUT);
  digitalWrite(OUTPUT1, LOW);
  pinMode(OUTPUT2, OUTPUT);
  digitalWrite(OUTPUT2, LOW);
  pinMode(OUTPUT3, OUTPUT);
  digitalWrite(OUTPUT3, LOW);
  pinMode(MSG_PIN, OUTPUT);
  digitalWrite(MSG_PIN, LOW);

  pinMode(INPUT0, INPUT);
  pinMode(INPUT1, INPUT);
  pinMode(INPUT2, INPUT);
  pinMode(INPUT3, INPUT);
}

char inbuf[256];
uint8_t iptr;
bool led;

void toggleLed() {
  led = !led;
  digitalWrite(MSG_PIN, led ? HIGH : LOW);
}

void loop() {
  uint32_t now = millis();
  bool gotCmd = false;

  int n = Serial.available();
  //  Parse whatever we have -- which may be partial lines, so keep buffer
  while (n > 0) {
    --n;
    gotCmd = true;
    int ch = Serial.read();
    if (ch == '\r' || ch == '\n') {
      if (iptr > 0) {
        if (inbuf[0] == 'V' && inbuf[1] == 'E') {
          Serial.write(VERSION_STR);
          toggleLed();
        } else if (inbuf[0] == 'S' && inbuf[1] == 'R' && inbuf[2] == ' ') {
          if (iptr < 7) {
            //  bad CMD!
          } else {
            outputs[0] = (inbuf[3] == '1');
            outputs[1] = (inbuf[4] == '1');
            outputs[2] = (inbuf[5] == '1');
            outputs[3] = (inbuf[6] == '1');
            char msg[5] = {
              inputs[0] ? '1' : '0',
              inputs[1] ? '1' : '0',
              inputs[2] ? '1' : '0',
              inputs[3] ? '1' : '0',
              '\n'
            };
            Serial.write(msg, 5);
            toggleLed();
          }
        } else if (inbuf[0] == '?') {
          inbuf[iptr] = 0;
          char buf[120];
#if ON_ARDUINO
          sprintf(buf, "* %d%d%d%d %d%d%d%d\n",
            outputs[0], outputs[1], outputs[2], outputs[3],
            inputs[0], inputs[1], inputs[2], inputs[3]);
#else
          sprintf(buf, "* %s %s %s %s %d%d%d%d\n",
            touchPin0.fmt(), touchPin1.fmt(), touchPin2.fmt(), touchPin3.fmt(),
            inputs[0], inputs[1], inputs[2], inputs[3]);
#endif
          Serial.write(buf);
        } else {
          //  bad CMD!!
        }
      }
      memset(inbuf, 0, iptr);
      iptr = 0;
    } else  {
      if (iptr == sizeof(inbuf)-1) {
        //  OVERFLOW!
        iptr = 0;
      } else {
        inbuf[iptr++] = ch;
      }
    }
  }

#if USE_TOUCH
  tInputs[0] = touchPin0.read();
  tInputs[1] = touchPin1.read();
  tInputs[2] = touchPin2.read();
  tInputs[3] = touchPin3.read();
#else
  tInputs[0] = digitalRead(INPUT0);
  tInputs[1] = digitalRead(INPUT1);
  tInputs[2] = digitalRead(INPUT2);
  tInputs[3] = digitalRead(INPUT3);
#endif

  //  debounce input pins
  for (int i = 0; i != 4; ++i) {
    if (tInputs[i] == inputs[i]) {
      tmInputs[i] = now;
    }
    if (now - tmInputs[i] > DEBOUNCE_MS) {
      inputs[i] = tInputs[i];
    }
  }
  
  //  output relay status
  digitalWrite(OUTPUT0, outputs[0]);
  digitalWrite(OUTPUT1, outputs[1]);
  digitalWrite(OUTPUT2, outputs[2]);
  digitalWrite(OUTPUT3, outputs[3]);

#if !USE_TOUCH
  //  Touch reading takes time, but if not using touch, no need to be too fast
  if (!gotCmd) {
    delay(1);
  }
#else
  (void)&gotCmd;
#endif
}

