#include <app.h>

void setupPins(){
	if (LEFT_MOTOR_PIN1 > -1) {
		pinMode(LEFT_MOTOR_PIN1, OUTPUT);
		digitalWrite(LEFT_MOTOR_PIN1, LOW);
		detachInterrupt(LEFT_MOTOR_PIN1);
	}
	if (LEFT_MOTOR_PIN2 > -1) {
		pinMode(LEFT_MOTOR_PIN2, OUTPUT);
		digitalWrite(LEFT_MOTOR_PIN2, LOW);
		detachInterrupt(LEFT_MOTOR_PIN2);
	}
	if (RIGHT_MOTOR_PIN1 > -1) {
		pinMode(RIGHT_MOTOR_PIN1, OUTPUT);
		digitalWrite(RIGHT_MOTOR_PIN1, LOW);
		detachInterrupt(RIGHT_MOTOR_PIN1);
	}
	if (RIGHT_MOTOR_PIN2 > -1) {
		pinMode(RIGHT_MOTOR_PIN2, OUTPUT);
		digitalWrite(RIGHT_MOTOR_PIN2, LOW);
		detachInterrupt(RIGHT_MOTOR_PIN2);
	}
	if (HEAD_SERVO_PIN > -1) {
		pinMode(HEAD_SERVO_PIN, OUTPUT);
		digitalWrite(HEAD_SERVO_PIN, LOW);
		detachInterrupt(HEAD_SERVO_PIN);
	}
	if (HAND_SERVO_PIN > -1) {
		pinMode(HAND_SERVO_PIN, OUTPUT);
		digitalWrite(HAND_SERVO_PIN, LOW);
		detachInterrupt(HAND_SERVO_PIN);
	}
	if (SCREEN_SDA_PIN > -1) {
		pinMode(SCREEN_SDA_PIN, OUTPUT);
		digitalWrite(SCREEN_SDA_PIN, LOW);
		detachInterrupt(SCREEN_SDA_PIN);
	}
	if (SCREEN_SCL_PIN > -1) {
		pinMode(SCREEN_SCL_PIN, OUTPUT);
		digitalWrite(SCREEN_SCL_PIN, LOW);
		detachInterrupt(SCREEN_SCL_PIN);
	}
	if (CLIFF_RIGHT_DETECTOR_PIN > -1) {
		pinMode(CLIFF_RIGHT_DETECTOR_PIN, OUTPUT);
		digitalWrite(CLIFF_RIGHT_DETECTOR_PIN, LOW);
		detachInterrupt(CLIFF_RIGHT_DETECTOR_PIN);
	}
	if (CLIFF_LEFT_DETECTOR_PIN > -1) {
		pinMode(CLIFF_LEFT_DETECTOR_PIN, OUTPUT);
		digitalWrite(CLIFF_LEFT_DETECTOR_PIN, LOW);
		detachInterrupt(CLIFF_LEFT_DETECTOR_PIN);
	}
	if (ULTRASONIC_TRIGGER_PIN > -1) {
		pinMode(ULTRASONIC_TRIGGER_PIN, OUTPUT);
		digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);
		detachInterrupt(ULTRASONIC_TRIGGER_PIN);
	}
	if (ULTRASONIC_ECHO_PIN > -1) {
		pinMode(ULTRASONIC_ECHO_PIN, OUTPUT);
		digitalWrite(ULTRASONIC_ECHO_PIN, LOW);
		detachInterrupt(ULTRASONIC_ECHO_PIN);
	}
}