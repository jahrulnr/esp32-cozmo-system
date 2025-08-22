#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include <Arduino.h>

#include "Database/CsvDatabase.h"
#include "Database/Model.h"

#include "Routing/Router.h"

#include "Http/Middleware.h"
#include "Http/Request.h"
#include "Http/Response.h"
#include "Http/Controller.h"
#include "Http/WebSocketRequest.h"

#include "View/View.h"

#include "Core/Config.h"
#include "Core/Application.h"
#include "Core/ServiceContainer.h"
#include "Core/ApplicationTemplates.h"

#endif