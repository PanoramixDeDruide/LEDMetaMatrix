const int colPins[32] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 22, 23, 24, 26, 25, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41};
const int rowPins[32] = {42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 19, 18, 17, 16, 15, 14, 20, 21, A12, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10};

const int dipPins[3] = {A13, A14, A15};

int x = 0;
int y = 0;
int oldx = 0;
int oldy = 0;
int oldnumm = 0;
int width = 0;
int height = 0;
int scrNo = 0;
int initialized = 0;
int numm = 0;
unsigned long timeSpent = 0;
char dataz[32][32] = {0};

void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i < 32; i++) {
    pinMode(colPins[i], INPUT);
    pinMode(rowPins[i], INPUT);
  }

  for (int i = 0; i < 3; i++) {
    pinMode(dipPins[i], INPUT_PULLUP);
  }

  scrNo = 0;
  for (int i = 0; i < 3; i++) {
    int tmp = digitalRead(dipPins[i]);
    if (tmp == LOW) {
      scrNo |= 1;
    }
    scrNo <<= 1;
  }
  scrNo >>= 1;
  
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}

int convXCoord(int oldX, int oldY) {
  int block = 8 * (oldX / 8);
  int xWithinBlock = oldX % 8;
  int yWithinBlock = oldY % 8;

  return block + yWithinBlock;
}

int convYCoord(int oldX, int oldY) {
  int block = 8 * (oldY / 8);
  int xWithinBlock = oldX % 8;
  int yWithinBlock = oldY % 8;

  return block + (7 - xWithinBlock);
}

void loop() {
  if (initialized == 4) {
    if (Serial.available()) {
      Serial.write(Serial.read());
    }
    if (dataz[x][y] == 1) {
      pinMode(colPins[x], OUTPUT);
      digitalWrite(colPins[x], LOW);
      pinMode(rowPins[y], OUTPUT);
      digitalWrite(rowPins[y], HIGH);
      timeSpent = micros() + 125;
      initialized = 5;
      oldx = x;
      oldy = y;
    }
    x++;
    if (x == width) {
      x = 0;
      y++;
    }
    if ((y == height) || (y == 32)) {
      y = 0;
      x = 0;
    }
  } else if (initialized == 5) {
    if (Serial.available()) {
      Serial.write(Serial.read());
    }
    if (micros() > timeSpent) {
      pinMode(colPins[oldx], INPUT);
      pinMode(rowPins[oldy], INPUT);
      initialized = 4;   
    }
  } else if (initialized == 3) {
    if (Serial.available()) {
      numm = Serial.peek();
      if (numm == '0') {
        Serial.write(Serial.read());
        x++;
        if (x == width) {
          x = 0;
          y++;
          Serial.println("");
        }
        if (y == height) {
          x = 0;
          y = 0;
        }
      }
    }
    if (micros() > timeSpent) {
      pinMode(colPins[convXCoord(oldx % 32, oldy)], INPUT);
      pinMode(rowPins[convYCoord(oldx % 32, oldy)], INPUT);
      initialized = 2;
    }
  } else if (initialized == 2) {
    if (Serial.available()) {
      numm = Serial.read();
      if ((numm == '0') || (numm == '1')) {
        Serial.write(numm);
      }
      if ((y / 32) == scrNo) {
        if (numm == '1') {
          pinMode(colPins[convXCoord(x, y % 32)], OUTPUT);
          digitalWrite(colPins[convXCoord(x, y % 32)], LOW);
          pinMode(rowPins[convYCoord(x, y % 32)], OUTPUT);
          digitalWrite(rowPins[convYCoord(x, y % 32)], HIGH);
          dataz[convXCoord(x % 32, y)][convYCoord(x, y % 32)] = 1;
          timeSpent = micros() + 250;
          initialized = 3;
          oldnumm = numm;
          oldx = x;
          oldy = y;
        }
      }
      if ((numm == '0') || (numm == '1')) {
        x++;
      }
      if (x == width) {
        x = 0;
        y++;
        Serial.println("");
      }
      if (y == height) {
        x = 0;
        y = 0;
        initialized = 4;
      }
    }
  } else if (initialized == 0) {
    if (Serial.available() > 2) {
      int a = Serial.read();
      Serial.write(a);
      int b = Serial.read();
      Serial.write(b);
      Serial.println("");
      if (a == 'P' && b == '1') {
        initialized = 1;
      }
    }
  } else if (initialized == 1) {
    if (Serial.available() > 5) {
      width = Serial.parseInt();
      height = Serial.parseInt();
      Serial.print(width, DEC);
      Serial.print(" ");
      Serial.print(height, DEC);
      Serial.println("");
      initialized = 2;
    }
  }
}
