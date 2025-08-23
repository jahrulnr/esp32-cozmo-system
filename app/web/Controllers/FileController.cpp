#include "FileController.h"
#include "setup/setup.h"

Response FileController::download(Request& request) {
    // Check authentication for protected downloads
    if (requiresAuthentication("download")) {
        User* user = AuthController::getCurrentUser(request);
        if (!user) {
            return unauthorizedResponse(request);
        }
        delete user;
    }
    
    String path = request.input("path");
    if (path.isEmpty()) {
        JsonDocument error = createErrorResponse("Missing path parameter", "MISSING_PATH");
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    path = sanitizePath(path);
    
    if (!isValidPath(path)) {
        JsonDocument error = createErrorResponse("Invalid file path", "INVALID_PATH");
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    if (!SPIFFS.exists(path)) {
        JsonDocument error = createErrorResponse("File not found", "FILE_NOT_FOUND");
        return Response(request.getServerRequest())
            .status(404)
            .json(error);
    }
		
    return Response(request.getServerRequest())
        .status(200)
        .header("Content-Disposition", "attachment; filename=\"" + path.substring(path.lastIndexOf('/') + 1) + "\"")
        .file(path);
}

Response FileController::upload(Request& request) {
    // Check authentication
    User* user = AuthController::getCurrentUser(request);
    if (!user) {
        return unauthorizedResponse(request);
    }
    delete user;
    
    String filename = request.input("filename");
    String content = request.input("content");
    String targetPath = request.input("path", "/");
    
    if (filename.isEmpty()) {
        JsonDocument error = createErrorResponse("Filename is required", "MISSING_FILENAME");
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    if (!isAllowedFileType(filename)) {
        JsonDocument error = createErrorResponse("File type not allowed", "INVALID_FILE_TYPE");
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    targetPath = sanitizePath(targetPath);
    if (!targetPath.endsWith("/")) {
        targetPath += "/";
    }
    
    // Ensure directory exists
    if (!targetPath.equals("/")) {
        if (!SPIFFS.exists(targetPath) && SPIFFS.mkdir(targetPath)) {
						logger->info("Creating directory path: " + targetPath);
        }
    }
    
    String fullPath = targetPath + filename;
    fullPath = sanitizePath(fullPath);
    
    // Open file for writing
    File file = SPIFFS.open(fullPath, "w");
    if (!file) {
        JsonDocument error = createErrorResponse("Failed to create file", "FILE_CREATION_ERROR");
        return Response(request.getServerRequest())
            .status(500)
            .json(error);
    }
    
    size_t bytesWritten = file.print(content);
    file.close();
    
    logger->info("File uploaded: " + fullPath + " (" + String(bytesWritten) + " bytes)");
    
    JsonDocument responseData;
    responseData["filename"] = filename;
    responseData["path"] = fullPath;
    responseData["size"] = bytesWritten;
    responseData["message"] = "File uploaded successfully";
    
    JsonDocument response = createSuccessResponse(responseData);
    
    return Response(request.getServerRequest())
        .status(201)
        .json(response);
}

Response FileController::listFiles(Request& request) {
    // Check authentication for file listing
    if (requiresAuthentication("list")) {
        User* user = AuthController::getCurrentUser(request);
        if (!user) {
            return unauthorizedResponse(request);
        }
        delete user;
    }
    
    String directory = request.input("directory", "/");
    directory = sanitizePath(directory);
    
    if (!isValidPath(directory)) {
        JsonDocument error = createErrorResponse("Invalid directory path", "INVALID_PATH");
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    JsonDocument responseData;
    JsonArray files = responseData["files"].to<JsonArray>();
    
    // List all files in SPIFFS (SPIFFS doesn't have true directories)
    File root = SPIFFS.open("/");
    if (!root) {
        JsonDocument error = createErrorResponse("Failed to open root directory", "DIRECTORY_ACCESS_ERROR");
        return Response(request.getServerRequest())
            .status(500)
            .json(error);
    }
    
    File file = root.openNextFile();
    int fileCount = 0;
    
    while (file) {
        String fileName = String(file.name());
        
        // Filter files by directory if not root
        if (directory.equals("/") || fileName.startsWith(directory)) {
            JsonObject fileInfo = files.add<JsonObject>();
            fileInfo["name"] = fileName;
            fileInfo["size"] = file.size();
            fileInfo["is_directory"] = file.isDirectory();
            
            // Add relative path (remove directory prefix if listing subdirectory)
            if (!directory.equals("/")) {
                String relativeName = fileName.substring(directory.length());
                if (relativeName.startsWith("/")) {
                    relativeName = relativeName.substring(1);
                }
                fileInfo["relative_name"] = relativeName;
            }
            
            fileCount++;
        }
        
        file = root.openNextFile();
    }
    
    root.close();
    
    responseData["directory"] = directory;
    responseData["count"] = fileCount;
    responseData["total_size"] = SPIFFS.totalBytes();
    responseData["used_size"] = SPIFFS.usedBytes();
    responseData["free_size"] = SPIFFS.totalBytes() - SPIFFS.usedBytes();
    
    JsonDocument response = createSuccessResponse(responseData);
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

Response FileController::deleteFile(Request& request) {
    // Check authentication
    User* user = AuthController::getCurrentUser(request);
    if (!user) {
        return unauthorizedResponse(request);
    }
    delete user;
    
    String path = request.input("path");
    if (path.isEmpty()) {
        JsonDocument error = createErrorResponse("Missing path parameter", "MISSING_PATH");
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    path = sanitizePath(path);
    
    if (!isValidPath(path)) {
        JsonDocument error = createErrorResponse("Invalid file path", "INVALID_PATH");
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    if (!SPIFFS.exists(path)) {
        JsonDocument error = createErrorResponse("File not found", "FILE_NOT_FOUND");
        return Response(request.getServerRequest())
            .status(404)
            .json(error);
    }
    
    // Prevent deletion of critical system files
    if (path.startsWith("/css/") || path.startsWith("/js/") || path.equals("/index.html")) {
        JsonDocument error = createErrorResponse("Cannot delete system files", "PROTECTED_FILE");
        return Response(request.getServerRequest())
            .status(403)
            .json(error);
    }
    
    if (SPIFFS.remove(path)) {
        logger->info("File deleted: " + path);
        
        JsonDocument responseData;
        responseData["path"] = path;
        responseData["message"] = "File deleted successfully";
        
        JsonDocument response = createSuccessResponse(responseData);
        
        return Response(request.getServerRequest())
            .status(200)
            .json(response);
    } else {
        JsonDocument error = createErrorResponse("Failed to delete file", "DELETE_ERROR");
        return Response(request.getServerRequest())
            .status(500)
            .json(error);
    }
}

Response FileController::getFileInfo(Request& request) {
    String path = request.input("path");
    if (path.isEmpty()) {
        JsonDocument error = createErrorResponse("Missing path parameter", "MISSING_PATH");
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    path = sanitizePath(path);
    
    if (!isValidPath(path)) {
        JsonDocument error = createErrorResponse("Invalid file path", "INVALID_PATH");
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    JsonDocument fileInfo = formatFileInfo(path);
    
    if (!fileInfo["exists"]) {
        JsonDocument error = createErrorResponse("File not found", "FILE_NOT_FOUND");
        return Response(request.getServerRequest())
            .status(404)
            .json(error);
    }
    
    JsonDocument response = createSuccessResponse(fileInfo);
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

Response FileController::getStorageInfo(Request& request) {
    JsonDocument responseData;
    responseData["total_bytes"] = SPIFFS.totalBytes();
    responseData["used_bytes"] = SPIFFS.usedBytes();
    responseData["free_bytes"] = SPIFFS.totalBytes() - SPIFFS.usedBytes();
    responseData["usage_percent"] = (SPIFFS.usedBytes() * 100) / SPIFFS.totalBytes();
    
    // Format human-readable sizes
    responseData["total_formatted"] = formatBytes(SPIFFS.totalBytes());
    responseData["used_formatted"] = formatBytes(SPIFFS.usedBytes());
    responseData["free_formatted"] = formatBytes(SPIFFS.totalBytes() - SPIFFS.usedBytes());
    
    JsonDocument response = createSuccessResponse(responseData);
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

// Helper methods
bool FileController::isValidPath(const String& path) {
    return path.startsWith("/") && 
           path.indexOf("..") == -1 && 
           path.length() > 0 && 
           path.length() < 256;
}

bool FileController::isAllowedFileType(const String& filename) {
    // Allow common text and web file types
    // return filename.endsWith(".txt") || 
    //        filename.endsWith(".json") || 
    //        filename.endsWith(".html") || 
    //        filename.endsWith(".css") || 
    //        filename.endsWith(".js") ||
    //        filename.endsWith(".md") ||
    //        filename.endsWith(".xml") ||
    //        filename.endsWith(".log");
		return true;
}

String FileController::sanitizePath(const String& path) {
    String cleaned = path;
    
    // Remove dangerous path traversal attempts
    cleaned.replace("../", "");
    cleaned.replace("..\\", "");
    cleaned.replace("//", "/");
    
    // Ensure path starts with /
    if (!cleaned.startsWith("/")) {
        cleaned = "/" + cleaned;
    }
    
    // Remove trailing slash unless it's root
    if (cleaned.length() > 1 && cleaned.endsWith("/")) {
        cleaned = cleaned.substring(0, cleaned.length() - 1);
    }
    
    return cleaned;
}

JsonDocument FileController::formatFileInfo(const String& path) {
    JsonDocument info;
    
    if (SPIFFS.exists(path)) {
        File file = SPIFFS.open(path, "r");
        if (file) {
            info["exists"] = true;
            info["path"] = path;
            info["name"] = path.substring(path.lastIndexOf('/') + 1);
            info["size"] = file.size();
            info["size_formatted"] = formatBytes(file.size());
            info["is_directory"] = file.isDirectory();
            
            // Determine file type
            String extension = "";
            int lastDot = path.lastIndexOf('.');
            if (lastDot > 0) {
                extension = path.substring(lastDot + 1);
            }
            info["extension"] = extension;
            
            // Determine MIME type
            String mimeType = "application/octet-stream";
            if (extension == "txt") mimeType = "text/plain";
            else if (extension == "html") mimeType = "text/html";
            else if (extension == "css") mimeType = "text/css";
            else if (extension == "js") mimeType = "application/javascript";
            else if (extension == "json") mimeType = "application/json";
            else if (extension == "xml") mimeType = "application/xml";
            
            info["mime_type"] = mimeType;
            file.close();
        } else {
            info["exists"] = false;
            info["error"] = "Cannot access file";
        }
    } else {
        info["exists"] = false;
    }
    
    return info;
}

JsonDocument FileController::createErrorResponse(const String& message, const String& code) {
    JsonDocument error;
    error["success"] = false;
    error["message"] = message;
    error["timestamp"] = millis();
    
    if (!code.isEmpty()) {
        error["error_code"] = code;
    }
    
    return error;
}

JsonDocument FileController::createSuccessResponse(const JsonDocument& data) {
    JsonDocument response;
    response["success"] = true;
    response["timestamp"] = millis();
    
    if (!data.isNull()) {
        response["data"] = data;
    }
    
    return response;
}

bool FileController::requiresAuthentication(const String& operation) {
    // List operations can be public for basic file access
    // Upload, delete, and downloads of sensitive files require auth
    return operation != "list" && operation != "info";
}

Response FileController::unauthorizedResponse(Request& request) {
    JsonDocument error = createErrorResponse("Authentication required", "UNAUTHORIZED");
    return Response(request.getServerRequest())
        .status(401)
        .json(error);
}

String FileController::formatBytes(size_t bytes) {
    if (bytes < 1024) {
        return String(bytes) + " B";
    } else if (bytes < 1024 * 1024) {
        return String(bytes / 1024.0, 1) + " KB";
    } else if (bytes < 1024 * 1024 * 1024) {
        return String(bytes / (1024.0 * 1024.0), 1) + " MB";
    } else {
        return String(bytes / (1024.0 * 1024.0 * 1024.0), 1) + " GB";
    }
}