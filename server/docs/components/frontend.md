# Frontend Architecture

This document describes the frontend architecture of the Cozmo Web Interface.

## Overview

The frontend of the Cozmo Web Interface is built using:
- HTML5 templates with Go's template system
- CSS3 with a custom GitHub Dark theme
- jQuery for DOM manipulation and AJAX requests
- Native WebSocket API for real-time communication

## Template Structure

### Layout Template

The layout template (`layout.html`) provides the common structure for all pages:

```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>{{ .title }}</title>
    <link rel="stylesheet" href="/assets/css/github-dark.css">
    <link rel="stylesheet" href="/assets/css/main.css">
    <script src="https://code.jquery.com/jquery-3.7.1.min.js"></script>
</head>
<body class="theme-github-dark">
    <div class="wrapper">
        <header class="header">
            <!-- Header content -->
        </header>

        <main class="main-content">
            <div class="container">
                {{ template "content" . }}
            </div>
        </main>

        <footer class="footer">
            <!-- Footer content -->
        </footer>
    </div>

    <script src="/assets/js/main.js"></script>
    {{ if .pageScript }}
    <script src="/assets/js/{{ .pageScript }}.js"></script>
    {{ end }}
</body>
</html>
```

### Page Templates

Each page has its own template that defines the `content` block:

- `index.html`: Home page
- `login.html`: Login and registration page
- `dashboard.html`: Dashboard for controlling the robot
- `chat.html`: Chat interface for conversing with the AI
- `settings.html`: Settings page for configuring the application

## CSS Architecture

The CSS architecture is based on a component-based approach with a GitHub Dark theme:

### Base Theme

The `github-dark.css` file defines the base theme variables and global styles:

```css
/* GitHub Dark Theme */
:root {
    --color-canvas-default: #0d1117;
    --color-canvas-subtle: #161b22;
    --color-border-default: #30363d;
    /* Other theme variables */
}

body.theme-github-dark {
    background-color: var(--color-canvas-default);
    color: var(--color-fg-default);
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif;
    margin: 0;
    padding: 0;
    line-height: 1.5;
}

/* Component styles */
```

### Component Styles

Styles are organized by component:

- Layout components (header, footer, container)
- Form components (inputs, buttons, switches)
- Card components (dashboard cards, feature cards)
- Message components (chat messages, notifications)
- Control components (control pad, action buttons)

### Responsive Design

The CSS includes media queries for responsive design:

```css
@media (max-width: 768px) {
    .theme-github-dark .chat-container {
        grid-template-columns: 1fr;
    }
    
    /* Other responsive styles */
}
```

## JavaScript Architecture

### Core Functionality

The `main.js` file provides core functionality used across all pages:

- Authentication token management
- AJAX request setup
- Logout functionality
- Protected page access control

```javascript
$(document).ready(function() {
    // Logout functionality
    $('#logout').on('click', function() {
        localStorage.removeItem('auth_token');
        localStorage.removeItem('username');
        window.location.href = '/';
    });

    // Check if token exists for protected pages
    const token = localStorage.getItem('auth_token');
    const currentPath = window.location.pathname;
    const protectedPages = ['/dashboard', '/chat', '/settings'];
    
    if (protectedPages.includes(currentPath) && !token) {
        window.location.href = '/login';
    }

    // Add token to all AJAX requests
    $.ajaxSetup({
        beforeSend: function(xhr) {
            if (token) {
                xhr.setRequestHeader('Authorization', 'Bearer ' + token);
            }
        }
    });
});
```

### Page-Specific JavaScript

Each page includes inline JavaScript for page-specific functionality:

#### Login Page

The login page handles form submission and authentication:

```javascript
// Tab switching
$('.auth-tab').on('click', function() {
    $('.auth-tab').removeClass('active');
    $(this).addClass('active');
    
    const tabId = $(this).data('tab');
    $('.tab-content').addClass('hidden');
    $(`#${tabId}-tab`).removeClass('hidden');
});

// Login form submission
$('#login-form').on('submit', function(e) {
    e.preventDefault();
    
    // Form submission logic
});

// Register form submission
$('#register-form').on('submit', function(e) {
    e.preventDefault();
    
    // Form submission logic
});
```

#### Dashboard Page

The dashboard page handles WebSocket communication and robot control:

```javascript
// Connect to WebSocket
$('#connect-cozmo').on('click', function() {
    // Connection logic
});

function connectWebSocket() {
    // WebSocket connection logic
}

// Control buttons
$('#move-forward').on('mousedown', function() {
    // Control logic
}).on('mouseup mouseleave', function() {
    // Control stop logic
});

// Other control buttons
```

#### Chat Page

The chat page handles message sending and receiving:

```javascript
// Send message
$('#send-message').on('click', function() {
    sendMessage();
});

// Send message function
function sendMessage() {
    const message = $('#chat-input-text').val().trim();
    if (message === '') return;
    
    // Message sending logic
}

// Add message to chat
function addMessage(content, isBot, isError = false) {
    // Message display logic
}
```

#### Settings Page

The settings page handles form submission and settings management:

```javascript
// Update range slider values
$('.range-slider input[type="range"]').on('input', function() {
    $(this).next('.range-value').text($(this).val() + '%');
});

// Save settings
$('#general-settings-form').on('submit', function(e) {
    e.preventDefault();
    
    // Settings saving logic
});
```

## Authentication Flow

1. **Login**:
   - User enters credentials
   - Client sends credentials to server
   - Server validates credentials and returns JWT token
   - Client stores token in localStorage
   - Client redirects to dashboard

2. **Token Usage**:
   - Client includes token in Authorization header for all requests
   - Server validates token for protected routes

3. **Logout**:
   - User clicks logout button
   - Client removes token from localStorage
   - Client redirects to home page

## WebSocket Communication

The WebSocket communication flow:

1. **Connection**:
   - Client connects to WebSocket endpoint
   - Client sends authentication token
   - Server validates token and establishes connection

2. **Sending Commands**:
   - Client sends commands as JSON objects
   - Server processes commands and forwards to Cozmo robot

3. **Receiving Updates**:
   - Server sends updates (camera feed, sensor data) to client
   - Client processes and displays updates

## Data Storage

The frontend uses localStorage for client-side data storage:

- `auth_token`: Authentication token
- `username`: Username of logged-in user
- `theme`: Selected theme
- `voice-enabled`: Setting for voice output
- `cozmo-speak`: Setting for Cozmo speech

## Future Enhancements

- Modular JavaScript architecture using ES6 modules
- TypeScript implementation for better type safety
- React or Vue.js implementation for more interactive UI
- Service workers for offline capabilities
- Progressive Web App (PWA) features
