#include <Servo.h>
#include "pitches.h"  // your melody pitches

/* ---------- Pins ---------- */
const int LDR_PIN   = A0;   
const int BTN_PIN   = 2;   
const int SERVO_PIN = 9;   
const int BUZZER_PIN = 8; // new pin for buzzer

/* ---------- Servo tuning ---------- */
const int SERVO_REST    = 25;  
const int SERVO_HIT_MIN = 100; 
const int SERVO_HIT_MAX = 180; 

/* ---------- Timing ---------- */
const unsigned long STRIKE_DOWN_MS_MIN = 55;  
const unsigned long STRIKE_DOWN_MS_MAX = 90;  
const unsigned long RETURN_MS          = 100;  
const unsigned long COOLDOWN_MS        = 3000;  
const unsigned long DEBOUNCE_MS        = 60;
const unsigned long CAL_TIME_MS        = 1500;  
const unsigned long AUTO_ARM_MS        = 3000;  

/* ---------- LDR smoothing / trigger ---------- */
const float ALPHA = 0.12f;  
const int   MIN_DROP = 40;  
const int   MAX_DROP = 260; 

/* ---------- State ---------- */
Servo beater;
bool armed = false;
bool sawAnyButton = false;
int  btnPrev = HIGH;
unsigned long lastBtnTime  = 0;
unsigned long lastStrike   = 0;
unsigned long bootTime     = 0;
float ema = 0;      
float baseline = 0; 

/* ---------- Melody ---------- */
int melody[] = {
  NOTE_D3, REST, NOTE_D3, REST, NOTE_D3, REST, NOTE_D3, NOTE_D3, NOTE_D3, NOTE_A2, REST,
  NOTE_D3, REST, NOTE_D3, REST, NOTE_D3, REST, NOTE_D3, NOTE_D3, NOTE_D3, NOTE_A2, REST,
  NOTE_D3, REST, NOTE_D3, REST, NOTE_D3, REST, NOTE_D3, NOTE_D3, NOTE_D3, NOTE_A2, REST,
  NOTE_D3,
  NOTE_D4, REST, NOTE_D4, NOTE_D4, REST,
  NOTE_E3, NOTE_D3, NOTE_F3, REST, NOTE_F3,
  NOTE_D4, REST, NOTE_D4, NOTE_D4, REST,
  NOTE_D3,
  NOTE_D4, REST, NOTE_D4, NOTE_D4, REST,
  NOTE_E3, NOTE_D3, NOTE_F3, REST, NOTE_F3,
  NOTE_D4,
  REST
};

int durations[] = {
  8, 16, 8, 16, 8, 16, 10, 10, 10, 3, 2,
  8, 16, 8, 16, 8, 16, 10, 10, 10, 3, 2,
  8, 16, 8, 16, 8, 16, 10, 10, 10, 3, 2,
  4,
  4, 3, 4, 4, 3,
  6, 6, 6, 33, 6,
  4, 3, 4, 4, 3,
  4,
  4, 3, 4, 4, 3,
  6, 6, 6, 33, 6,
  3,
  1
};


void servoTo(int angle) {
  angle = constrain(angle, 0, 180);
  beater.write(angle);
}

void playMelody() {
  int size = sizeof(durations) / sizeof(int);
  for (int note = 0; note < size; note++) {
    int duration = 1000 / durations[note];
    tone(BUZZER_PIN, melody[note], duration);
    int pauseBetweenNotes = duration * 0.30;
    delay(pauseBetweenNotes);
    noTone(BUZZER_PIN);
  }
}

void strikeOnce(int hitAngle, unsigned long downMs) {
  servoTo(hitAngle);
  delay(downMs);
  servoTo(SERVO_REST);
  delay(RETURN_MS);
}

void strikeMapped(float drop) {
  float d = constrain(drop, MIN_DROP, MAX_DROP);
  float k = (d - MIN_DROP) / float(MAX_DROP - MIN_DROP); 
  int hitAngle = SERVO_HIT_MIN + int((SERVO_HIT_MAX - SERVO_HIT_MIN) * k);
  unsigned long downMs = STRIKE_DOWN_MS_MAX - (unsigned long)((STRIKE_DOWN_MS_MAX - STRIKE_DOWN_MS_MIN) * k);

  // Start melody 
  playMelody(); 
  strikeOnce(hitAngle, downMs);
}

void calibrateAmbient() {
  unsigned long t0 = millis();
  long acc = 0;
  int  n   = 0;
  while (millis() - t0 < CAL_TIME_MS) {
    int r = analogRead(LDR_PIN);
    acc += r;
    n++;
    delay(10);
  }
  baseline = float(acc) / max(1, n);
  ema = baseline;
}

void setup() {
  pinMode(LDR_PIN, INPUT);
  pinMode(BTN_PIN, INPUT_PULLUP); 
  pinMode(BUZZER_PIN, OUTPUT);

  beater.attach(SERVO_PIN);
  servoTo(SERVO_REST);
  delay(300);

  Serial.begin(9600);
  Serial.println("Booting… Keep your hand away for calibration.");
  calibrateAmbient();
  bootTime = millis();
  Serial.print("Baseline = "); Serial.println(baseline);
}

void loop() {
  unsigned long now = millis();
  int raw = analogRead(LDR_PIN);
  ema = ALPHA * raw + (1 - ALPHA) * ema;

  int btn = digitalRead(BTN_PIN);
  if (btn != btnPrev && (now - lastBtnTime) > DEBOUNCE_MS) {
    lastBtnTime = now;
    btnPrev = btn;
    if (btn == LOW) {
      sawAnyButton = true;
      armed = !armed;
      if (armed) baseline = ema;
      Serial.print("TOGGLE armed = "); Serial.println(armed ? "true" : "false");
    }
  }

  if (!armed && !sawAnyButton && (now - bootTime) > AUTO_ARM_MS) {
    armed = true;
    baseline = ema;
    Serial.println("Auto-armed (no button detected).");
  }

  if (armed && (now - lastStrike) > COOLDOWN_MS) {
    float drop = baseline - ema; 
    if (drop > MIN_DROP) {
      Serial.print("Strike. raw="); Serial.print(raw);
      Serial.print(" ema="); Serial.print(ema);
      Serial.print(" drop="); Serial.println(drop);
      strikeMapped(drop);
      lastStrike = millis();
      baseline = 0.98f * baseline + 0.02f * ema;
    }
  }

  if (!armed || (now - lastStrike) > 1200) {
    baseline = 0.995f * baseline + 0.005f * ema;
  }
}
