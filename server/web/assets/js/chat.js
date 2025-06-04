// Chat specific JavaScript
$(document).ready(function() {
    console.log('Chat script loaded');
    
    // WebSocket connection
    let socket;
    
    // Load notification sounds
    let notificationSound;
    
    // Create audio elements for notifications
    function initializeAudio() {
        // Create the notification audio element
        notificationSound = new Audio();
    }
    
    // Initialize audio on page load
    initializeAudio();
    
    // Initialize WebSocket connection
    function connectWebSocket() {
        // Get the secure protocol based on the current page protocol
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        const token = localStorage.getItem('auth_token');
        
        // Connect to the WebSocket server with the auth token
        socket = new WebSocket(`${protocol}//${window.location.host}/ws/browser?token=${token}`);
        
        socket.onopen = function() {
            console.log('WebSocket connection established');
        };
        
        socket.onmessage = function(event) {
            try {
                const data = JSON.parse(event.data);
                
                // Handle different message types
                switch (data.type) {
                    case 'chat_response':
                        // Handle chat response
                        addMessage(data.data, 'assistant');
                        
                        // Check if TTS is enabled
                        const cozmoSpeak = localStorage.getItem('cozmo-speak') === 'true' || 
                                          localStorage.getItem('cozmo-speak') === null;
                        if (cozmoSpeak) {
                            // Request TTS for the response
                            requestTTS(data.data);
                        } else {
                            // Play notification sound
                            if (notificationSound && notificationSound.src) {
                                notificationSound.play().catch(e => console.log('Audio play error:', e));
                            }
                        }
                        break;
                        
                    case 'tts_response':
                        // Play the generated speech
                        const audio = new Audio(data.data.audioPath);
                        audio.play().catch(e => console.log('TTS audio play error:', e));
                        break;
                        
                    default:
                        console.warn('Unknown WebSocket message type:', data.type);
                }
            } catch (error) {
                console.error('Error parsing WebSocket message:', error);
            }
        };
        
        socket.onclose = function(event) {
            if (event.wasClean) {
                console.log('WebSocket connection closed cleanly');
            } else {
                console.error('WebSocket connection abruptly closed');
                // Try to reconnect after a delay
                setTimeout(connectWebSocket, 3000);
            }
        };
        
        socket.onerror = function(error) {
            console.error('WebSocket error:', error);
        };
    }
    
    // Connect to WebSocket when the page loads
    connectWebSocket();
    
    // Send message function
    function sendMessage() {
        const message = $('#chat-input-text').val().trim();
        if (message && socket && socket.readyState === WebSocket.OPEN) {
            // Add user message to chat
            addMessage(message, 'user');
            
            // Clear input
            $('#chat-input-text').val('');
            
            // Send message via WebSocket
            socket.send(JSON.stringify({
                type: 'chat',
                data: message
            }));
        } else if (!socket || socket.readyState !== WebSocket.OPEN) {
            console.error('WebSocket connection not available');
            alert('Connection lost. Please refresh the page.');
        }
    }
    
    // Request TTS via WebSocket
    function requestTTS(text) {
        if (socket && socket.readyState === WebSocket.OPEN) {
            socket.send(JSON.stringify({
                type: 'tts',
                data: text
            }));
        } else {
            console.error('WebSocket connection not available for TTS request');
        }
    }
    
    // Add message to chat
    function addMessage(text, sender) {
        const $message = $('<div>').addClass('message').addClass(sender);
        const $avatar = $('<div>').addClass('message-avatar');
        const $content = $('<div>').addClass('message-content').text(text);
        
        if (sender === 'user') {
            $avatar.html('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="24" height="24"><path fill="currentColor" d="M12 12c2.21 0 4-1.79 4-4s-1.79-4-4-4-4 1.79-4 4 1.79 4 4 4zm0 2c-2.67 0-8 1.34-8 4v2h16v-2c0-2.66-5.33-4-8-4z"/></svg>');
        } else {
            $avatar.html('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="24" height="24"><path fill="currentColor" d="M12 2a10 10 0 1 0 10 10A10 10 0 0 0 12 2zm0 18a8 8 0 1 1 8-8 8 8 0 0 1-8 8zm4-9h-3V8a1 1 0 0 0-2 0v3H8a1 1 0 0 0 0 2h3v3a1 1 0 0 0 2 0v-3h3a1 1 0 0 0 0-2z"/></svg>');
        }
        
        $message.append($avatar).append($content);
        $('#chat-messages').append($message);
        
        // Scroll to bottom
        $('#chat-messages').scrollTop($('#chat-messages')[0].scrollHeight);
    }
    
    // Event listeners
    $('#send-message').on('click', sendMessage);
    
    $('#chat-input-text').on('keydown', function(e) {
        if (e.keyCode === 13 && !e.shiftKey) {
            e.preventDefault();
            sendMessage();
        }
    });
});
