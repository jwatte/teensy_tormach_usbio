
/* This sketch runs on Teensy LC.
 *  It drives the same relay (pin 20) with four different duty cycles
 *  based on which output relay is on.
 *  P3 -- 3 seconds every 16 seconds.
 *  P2 -- 2 seconds every 8 seconds
 *  P1 -- 1 second every 4 seconds
 *  P0 -- continually on
 */

#define OUTPUT0 20
#define MSG_PIN 13


#define VERSION_STR "Teensy for Tormach I/O ID=0\n"

//  what to actually set the outputs to
bool outputs[4];
//  what to actually send as inputs
bool inputs[4];

//  de-bounce input
bool tInputs[4];
//  time to use for de-bounce
uint32_t tmInputs[4];

void setup() {
  // put your setup code here, to run once:
  pinMode(OUTPUT0, OUTPUT);
  digitalWrite(OUTPUT0, LOW);
}

char inbuf[256];
uint8_t iptr;
bool led;
uint32_t lastcmdtime;

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
    lastcmdtime = now;
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
          sprintf(buf, "* %d%d%d%d %d%d%d%d\n",
            outputs[0], outputs[1], outputs[2], outputs[3],
            inputs[0], inputs[1], inputs[2], inputs[3]);
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

  //  turn off if the computer goes away and stops sending
  if (uint32_t(now - lastcmdtime) > 5000) {
    memset(outputs, 0, sizeof(outputs));
    lastcmdtime = now;
  }

  //  output relay status
  digitalWrite(OUTPUT0,
    (outputs[3] && ((now & 16383) < 3000)) ||
    (outputs[2] && ((now & 8191) < 2000)) ||
    (outputs[1] && ((now & 4095) < 1000)) ||
    outputs[0]
    );

  delay(1);
}

