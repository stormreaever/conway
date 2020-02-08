#include "Adafruit_NeoTrellisM4.h"

#define CONWAY_INTERVAL 1500

unsigned long currentMillis = 0;
unsigned long previousConwayMillis = 0;

// The NeoTrellisM4 object is a keypad and neopixel strip subclass
// that does things like auto-update the NeoPixels and stuff!
Adafruit_NeoTrellisM4 trellis = Adafruit_NeoTrellisM4();

boolean keystates[32];

int stepColor;

void setup() {
  Serial.begin(115200);
  trellis.begin();
  Serial.println("CONWAY");

  // fill in keystates with random values
  seedConway();
}

void loop() {
  currentMillis = millis();

  // put your main code here, to run repeatedly:
  trellis.tick();

  while (trellis.available()) {
    keypadEvent e = trellis.read();
    if (e.bit.EVENT == KEY_JUST_PRESSED) {
      Serial.print((int)e.bit.KEY);
      Serial.println(" pressed");
      keystates[e.bit.KEY] = !keystates[e.bit.KEY];

      changeColor();
      resetTimer();
    }
  }

  updateConway();
  renderLights();

  delay(10);
}

void renderLights() {
  for (int i = 0; i < 32; i++) {
    trellis.setPixelColor(i, stateColor( keystates[i]) );
  }
}

boolean randomState() {
  return random(0, 2) < 1;
}

int stateColor(int state) {
  if (state) {
    return stepColor;
  }
  return 0x0;
}

int randomColor() {
  return (int)random(0, 0xFFFFFF);
}

void changeColor() {
  stepColor = randomColor();
}

void resetTimer() {
  previousConwayMillis = currentMillis - 1;
}

void updateConway() {
  if (currentMillis - previousConwayMillis >= CONWAY_INTERVAL) {

    if ( gridIsEmpty() ) {
      seedConway();
      changeColor();
    } else {
      stepConway();
    }

    previousConwayMillis += CONWAY_INTERVAL;

  }
}

void stepConway() {

  boolean newkeystates[32]; // this array will be populated by our calculations

  // calculate the new states
  for (int i = 0; i < 32; i++) {
    newkeystates[i] = calculateState(i);
  }

  // copy the new states to the live lights
  memcpy( keystates, newkeystates, sizeof keystates );
}

boolean calculateState(int i) {
  // sum the 9-square around the led
  int sum = 0;

  // get the values of the 9 squares around the led
  // ooo
  // oXo
  // ooo

  // get the x & y coords for this led
  int xLed = i % 8;
  int yLed = floor(i / 8);

  // find the points around it
  for (int x = -1; x <= 1; x++) {
    for (int y = -1; y <= 1; y++) {
      // mod the values so they fall over the edges like pac man
      int xVal = xLed + x;
      if (xVal < 0) {
        xVal = 7;
      }
      if (xVal > 7) {
        xVal = 0;
      }
      int yVal = yLed + y;
      if (yVal < 0) {
        yVal = 3;
      }
      if (yVal > 3) {
        yVal = 0;
      }

      // get the array index from the coord
      int arrayIndex = yVal * 8 + xVal;

      // add the value to the total
      if (keystates[arrayIndex]) {
        sum ++;
      }
    }
  }

  // get the new state based on the sum
  return doRules( keystates[i], sum );
}

boolean doRules(boolean state, int sum) {
  if (sum == 3) {
    return true;
  }
  if (sum == 4) {
    return state;
  }
  return false;
}

// re-seed when at 0
boolean gridIsEmpty() {
  for (int i = 0; i < 32; i++) {
    if (keystates[i]) {
      return false;
    }
  }
  return true;
}

void seedConway() {
  Serial.println("Re-seed!");
  for (int i = 0; i < 32; i++) {
    keystates[i] = randomState();
  }
}
