#ifndef ROUTES_H
#define ROUTES_H

#include <MVCFramework.h>
#include <setup/setup.h>
#include <SPIFFS.h>
#include "Routing/Router.h"
#include "../Controllers/AuthController.h"
#include "../Controllers/SystemController.h"

void registerWebRoutes(Router* router);
void registerApiRoutes(Router* router);
void registerWebSocketRoutes(Router* router);

#endif
