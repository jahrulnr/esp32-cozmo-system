#include "FileController.h"
#include "setup/setup.h"

Response FileController::download(Request& request) {
    // Check authentication for protected downloads
    if (requiresAuthentication("download")) {
        IModel::User* user = AuthController::getCurrentUser(request);
        if (!user) {
            return unauthorizedResponse(request);
        }
        delete user;
    }
    
    String path = request.input("path");
    if (path.isEmpty()) {
        Utils::SpiJsonDocument error = createErrorResponse("Missing path parameter", "MISSING_PATH");
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    path = sanitizePath(path).c_str();
    
    if (!isValidPath(path)) {
        Utils::SpiJsonDocument error = createErrorResponse("Invalid file path", "INVALID_PATH");
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    if (!LittleFS.exists(path)) {
        Utils::SpiJsonDocument error = createErrorResponse("File not found", "FILE_NOT_FOUND");
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
    IModel::User* user = AuthController::getCurrentUser(request);
    if (!user) {
        return unauthorizedResponse(request);
    }
    delete user;
    
    String filename = request.input("filename");
    String content = request.input("content");
    String targetPath = request.input("path", "/");
    
    if (filename.isEmpty()) {
        Utils::SpiJsonDocument error = createErrorResponse("Filename is required", "MISSING_FILENAME");
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    if (!isAllowedFileType(filename)) {
        Utils::SpiJsonDocument error = createErrorResponse("File type not allowed", "INVALID_FILE_TYPE");
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    targetPath = sanitizePath(targetPath).c_str();
    if (!targetPath.endsWith("/")) {
        targetPath += "/";
    }
    
    // Ensure directory exists
    if (!targetPath.equals("/")) {
        if (!LittleFS.exists(targetPath) && LittleFS.mkdir(targetPath)) {
						logger->info("Creating directory path: " + targetPath);
        }
    }
    
    String fullPath = (targetPath + filename).c_str();
    fullPath = sanitizePath(fullPath).c_str();
    
    // Open file for writing
    File file = LittleFS.open(fullPath, "w");
    if (!file) {
        Utils::SpiJsonDocument error = createErrorResponse("Failed to create file", "FILE_CREATION_ERROR");
        return Response(request.getServerRequest())
            .status(500)
            .json(error);
    }
    
    size_t bytesWritten = file.print(content.c_str());
    file.close();
    
    logger->info("File uploaded: %s (%d bytes)", fullPath, bytesWritten);
    
    Utils::SpiJsonDocument responseData;
    responseData["filename"] = filename;
    responseData["path"] = fullPath;
    responseData["size"] = bytesWritten;
    responseData["message"] = "File uploaded successfully";
    
    Utils::SpiJsonDocument response = createSuccessResponse(responseData);
    
    return Response(request.getServerRequest())
        .status(201)
        .json(response);
}

Response FileController::listFiles(Request& request) {
    // Check authentication for file listing
    if (requiresAuthentication("list")) {
        IModel::User* user = AuthController::getCurrentUser(request);
        if (!user) {
            return unauthorizedResponse(request);
        }
        delete user;
    }
    
    Utils::Sstring directory = request.input("directory", "/");
    directory = sanitizePath(directory).c_str();
    
    if (!isValidPath(directory)) {
        Utils::SpiJsonDocument error = createErrorResponse("Invalid directory path", "INVALID_PATH");
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    Utils::SpiJsonDocument responseData;
    JsonArray files = responseData["files"].to<JsonArray>();
    
    // List all files in LittleFS (LittleFS doesn't have true directories)
    File root = LittleFS.open("/");
    if (!root) {
        Utils::SpiJsonDocument error = createErrorResponse("Failed to open root directory", "DIRECTORY_ACCESS_ERROR");
        return Response(request.getServerRequest())
            .status(500)
            .json(error);
    }
    
    File file = root.openNextFile();
    int fileCount = 0;
    
    while (file) {
        Utils::Sstring fileName = Utils::Sstring(file.name());
        
        // Filter files by directory if not root
        if (directory.equals("/") || fileName.startsWith(directory)) {
            JsonObject fileInfo = files.add<JsonObject>();
            fileInfo["name"] = fileName;
            fileInfo["size"] = file.size();
            fileInfo["is_directory"] = file.isDirectory();
            
            // Add relative path (remove directory prefix if listing subdirectory)
            if (!directory.equals("/")) {
                Utils::Sstring relativeName = fileName.substring(directory.length());
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
    responseData["total_size"] = LittleFS.totalBytes();
    responseData["used_size"] = LittleFS.usedBytes();
    responseData["free_size"] = LittleFS.totalBytes() - LittleFS.usedBytes();
    
    Utils::SpiJsonDocument response = createSuccessResponse(responseData);
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

Response FileController::deleteFile(Request& request) {
    // Check authentication
    IModel::User* user = AuthController::getCurrentUser(request);
    if (!user) {
        return unauthorizedResponse(request);
    }
    delete user;
    
    String path = request.input("path");
    if (path.isEmpty()) {
        Utils::SpiJsonDocument error = createErrorResponse("Missing path parameter", "MISSING_PATH");
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    path = sanitizePath(path).c_str();
    
    if (!isValidPath(path)) {
        Utils::SpiJsonDocument error = createErrorResponse("Invalid file path", "INVALID_PATH");
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    if (!LittleFS.exists(path)) {
        Utils::SpiJsonDocument error = createErrorResponse("File not found", "FILE_NOT_FOUND");
        return Response(request.getServerRequest())
            .status(404)
            .json(error);
    }
    
    // Prevent deletion of critical system files
    if (path.startsWith("/css/") || path.startsWith("/js/") || path.equals("/index.html")) {
        Utils::SpiJsonDocument error = createErrorResponse("Cannot delete system files", "PROTECTED_FILE");
        return Response(request.getServerRequest())
            .status(403)
            .json(error);
    }
    
    if (LittleFS.remove(path.c_str())) {
        logger->info("File deleted: %s" + path);
        
        Utils::SpiJsonDocument responseData;
        responseData["path"] = path;
        responseData["message"] = "File deleted successfully";
        
        Utils::SpiJsonDocument response = createSuccessResponse(responseData);
        
        return Response(request.getServerRequest())
            .status(200)
            .json(response);
    } else {
        Utils::SpiJsonDocument error = createErrorResponse("Failed to delete file", "DELETE_ERROR");
        return Response(request.getServerRequest())
            .status(500)
            .json(error);
    }
}

Response FileController::getFileInfo(Request& request) {
    Utils::Sstring path = request.input("path");
    if (path.isEmpty()) {
        Utils::SpiJsonDocument error = createErrorResponse("Missing path parameter", "MISSING_PATH");
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    path = sanitizePath(path).c_str();
    
    if (!isValidPath(path)) {
        Utils::SpiJsonDocument error = createErrorResponse("Invalid file path", "INVALID_PATH");
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    Utils::SpiJsonDocument fileInfo = formatFileInfo(path);
    
    if (!fileInfo["exists"]) {
        Utils::SpiJsonDocument error = createErrorResponse("File not found", "FILE_NOT_FOUND");
        return Response(request.getServerRequest())
            .status(404)
            .json(error);
    }
    
    Utils::SpiJsonDocument response = createSuccessResponse(fileInfo);
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

Response FileController::getStorageInfo(Request& request) {
    Utils::SpiJsonDocument responseData;
    responseData["total_bytes"] = LittleFS.totalBytes();
    responseData["used_bytes"] = LittleFS.usedBytes();
    responseData["free_bytes"] = LittleFS.totalBytes() - LittleFS.usedBytes();
    responseData["usage_percent"] = (LittleFS.usedBytes() * 100) / LittleFS.totalBytes();
    
    // Format human-readable sizes
    responseData["total_formatted"] = formatBytes(LittleFS.totalBytes());
    responseData["used_formatted"] = formatBytes(LittleFS.usedBytes());
    responseData["free_formatted"] = formatBytes(LittleFS.totalBytes() - LittleFS.usedBytes());
    
    Utils::SpiJsonDocument response = createSuccessResponse(responseData);
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

// Helper methods
bool FileController::isValidPath(const Utils::Sstring& path) {
    return path.startsWith("/") && 
           path.indexOf("..") == -1 && 
           path.length() > 0 && 
           path.length() < 256;
}

bool FileController::isAllowedFileType(const Utils::Sstring& filename) {
		return true;
}

Utils::Sstring FileController::sanitizePath(const Utils::Sstring& path) {
    Utils::Sstring cleaned = path;
    
    // Remove dangerous path traversal attempts
    cleaned.replace("../", "");
    cleaned.replace("..\\", "");
    cleaned.replace("//", "/");
    
    // Ensure path starts with /
    if (!cleaned.startsWith("/")) {
        cleaned = "/"; 
        cleaned += cleaned.c_str();
    }
    
    // Remove trailing slash unless it's root
    if (cleaned.length() > 1 && cleaned.toString().endsWith("/")) {
        cleaned = cleaned.substring(0, cleaned.length() - 1);
    }
    
    return cleaned;
}

Utils::SpiJsonDocument FileController::formatFileInfo(const Utils::Sstring& path) {
    Utils::SpiJsonDocument info;
    
    if (LittleFS.exists(path.c_str())) {
        File file = LittleFS.open(path.c_str(), "r");
        if (file) {
            info["exists"] = true;
            info["path"] = path;
            info["name"] = path.substring(path.toString().lastIndexOf('/') + 1);
            info["size"] = file.size();
            info["size_formatted"] = formatBytes(file.size());
            info["is_directory"] = file.isDirectory();
            
            // Determine file type
            Utils::Sstring extension = "";
            int lastDot = path.toString().lastIndexOf('.');
            if (lastDot > 0) {
                extension = path.substring(lastDot + 1);
            }
            info["extension"] = extension;
            
            // Determine MIME type
            Utils::Sstring mimeType = "application/octet-stream";
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

Utils::SpiJsonDocument FileController::createErrorResponse(const Utils::Sstring& message, const Utils::Sstring& code) {
    Utils::SpiJsonDocument error;
    error["success"] = false;
    error["message"] = message;
    error["timestamp"] = millis();
    
    if (!code.isEmpty()) {
        error["error_code"] = code;
    }
    
    return error;
}

Utils::SpiJsonDocument FileController::createSuccessResponse(const Utils::SpiJsonDocument& data) {
    Utils::SpiJsonDocument response;
    response["success"] = true;
    response["timestamp"] = millis();
    
    if (!data.isNull()) {
        response["data"] = data;
    }
    
    return response;
}

bool FileController::requiresAuthentication(const Utils::Sstring& operation) {
    // List operations can be public for basic file access
    // Upload, delete, and downloads of sensitive files require auth
    return operation != "list" && operation != "info";
}

Response FileController::unauthorizedResponse(Request& request) {
    Utils::SpiJsonDocument error = createErrorResponse("Authentication required", "UNAUTHORIZED");
    return Response(request.getServerRequest())
        .status(401)
        .json(error);
}

Utils::Sstring FileController::formatBytes(size_t bytes) {
    if (bytes < 1024) {
        return Utils::Sstring(bytes) + " B";
    } else if (bytes < 1024 * 1024) {
        return Utils::Sstring(bytes / 1024.0, 1) + " KB";
    } else if (bytes < 1024 * 1024 * 1024) {
        return Utils::Sstring(bytes / (1024.0 * 1024.0), 1) + " MB";
    } else {
        return Utils::Sstring(bytes / (1024.0 * 1024.0 * 1024.0), 1) + " GB";
    }
}