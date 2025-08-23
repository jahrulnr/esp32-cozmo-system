#include <Arduino.h>
#include "setup/setup.h"

void setupWebServer() {
  if (WEBSERVER_ENABLED) {
    logger->info("Setting up web server...");
    
    webServer = new Communication::WebServer();
    if (webServer->init(WEBSERVER_PORT)) {
      // Setup routes
      webServer->on("/", [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html");
      });
      
      // Serve static files from SPIFFS
      webServer->getServer()->serveStatic("/css/", SPIFFS, "/css/");
      webServer->getServer()->serveStatic("/js/", SPIFFS, "/js/");
      
      webServer->on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("path")) {
          String path = request->getParam("path")->value();
          if (SPIFFS.exists(path)) {
            request->send(SPIFFS, path, String(), true);
          } else {
            request->send(404, "text/plain", "File not found");
          }
        } else {
          request->send(400, "text/plain", "Missing path parameter");
        }
      });
      
      webServer->onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not found");
      });
      
      webServer->begin();
      logger->info("Web server started on port " + String(WEBSERVER_PORT));
    } else {
      logger->error("Web server initialization failed");
    }

    // File upload endpoint
    webServer->getServer()->on("/upload", HTTP_POST, 
      [](AsyncWebServerRequest *request) {},
      [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
        // This second callback handles the actual upload
        static File uploadFile;
        
        if (index == 0) {
          logger->info("Upload started: " + filename);
          
          // Get target path from request parameter or use root directory
          String targetPath = "/";
          if (request->hasParam("path")) {
            targetPath = request->getParam("path")->value();
            // Ensure path ends with slash
            if (!targetPath.endsWith("/")) {
              targetPath += "/";
            }
          }
          
          // Ensure directory exists
          if (!targetPath.equals("/")) {
            if (!fileManager->exists(targetPath)) {
              fileManager->createDir(targetPath);
            }
          }
          
          // Create or open file for writing
          String fullPath = targetPath + filename;
          uploadFile = SPIFFS.open(fullPath, "w");
          
          if (!uploadFile) {
            logger->error("Failed to open file for writing: " + fullPath);
            request->send(500, "application/json", "{\"version\":\"1.0\",\"type\":\"error\",\"data\":{\"code\":500,\"message\":\"Failed to create file\"}}");
            return;
          }
        }
        
        // Write file data
        if (uploadFile && len) {
          uploadFile.write(data, len);
        }
        
        // Final block received
        if (final) {
          if (uploadFile) {
            logger->info("Upload complete: " + String(uploadFile.name()) + " (" + String(uploadFile.size()) + " bytes)");
            
            // Send success response following the new DTO contract format
            Utils::SpiJsonDocument responseDoc;
            responseDoc["version"] = "1.0";
            responseDoc["type"] = "ok";
            responseDoc["data"]["message"] = "File uploaded successfully";
            responseDoc["data"]["filename"] = filename;
            responseDoc["data"]["size"] = uploadFile.size();
            uploadFile.close();
            
            String response;
            serializeJson(responseDoc, response);
            request->send(200, "application/json", response);
          } else {
            request->send(500, "application/json", "{\"version\":\"1.0\",\"type\":\"error\",\"data\":{\"code\":500,\"message\":\"Upload failed\"}}");
          }
        }
      }
    );
  }
}