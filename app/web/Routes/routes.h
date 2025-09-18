#ifndef ROUTES_H
#define ROUTES_H

#include <MVCFramework.h>
#include <setup/setup.h>
#include <LittleFS.h>
#include "Routing/Router.h"
#include "web/Controllers/AuthController.h"
#include "web/Controllers/SystemController.h"
#include "web/Controllers/RobotController.h"

void registerWebRoutes(Router* router);
void registerApiRoutes(Router* router);
void registerWebSocketRoutes(Router* router);

#endif
