#include "setup/setup.h"
#include "tasks/register.h"

/**
 * Handles cliff detection and evasive maneuvers
 * @return true if cliff was detected and handled
 */
bool handleCliffDetection() {
    if (!cliffLeftDetector || !cliffRightDetector) {
        return false;  // No cliff detected
    }

    if (motors) {
        motors->interuptMotor();

        motors->move(Motors::MotorControl::BACKWARD, 1000);
        Motors::MotorControl::Direction turnDirection = (rand() % 2 == 0) ?
                Motors::MotorControl::LEFT : Motors::MotorControl::RIGHT;
        motors->move(turnDirection, 1000);
        motors->stop();
    }

    if (logger) {
        logger->info("Cliff detected - evasive action taken");
    }

    return true;  // Cliff was detected and handled
}

/**
 * Handles obstacle detection and evasive maneuvers
 * @return true if obstacle was detected and handled
 */
bool handleObstacleDetection() {
    if (!distanceSensor || !distanceSensor->isObstacleDetected()) {
        return false;  // No obstacle detected or sensor not available
    }

    bool pathFound = false;

    if (motors) {
        Motors::MotorControl::Direction currentMove = motors->getCurrentDirection();
        motors->interuptMotor();

        // Try to find a safe path
        int attempts = 0;
        const int maxAttempts = 20;  // Prevent infinite loop

        while (!pathFound && attempts < maxAttempts) {
            motors->interuptMotor();
            // Choose random direction
            Motors::MotorControl::Direction turnDirection = (rand() % 2 == 0) ?
                Motors::MotorControl::LEFT : Motors::MotorControl::RIGHT;

            motors->move(Motors::MotorControl::BACKWARD, 1000);
            motors->move(turnDirection, 1500);

            // Check if path is clear
            pathFound = !distanceSensor->isObstacleDetected();
            attempts++;

            // Small delay before next check
            vTaskDelay(pdMS_TO_TICKS(300));
        }
        
        if(!pathFound) motors->interuptMotor();
    }

    if (logger) {
        logger->info("Obstacle detected - evasive action taken");
    }

    return true;  // Obstacle was detected and handled
}

/**
 * Main protection function that prevents the robot from damage
 * by detecting obstacles and cliffs
 */
bool _protectInProgress = false;
void protectCozmo() {
    if (!motors || _protectInProgress) {
        return;
    }

    Motors::MotorControl::Direction currentDirection = motors->getCurrentDirection();

    // Only run protection when moving forward or turning
    if (currentDirection == Motors::MotorControl::STOP ||
        currentDirection == Motors::MotorControl::BACKWARD) {
        return;
    }

    _protectInProgress = true;
    bool protectionActivated = false;

    // Handle cliff detection first (higher priority)
    if (cliffLeftDetector->isCliffDetected() || cliffRightDetector->isCliffDetected()) {
        protectionActivated = true;
    }

    // Then handle obstacle detection if no cliff was detected
    else if (distanceSensor->isObstacleDetected()) {
        protectionActivated = true;
    }

    // Make sure we stop at the end if protection was activated
    if (protectionActivated) {
        if (motors) {
            motors->stop();
        }
    }

    _protectInProgress = false;
    vTaskDelay(pdMS_TO_TICKS(100));
}

void protectCozmoTask(void * param) {
	while (true)
	{
		protectCozmo();
		vTaskDelay(pdMS_TO_TICKS(5));

		taskYIELD();
	}

}