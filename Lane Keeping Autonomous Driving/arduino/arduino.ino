// Pins configuration
#define PIN_Motor_PWMA 5    // Right motor speed
#define PIN_Motor_PWMB 6    // Left motor speed
#define PIN_Motor_AIN_1 7   // Right motor direction
#define PIN_Motor_BIN_1 8   // Left motor direction
#define PIN_Motor_STBY 3

// Operating parameters
#define SPEED 50    // normal speed（0-255）

// Motors control function
void setMotors(int STEER) {
  // Right motors control
  digitalWrite(PIN_Motor_AIN_1, HIGH);
  analogWrite(PIN_Motor_PWMA, abs(SPEED - STEER));
  
  // Left motors control
  digitalWrite(PIN_Motor_BIN_1, HIGH);
  analogWrite(PIN_Motor_PWMB, abs(SPEED + STEER));
}

void setup() {
  // Initialize the motor control pins
  pinMode(PIN_Motor_PWMA, OUTPUT);
  pinMode(PIN_Motor_AIN_1, OUTPUT);
  pinMode(PIN_Motor_PWMB, OUTPUT);
  pinMode(PIN_Motor_BIN_1, OUTPUT);
  pinMode(PIN_Motor_STBY, OUTPUT);
  Serial.begin(115200); 
}

unsigned long lastDataTime = 0;  // Record the timestamp of the last received data

void loop() {
  if (Serial.available()) {
    // Record the latest timestamp when data is received
    lastDataTime = millis();
    
    digitalWrite(PIN_Motor_STBY, HIGH);
    String input = Serial.readStringUntil('\n');
    // Convert to integer
    int result = input.toInt();
    Serial.print("Received the prediction: ");
    Serial.println(result);
    setMotors(result);
  }
  else {
    // Check if there has been no data for more than 1 second
    if (millis() - lastDataTime > 1000) {
      digitalWrite(PIN_Motor_STBY, LOW);
    }
  }
}
