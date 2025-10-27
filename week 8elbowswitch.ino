// Elbow-activated switch with LED

const int switchPin = 2;   // Pin connected to Coin A
const int ledPin = 13;     // Pin connected to LED (built-in or external)

void setup() {
  // Set Pin 2 as input with internal pull-up resistor
  pinMode(switchPin, INPUT_PULLUP);

  // Set LED pin as output
  pinMode(ledPin, OUTPUT);

  // Start with LED off
  digitalWrite(ledPin, LOW);
}

void loop() {
  // Read the state of the switch
  int state = digitalRead(switchPin);

  // If coins are bridged (elbow touches), state is LOW
  if (state == LOW) {
    digitalWrite(ledPin, HIGH);  // Turn LED ON
  } else {
    digitalWrite(ledPin, LOW);   // Turn LED OFF
  }

  // Small delay for stability (optional)
  delay(50);
}
