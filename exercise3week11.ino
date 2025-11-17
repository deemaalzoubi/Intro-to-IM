int led = 9;

void setup() {
  Serial.begin(9600);
  pinMode(led, OUTPUT);
}

void loop() {
  if (Serial.available()) {
    int value = Serial.read();
    analogWrite(led, value);
  }
}
