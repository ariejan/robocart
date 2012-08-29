#include <AFMotor.h>
#include <Servo.h>

Servo pingServo;

AF_DCMotor motorFrontLeft(3);
AF_DCMotor motorBackLeft(4);
AF_DCMotor motorFrontRight(2);
AF_DCMotor motorBackRight(1);

// Speed
int speed = 100;

// Pin definitions
const int pingServoPin = 10;
const int pingPin = 15;

// Moving around
const long minClearanceCm = 30;
const long minTurnClearanceCm = 15;

// pingServo locations
const int headFullRight = 188;
const int headFullLeft = 0;
const int headRight = 150;  // Don't look all-the-way to each side.
const int headLeft = 30;
const int headForward = 94;

bool movingBack = false;

void setMotorSpeed(int speed) {
  motorFrontLeft.setSpeed(speed);
  motorBackLeft.setSpeed(speed);
  motorFrontRight.setSpeed(speed);
  motorBackRight.setSpeed(speed);
}

void runLeftMotors(int direction) {
  motorFrontLeft.run(direction);
  motorBackLeft.run(direction);
}

void runRightMotors(int direction) {
  motorFrontRight.run(direction);
  motorBackRight.run(direction);
}

void runAllMotors(int direction) {
  runLeftMotors(direction);
  runRightMotors(direction);
}

void releaseAllMotors() {
  runAllMotors(RELEASE);
}

void setup() {
  Serial.begin(9600);

  // Initialize our head servo
  pingServo.attach(pingServoPin);

  // Strech our head a little
  pingServo.write(headLeft);  delay(750);
  pingServo.write(headRight); delay(750);
  pingServo.write(headForward);

  setMotorSpeed(speed);
  releaseAllMotors();
}

long ping() {
  // Send a ping
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  // Await feedback
  pinMode(pingPin, INPUT);
  return pulseIn(pingPin, HIGH);
}

/**
 * Measure the distance of the nearest object for the current
 * pingServo position.
 *
 * @returns int distance in centimeters to nearest object
 **/
int measureDistanceInCentimeters() {
  long duration = ping(); // In microseconds

  // Speed of sound is 340.29 m/s, or 0.034029 cm/µs
  return (duration * 0.034029) / 2;
}

/**
 * Look ahead and return the distance to the closest object.
 *
 * @returns int distance in centimeters to nearest object straight ahead.
 **/
int lookAhead() {
  pingServo.write(headForward);
  delay(100);
  return measureDistanceInCentimeters();
}

int lookLeft() {
  pingServo.write(headLeft);
  delay(100);
  return measureDistanceInCentimeters();
}

int lookRight() {
  pingServo.write(headRight);
  delay(100);
  return measureDistanceInCentimeters();
}

void driveForwardForMs(long ms) {
  Serial.print("Moving forward for "); Serial.print(ms); Serial.println("ms.");
  runAllMotors(FORWARD);
  delay(ms);
  releaseAllMotors();
}

void driveBackwardsForMs(long ms) {
  Serial.print("Moving backward for "); Serial.print(ms); Serial.println("ms.");
  runAllMotors(BACKWARD);
  delay(ms);
  releaseAllMotors();
}

void turnLeftForMs(long ms) {
  Serial.print("Turning left for "); Serial.print(ms); Serial.println("ms.");
  runLeftMotors(FORWARD);
  runRightMotors(BACKWARD);
  delay(ms);
  releaseAllMotors();
}

void turnRightForMs(long ms) {
  Serial.print("Turning right for "); Serial.print(ms); Serial.println("ms.");
  runLeftMotors(BACKWARD);
  runRightMotors(FORWARD);
  delay(ms);
  releaseAllMotors();
}

void loop() {
  Serial.println("----");
  Serial.print("Moving back: "); Serial.println(movingBack);

  // Unless moving back, find out the distances to the nearest object
  // If we're moving back, pretend we can never move forward, so we check
  // a different corner.
  int distanceAhead = 0;
  if (!movingBack) {
    distanceAhead = lookAhead();
  }

  Serial.print("distanceAhead: "); Serial.print(distanceAhead); Serial.println("cm.");
  if (distanceAhead > minClearanceCm) {
    // move forward
    driveForwardForMs(1000);
  } else {
    // We can't move forward, where to go?
    int distanceLeft = lookLeft();
    int distanceRight = lookRight();

    Serial.print("distanceLeft: "); Serial.print(distanceLeft); Serial.println("cm.");
    Serial.print("distanceRight: "); Serial.print(distanceRight); Serial.println("cm.");

    if (distanceLeft > minTurnClearanceCm && distanceLeft > distanceRight) {
      movingBack = false;

      // Okay, most room to the left, go there.
      turnLeftForMs(750);
    } else if (distanceRight > minTurnClearanceCm && distanceRight > distanceLeft) {
      movingBack = false;

      // Okay, move to the right.
      turnRightForMs(750);
    } else {
      // No where to go! Move back a little
      movingBack = true;
      driveBackwardsForMs(500);
    }
  }
}