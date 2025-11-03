// Pin Definitions 
const int potPin = A0;       // Potentiometer middle pin connected to A0
const int buttonPin = 2;     // Pushbutton connected to digital pin 2
const int greenLedPin = 8;   // Green LED connected to digital pin 8 
const int blueLedPin = 9;    // Blue LED connected to digital pin 9 

// Variables 
int potValue = 0;           // Stores analog reading from potentiometer
int ledBrightness = 0;      // Stores mapped PWM value for blue LED
int buttonState = HIGH;     // Current reading of the button
int lastButtonState = HIGH; // Previous reading of the button
bool greenLedOn = false;    // Green LED toggle state

void setup() {
  pinMode(buttonPin, INPUT_PULLUP); // Enable internal pull-up resistor
  pinMode(greenLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);
  
  Serial.begin(9600); 
}

void loop() {
  //Read potentiometer and set blue LED brightness
  potValue = analogRead(potPin);                   // Read 0-1023
  ledBrightness = map(potValue, 0, 1023, 0, 255); // Map to PWM 0-255
  analogWrite(blueLedPin, ledBrightness);         // Set blue LED brightness

  // Read pushbutton and toggle green LED 
  buttonState = digitalRead(buttonPin);

  // Detect button press (LOW) that just changed from HIGH
  if (buttonState == LOW && lastButtonState == HIGH) {
    greenLedOn = !greenLedOn;                       // Toggle green LED state
    digitalWrite(greenLedPin, greenLedOn ? HIGH : LOW);
    delay(50);                                     // Debounce delay
  }

  lastButtonState = buttonState;                   // Save for next loop

  //debug output
  Serial.print("Pot: ");
  Serial.print(potValue);
  Serial.print("  Brightness (Blue LED): ");
  Serial.print(ledBrightness);
  Serial.print("  Green LED: ");
  Serial.println(greenLedOn);

  delay(10); // Small loop delay
}
