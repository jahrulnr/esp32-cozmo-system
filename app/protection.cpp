#include "app.h"

// Global semaphore for protection screen messages
SemaphoreHandle_t g_protectionScreenMutex = nullptr;

/**
 * Handles cliff detection and evasive maneuvers
 * @return true if cliff was detected and handled
 */
bool handleCliffDetection() {
    if (!cliffDetected()) {
        return false;  // No cliff detected
    }
    
    if (motors && screen) {
        motors->stop();
        
        // Initialize mutex if needed
        if (!g_protectionScreenMutex) {
            g_protectionScreenMutex = xSemaphoreCreateMutex();
        }
        
        // Try to take the mutex to update the screen, but don't block if unavailable
        if (xSemaphoreTake(g_protectionScreenMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            screen->mutexClear();
            screen->drawCenteredText(20, "Oops! Not a safe area.");
            screen->mutexUpdate();
            
            // Release the mutex so other protection code can use the screen
            xSemaphoreGive(g_protectionScreenMutex);
        }
        
        // Perform evasive action
        vTaskDelay(pdMS_TO_TICKS(300));
        motors->move(Motors::MotorControl::BACKWARD, 1000);
        motors->move(Motors::MotorControl::LEFT, 500);
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
    
    if (motors && screen) {
        Motors::MotorControl::Direction currentMove = motors->getCurrentDirection();
        motors->stop();
        
        // Initialize mutex if needed
        if (!g_protectionScreenMutex) {
            g_protectionScreenMutex = xSemaphoreCreateMutex();
        }
        
        // Try to take the mutex to update the screen, but don't block if unavailable
        if (xSemaphoreTake(g_protectionScreenMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            screen->mutexClear();
            screen->drawCenteredText(20, "Oops! Finding another way!");
            screen->mutexUpdate();
            
            // Release the mutex so other protection code can use the screen
            xSemaphoreGive(g_protectionScreenMutex);
        }
        
        // Try to find a safe path
        int attempts = 0;
        const int maxAttempts = 5;  // Prevent infinite loop
        bool pathFound = false;
        
        while (!pathFound && attempts < maxAttempts) {
            // Choose random direction
            Motors::MotorControl::Direction turnDirection = (rand() % 2 == 0) ? 
                Motors::MotorControl::LEFT : Motors::MotorControl::RIGHT;
            
            motors->move(turnDirection, 300);
            
            // Check if path is clear
            pathFound = !distanceSensor->isObstacleDetected();
            attempts++;
            
            // Small delay before next check
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        if (pathFound) {
            // Resume previous movement if it was moving forward
            if (currentMove == Motors::MotorControl::FORWARD) {
                motors->move(Motors::MotorControl::FORWARD, 500);
            }
        } else {
            // If no path found after attempts, just stop
            motors->stop();
            
            // Try to take the mutex to update the screen
            if (xSemaphoreTake(g_protectionScreenMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                screen->mutexClear();
                screen->drawCenteredText(40, "I'm stuck!");
                screen->mutexUpdate();
                
                // Release the mutex
                xSemaphoreGive(g_protectionScreenMutex);
            }
        }
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
void protectCozmo() {
    // Only protect when moving forward (not when already stopped or backing up)
    if (!motors) {
        return;  // Can't protect without motors
    }
    
    Motors::MotorControl::Direction currentDirection = motors->getCurrentDirection();
    
    // Only run protection when moving forward or turning
    if (currentDirection == Motors::MotorControl::STOP || 
        currentDirection == Motors::MotorControl::BACKWARD) {
        return;
    }
    
    bool protectionActivated = false;
    
    // Handle cliff detection first (higher priority)
    if (handleCliffDetection()) {
        protectionActivated = true;
    }
    
    // Then handle obstacle detection if no cliff was detected
    else if (handleObstacleDetection()) {
        protectionActivated = true;
    }
    
    // Make sure we stop at the end if protection was activated
    if (protectionActivated) {
        if (motors) {
            motors->stop();
        }
        
        // Update last manual control time to prevent automation from taking over immediately
        updateManualControlTime();
    }
}

void protectCozmoTask(void * param) {
	while (true)
	{
		protectCozmo();
		vTaskDelay(pdMS_TO_TICKS(5));
	}
	
}