// Settings specific JavaScript
$(document).ready(function() {
    console.log('Settings script loaded');
    
    // General settings form submission
    $('#general-settings-form').on('submit', function(e) {
        e.preventDefault();
        
        const theme = $('#theme').val();
        
        // Apply theme
        $('body').removeClass().addClass('theme-' + theme);
        
        // Save to local storage
        localStorage.setItem('theme', theme);
        
        showNotification('General settings saved');
    });
    
    // Robot settings form submission
    $('#robot-settings-form').on('submit', function(e) {
        e.preventDefault();
        
        const motorSpeed = $('#motor-speed').val();
        const volume = $('#volume').val();
        const brightness = $('#brightness').val();
        
        // Save to local storage
        localStorage.setItem('motor-speed', motorSpeed);
        localStorage.setItem('volume', volume);
        localStorage.setItem('brightness', brightness);
        
        showNotification('Robot settings saved');
    });
    
    // Chat settings form submission
    $('#chat-settings-form').on('submit', function(e) {
        e.preventDefault();
        
        const chatModel = $('#chat-model').val();
        const systemPrompt = $('#system-prompt').val();
        const voiceEnabled = $('#voice-enabled').is(':checked');
        
        // Save to local storage
        localStorage.setItem('chat-model', chatModel);
        localStorage.setItem('system-prompt', systemPrompt);
        localStorage.setItem('voice-enabled', voiceEnabled);
        
        showNotification('Chat settings saved');
    });
    
    // Security form submission
    $('#security-form').on('submit', function(e) {
        e.preventDefault();
        
        const currentPassword = $('#current-password').val();
        const newPassword = $('#new-password').val();
        const confirmPassword = $('#confirm-password').val();
        
        if (!currentPassword || !newPassword || !confirmPassword) {
            showNotification('Please fill in all password fields', 'error');
            return;
        }
        
        if (newPassword !== confirmPassword) {
            showNotification('New passwords do not match', 'error');
            return;
        }
        
        // TODO: Send to backend
        // For now, just show success
        $('#current-password').val('');
        $('#new-password').val('');
        $('#confirm-password').val('');
        showNotification('Password changed successfully');
    });
    
    // Helper function to show notifications
    function showNotification(message, type = 'success') {
        const $notification = $('<div>').addClass('notification').addClass(type).text(message);
        
        $('.settings-container').append($notification);
        
        setTimeout(function() {
            $notification.addClass('show');
            
            setTimeout(function() {
                $notification.removeClass('show');
                setTimeout(function() {
                    $notification.remove();
                }, 300);
            }, 2000);
        }, 100);
    }
    
    // Initialize range sliders
    $('.range-slider input[type="range"]').on('input', function() {
        $(this).next('.range-value').text($(this).val() + '%');
    });
    
    // Load saved settings
    const theme = localStorage.getItem('theme') || 'github-dark';
    $('#theme').val(theme);
    
    const motorSpeed = localStorage.getItem('motor-speed') || '50';
    $('#motor-speed').val(motorSpeed).trigger('input');
    
    const volume = localStorage.getItem('volume') || '70';
    $('#volume').val(volume).trigger('input');
    
    const brightness = localStorage.getItem('brightness') || '80';
    $('#brightness').val(brightness).trigger('input');
    
    const chatModel = localStorage.getItem('chat-model') || 'gpt-4.1-nano-2025-04-14';
    $('#chat-model').val(chatModel);
    
    const systemPrompt = localStorage.getItem('system-prompt') || 'You are an AI assistant for the Cozmo robot. You can help control the robot and answer questions about it.';
    $('#system-prompt').val(systemPrompt);
    
    const voiceEnabled = localStorage.getItem('voice-enabled') === 'true';
    $('#voice-enabled').prop('checked', voiceEnabled);
});
