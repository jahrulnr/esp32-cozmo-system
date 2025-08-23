#include "AuthController.h"
#include <SPIFFS.h>

Response AuthController::showLogin(Request& request) {
		// Check if user is already authenticated
		String token = request.header("Authorization");
		if (token.length() > 0 && token.startsWith("Bearer ")) {
				String authToken = token.substring(7);
				if (verifyToken(authToken)) {
						return Response(request.getServerRequest())
								.redirect("/dashboard");
				}
		}
		
		// Serve login page from SPIFFS
		if (SPIFFS.exists("/views/login.html")) {
				return Response(request.getServerRequest())
						.file("/views/login.html");
		}
		
		// Fallback if file doesn't exist
		JsonDocument data;
		data["title"] = "Login";
		data["action"] = "/login";
		data["redirect"] = request.get("redirect", "/dashboard");
		
		return Response(request.getServerRequest())
				.json(data);
}

Response AuthController::login(Request& request) {
		String username = request.input("username");
		String password = request.input("password");
		String redirect = request.input("redirect", "/dashboard");
		
		// Validate input
		if (username.length() == 0 || password.length() == 0) {
				JsonDocument error;
				error["success"] = false;
				error["message"] = "Username and password are required";
				
				return Response(request.getServerRequest())
						.status(400)
						.json(error);
		}
		
		// Validate credentials
		User* user = nullptr;
		if (!validateCredentials(username, password)) {
				JsonDocument error;
				error["success"] = false;
				error["message"] = "Invalid username or password";
				
				return Response(request.getServerRequest())
						.status(401)
						.json(error);
		}
		
		// Get user data from database
		user = User::findByUsername(username);
		
		// Generate JWT token
		String token = generateToken(username);
		
		JsonDocument response;
		response["success"] = true;
		response["message"] = "Login successful";
		response["token"] = token;
		response["redirect"] = redirect;
		response["user"]["username"] = user ? user->getUsername() : username;
		
		if (user) {
				delete user;
		}
		
		return Response(request.getServerRequest())
				.json(response);
}

Response AuthController::logout(Request& request) {
		// In a real implementation, you might want to blacklist the token
		JsonDocument response;
		response["success"] = true;
		response["message"] = "Logged out successfully";
		response["redirect"] = "/login";
		
		return Response(request.getServerRequest())
				.json(response);
}

Response AuthController::dashboard(Request& request) {
		// For web requests, let the client-side JavaScript handle authentication
		// The HTML page will check localStorage for token and redirect if needed
		
		// Serve dashboard page from SPIFFS
		if (SPIFFS.exists("/views/dashboard.html")) {
				File file = SPIFFS.open("/views/dashboard.html", "r");
				String html = file.readString();
				file.close();
				
				return Response(request.getServerRequest())
						.html(html);
		}
		
		// Fallback dashboard data (for JSON requests)
		JsonDocument data;
		data["title"] = "Dashboard";
		data["user"]["username"] = "admin";
		data["stats"] = JsonObject();
		data["stats"]["uptime"] = millis();
		data["stats"]["free_heap"] = ESP.getFreeHeap();
		
		return Response(request.getServerRequest())
				.json(data);
}

bool AuthController::validateCredentials(const String& username, const String& password) {
		// Find user by username in CSV database
		User* user = User::findByUsername(username);
		
		if (user == nullptr) {
				return false; // User not found
		}
		
		// Authenticate with password
		bool isValid = user->authenticate(password);
		
		delete user;
		return isValid;
}

String AuthController::generateToken(const String& username) {
		// Simple token generation - in production use proper JWT library
		// For demo, encode username in the token for easy extraction
		String token = "cozmo_token_" + username + "_" + String(millis());
		// In real implementation, use proper JWT encoding with signature
		return token;
}

bool AuthController::verifyToken(const String& token) {
		// Simple token verification - in production use proper JWT verification
		return token.startsWith("cozmo_token_") && token.length() > 20;
}

String AuthController::extractUsernameFromToken(const String& token) {
		// Simple username extraction - in production parse JWT payload
		if (token.indexOf("cozmo_token_") == 0) {
				// Extract username from token format: cozmo_token_username_timestamp
				int firstUnderscore = token.indexOf('_', 11); // After "cozmo_token_"
				int lastUnderscore = token.lastIndexOf('_');
				
				if (firstUnderscore != -1 && lastUnderscore != -1 && firstUnderscore != lastUnderscore) {
						String username = token.substring(11, lastUnderscore); // Extract username part\
						return username;
				}
		}
		return "";
}

// Static helper methods for other controllers
String AuthController::getCurrentUserUsername(Request& request) {
		String token = request.header("Authorization");
		
		if (token.length() == 0) {
				return "";
		}
		
		if (token.startsWith("Bearer ")) {
				token = token.substring(7);
		}
		
		// Simple token verification - in production use proper JWT
		if (!token.startsWith("cozmo_token_")) {
				return "";
		}
		
		// Extract username from token format: cozmo_token_username_timestamp
		int tokenPrefix = String("cozmo_token_").length();
		int lastUnderscore = token.lastIndexOf('_');
		
		if (lastUnderscore != -1) {
				String username = token.substring(tokenPrefix, lastUnderscore); // Extract username part
				return username;
		}
		
		return "";
}

User* AuthController::getCurrentUser(Request& request) {
		String username = getCurrentUserUsername(request);

		if (username.length() == 0) {
				return nullptr;
		}
		
		User* user = User::findByUsername(username);
		return user;
}

Response AuthController::getUserInfo(Request& request) {
	// Use the same authentication logic as other methods
	User* user = getCurrentUser(request);
	if (user == nullptr) {
		JsonDocument error;
		error["success"] = false;
		error["message"] = "Authentication required or user not found";
		
		return Response(request.getServerRequest())
			.status(401)
			.json(error);
	}
	
	// Return user info with permissions
	JsonDocument response;
	response["success"] = true;
	response["user"]["username"] = user->getUsername();
	
	// For demo purposes, check if username is admin
	bool isAdmin = (user->getUsername() == "admin");
	response["user"]["permissions"]["canManageUsers"] = isAdmin;
	response["user"]["permissions"]["canRestartSystem"] = isAdmin;
	response["user"]["role"] = isAdmin ? "admin" : "user";
	
	delete user;
	
	return Response(request.getServerRequest())
		.json(response);
}
