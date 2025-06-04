/* Main JavaScript */
$(document).ready(function() {
    // Logout functionality
    $('#logout').on('click', function() {
        // Clear local storage
        localStorage.removeItem('auth_token');
        localStorage.removeItem('username');
        
        // Redirect to home page
        window.location.href = '/';
    });

    // Check if token exists and redirect to login if not on protected pages
    const token = localStorage.getItem('auth_token');
    const currentPath = window.location.pathname;
    
    // List of protected pages
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
