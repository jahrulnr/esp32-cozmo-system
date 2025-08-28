#include "AuthController.h"
#include <SPIFFS.h>

Response AuthController::showLogin(Request& request) {
		// Check if user is already authenticated
		Utils::Sstring token = request.header("Authorization");
		if (token.length() > 0 && token.startsWith("Bearer ")) {
				Utils::Sstring authToken = token.substring(7);
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
		Utils::SpiJsonDocument data;
		data["title"] = "Login";
		data["action"] = "/login";
		data["redirect"] = request.get("redirect", "/dashboard");
		
		return Response(request.getServerRequest())
				.json(data);
}

Response AuthController::login(Request& request) {
		Utils::Sstring username = request.input("username");
		Utils::Sstring password = request.input("password");
		Utils::Sstring redirect = request.input("redirect", "/dashboard");
		
		// Validate input
		if (username.length() == 0 || password.length() == 0) {
				Utils::SpiJsonDocument error;
				error["success"] = false;
				error["message"] = "Username and password are required";
				
				return Response(request.getServerRequest())
						.status(400)
						.json(error);
		}
		
		// Validate credentials
		User* user = nullptr;
		if (!validateCredentials(username, password)) {
				Utils::SpiJsonDocument error;
				error["success"] = false;
				error["message"] = "Invalid username or password";
				
				return Response(request.getServerRequest())
						.status(401)
						.json(error);
		}
		
		// Get user data from database
		user = User::findByUsername(username.c_str());
		
		// Generate JWT token
		Utils::Sstring token = generateToken(username);
		
		Utils::SpiJsonDocument response;
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
		Utils::SpiJsonDocument response;
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
				Utils::Sstring html = file.readString();
				file.close();
				
				return Response(request.getServerRequest())
						.html(html.c_str());
		}
		
		// Fallback dashboard data (for JSON requests)
		Utils::SpiJsonDocument data;
		data["title"] = "Dashboard";
		data["user"]["username"] = "admin";
		data["stats"] = JsonObject();
		data["stats"]["uptime"] = millis();
		data["stats"]["free_heap"] = ESP.getFreeHeap();
		
		return Response(request.getServerRequest())
				.json(data);
}

bool AuthController::validateCredentials(const Utils::Sstring& username, const Utils::Sstring& password) {
		// Find user by username in CSV database
		User* user = User::findByUsername(username.c_str());
		
		if (user == nullptr) {
				return false; // User not found
		}
		
		// Authenticate with password
		bool isValid = user->authenticate(password.c_str());
		
		delete user;
		return isValid;
}

Utils::Sstring AuthController::generateToken(const Utils::Sstring& username) {
		// Simple token generation - in production use proper JWT library
		// For demo, encode username in the token for easy extraction
		Utils::Sstring token = Utils::Sstring("cozmo_token_") + username + '_' + Utils::Sstring(millis());
		// In real implementation, use proper JWT encoding with signature
		return token;
}

bool AuthController::verifyToken(const Utils::Sstring& token) {
		// Simple token verification - in production use proper JWT verification
		return token.startsWith("cozmo_token_") && token.length() > 20;
}

Utils::Sstring AuthController::extractUsernameFromToken(const Utils::Sstring& token) {
		// Simple username extraction - in production parse JWT payload
		if (token.indexOf("cozmo_token_") == 0) {
				// Extract username from token format: cozmo_token_username_timestamp
				int firstUnderscore = token.indexOf('_', 11); // After "cozmo_token_"
				int lastUnderscore = token.toString().lastIndexOf('_');
				
				if (firstUnderscore != -1 && lastUnderscore != -1 && firstUnderscore != lastUnderscore) {
						Utils::Sstring username = token.substring(11, lastUnderscore); // Extract username part\
						return username;
				}
		}
		return "";
}

// Static helper methods for other controllers
Utils::Sstring AuthController::getCurrentUserUsername(Request& request) {
		Utils::Sstring token = request.header("Authorization");
		
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
		int tokenPrefix = Utils::Sstring("cozmo_token_").length();
		int lastUnderscore = token.toString().lastIndexOf('_');
		
		if (lastUnderscore != -1) {
				Utils::Sstring username = token.substring(tokenPrefix, lastUnderscore); // Extract username part
				return username;
		}
		
		return "";
}

User* AuthController::getCurrentUser(Request& request) {
		Utils::Sstring username = getCurrentUserUsername(request);

		if (username.length() == 0) {
				return nullptr;
		}
		
		User* user = User::findByUsername(username.c_str());
		return user;
}

Response AuthController::getUserInfo(Request& request) {
	// Use the same authentication logic as other methods
	User* user = getCurrentUser(request);
	if (user == nullptr) {
		Utils::SpiJsonDocument error;
		error["success"] = false;
		error["message"] = "Authentication required or user not found";
		
		return Response(request.getServerRequest())
			.status(401)
			.json(error);
	}
	
	// Return user info with permissions
	Utils::SpiJsonDocument response;
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
