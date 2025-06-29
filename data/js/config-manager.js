/**
 * Config Manager
 * 
 * This module handles loading, editing and saving device configuration
 * from the web interface.
 */

class ConfigManager {
  constructor() {
    this.config = null;
    this.originalConfig = null;
    this.configSections = {
      'general': [
        'development',
        'cozmo.protect',
        'cozmo.automation',
        'misc'
      ],
      'motors': [
        'motor',
        'servo'
      ],
      'sensors': [
        'camera',
        'screen',
        'orientation',
        'cliff_detector',
        'ultrasonic'
      ],
      'connectivity': [
        'wifi',
        'webserver',
        'websocket',
        'gpt'
      ],
      'advanced': [
        'health_check'
      ]
    };
    
    this.init();
  }
  
  async init() {
    this.bindUIEvents();
    await this.loadConfig();
    this.renderConfigUI();
  }
  
  bindUIEvents() {
    // Tab navigation
    document.querySelectorAll('.config-tab-button').forEach(button => {
      button.addEventListener('click', (e) => {
        // Hide all content
        document.querySelectorAll('.config-tab-content').forEach(content => {
          content.classList.remove('active');
        });
        
        // Deactivate all buttons
        document.querySelectorAll('.config-tab-button').forEach(btn => {
          btn.classList.remove('active');
        });
        
        // Activate clicked button and content
        button.classList.add('active');
        const tabId = button.getAttribute('data-tab');
        document.getElementById(tabId).classList.add('active');
      });
    });
    
    // Save button
    const saveButton = document.getElementById('save-config');
    if (saveButton) {
      saveButton.addEventListener('click', async () => {
        await this.saveConfig();
      });
    }
    
    // Reset button
    const resetButton = document.getElementById('reset-config');
    if (resetButton) {
      resetButton.addEventListener('click', () => {
        if (confirm('Are you sure you want to reset to the default configuration? This will discard all changes.')) {
          this.resetConfig();
        }
      });
    }
  }
  
  async loadConfig() {
    try {
      const response = await fetch('/api/config');
      if (!response.ok) {
        throw new Error(`Failed to load config: ${response.status} ${response.statusText}`);
      }
      
      this.config = await response.json();
      this.originalConfig = JSON.parse(JSON.stringify(this.config)); // Deep copy
      console.log('Config loaded successfully:', this.config);
    } catch (error) {
      console.error('Error loading config:', error);
      showToast('Failed to load configuration', 'error');
    }
  }
  
  async saveConfig() {
    try {
      // Collect values from form fields
      this.updateConfigFromForm();
      
      const response = await fetch('/api/config', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(this.config)
      });
      
      if (!response.ok) {
        throw new Error(`Failed to save config: ${response.status} ${response.statusText}`);
      }
      
      const result = await response.json();
      console.log('Config saved successfully:', result);
      showToast('Configuration saved successfully', 'success');
      
      // Update original config with new values
      this.originalConfig = JSON.parse(JSON.stringify(this.config));
    } catch (error) {
      console.error('Error saving config:', error);
      showToast('Failed to save configuration', 'error');
    }
  }
  
  updateConfigFromForm() {
    // Find all inputs with data-config-path attribute
    document.querySelectorAll('[data-config-path]').forEach(input => {
      const path = input.getAttribute('data-config-path');
      let value;
      
      if (input.type === 'checkbox') {
        value = input.checked;
      } else if (input.type === 'number') {
        value = parseFloat(input.value);
        if (isNaN(value)) {
          if (input.hasAttribute('min')) {
            value = parseFloat(input.getAttribute('min')) || 0;
          } else {
            value = 0;
          }
        }
      } else {
        value = input.value;
      }
      
      this.setNestedProperty(this.config, path, value);
    });
  }
  
  resetConfig() {
    if (this.originalConfig) {
      this.config = JSON.parse(JSON.stringify(this.originalConfig));
      this.renderConfigUI();
      showToast('Configuration reset to original values', 'info');
    }
  }
  
  renderConfigUI() {
    if (!this.config) return;
    
    // Render each tab's content
    Object.entries(this.configSections).forEach(([tabName, sections]) => {
      const tabContentEl = document.getElementById(`tab-${tabName}`);
      if (!tabContentEl) return;
      
      tabContentEl.innerHTML = '';
      
      sections.forEach(section => {
        const sectionEl = this.renderConfigSection(section);
        if (sectionEl) {
          tabContentEl.appendChild(sectionEl);
        }
      });
    });
  }
  
  renderConfigSection(sectionPath) {
    const sectionConfig = this.getNestedProperty(this.config, sectionPath);
    if (!sectionConfig) return null;
    
    const sectionDiv = document.createElement('div');
    sectionDiv.className = 'config-section';
    
    // Create section title
    const title = document.createElement('h3');
    title.className = 'config-section-title';
    title.textContent = this.formatSectionTitle(sectionPath);
    sectionDiv.appendChild(title);
    
    // If section is an object with nested properties
    if (typeof sectionConfig === 'object' && !Array.isArray(sectionConfig)) {
      Object.entries(sectionConfig).forEach(([key, value]) => {
        const fullPath = `${sectionPath}.${key}`;
        
        if (typeof value === 'object' && !Array.isArray(value)) {
          // Nested group
          const groupDiv = document.createElement('div');
          groupDiv.className = 'config-group';
          
          // Group title
          const groupTitle = document.createElement('h4');
          groupTitle.textContent = this.formatTitle(key);
          groupDiv.appendChild(groupTitle);
          
          // Render each property in the nested group
          Object.entries(value).forEach(([nestedKey, nestedValue]) => {
            const nestedPath = `${fullPath}.${nestedKey}`;
            const controlDiv = this.createControl(nestedKey, nestedValue, nestedPath);
            if (controlDiv) {
              groupDiv.appendChild(controlDiv);
            }
          });
          
          sectionDiv.appendChild(groupDiv);
        } else {
          // Single property
          const controlDiv = this.createControl(key, value, fullPath);
          if (controlDiv) {
            sectionDiv.appendChild(controlDiv);
          }
        }
      });
    }
    
    return sectionDiv;
  }
  
  createControl(key, value, path) {
    const controlDiv = document.createElement('div');
    const isEnabled = key === 'enabled';
    
    if (isEnabled) {
      controlDiv.className = 'config-item-inline';
      
      const label = document.createElement('label');
      label.htmlFor = path.replace(/\./g, '_');
      label.textContent = 'Enabled';
      
      const checkbox = document.createElement('input');
      checkbox.type = 'checkbox';
      checkbox.id = path.replace(/\./g, '_');
      checkbox.checked = value;
      checkbox.setAttribute('data-config-path', path);
      
      controlDiv.appendChild(label);
      controlDiv.appendChild(checkbox);
    } else if (typeof value === 'boolean') {
      controlDiv.className = 'config-item-inline';
      
      const label = document.createElement('label');
      label.htmlFor = path.replace(/\./g, '_');
      label.textContent = this.formatTitle(key);
      
      const checkbox = document.createElement('input');
      checkbox.type = 'checkbox';
      checkbox.id = path.replace(/\./g, '_');
      checkbox.checked = value;
      checkbox.setAttribute('data-config-path', path);
      
      controlDiv.appendChild(label);
      controlDiv.appendChild(checkbox);
    } else {
      controlDiv.className = 'config-item';
      
      const label = document.createElement('label');
      label.htmlFor = path.replace(/\./g, '_');
      label.textContent = this.formatTitle(key);
      
      let input;
      if (typeof value === 'number') {
        input = document.createElement('input');
        input.type = 'number';
        input.value = value;
        input.step = key.includes('angle') ? '1' : key.includes('timeout') || key.includes('interval') ? '100' : '0.1';
      } else {
        input = document.createElement('input');
        input.type = 'text';
        input.value = value;
        
        // Special case for protected fields
        if (key.includes('password') || key.includes('api_key')) {
          input.type = 'password';
          
          // Add toggle to show/hide password
          const toggleBtn = document.createElement('button');
          toggleBtn.type = 'button';
          toggleBtn.className = 'btn btn-sm';
          toggleBtn.innerHTML = '<i class="fas fa-eye"></i>';
          toggleBtn.style.position = 'absolute';
          toggleBtn.style.right = '10px';
          toggleBtn.style.top = '50%';
          toggleBtn.style.transform = 'translateY(-50%)';
          
          toggleBtn.addEventListener('click', () => {
            if (input.type === 'password') {
              input.type = 'text';
              toggleBtn.innerHTML = '<i class="fas fa-eye-slash"></i>';
            } else {
              input.type = 'password';
              toggleBtn.innerHTML = '<i class="fas fa-eye"></i>';
            }
          });
          
          const inputWrapper = document.createElement('div');
          inputWrapper.style.position = 'relative';
          inputWrapper.appendChild(input);
          inputWrapper.appendChild(toggleBtn);
          
          controlDiv.appendChild(label);
          controlDiv.appendChild(inputWrapper);
          input.setAttribute('data-config-path', path);
          
          return controlDiv;
        }
      }
      
      input.id = path.replace(/\./g, '_');
      input.className = 'form-control';
      input.setAttribute('data-config-path', path);
      
      controlDiv.appendChild(label);
      controlDiv.appendChild(input);
      
      // Add description if needed for special fields
      if (this.getFieldDescription(key)) {
        const description = document.createElement('div');
        description.className = 'config-description';
        description.textContent = this.getFieldDescription(key);
        controlDiv.appendChild(description);
      }
    }
    
    return controlDiv;
  }
  
  getFieldDescription(key) {
    const descriptions = {
      'api_key': 'API key for authenticating with the service',
      'port': 'Port number for the server (1-65535)',
      'trigger_pin': 'GPIO pin number for the trigger signal',
      'echo_pin': 'GPIO pin number for the echo signal',
      'obstacle_threshold': 'Distance in cm to consider as an obstacle',
      'inactivity_timeout': 'Time in milliseconds before considering inactive',
      'check_interval': 'Interval in milliseconds between checks',
      'max_behaviors': 'Maximum number of behaviors to store',
      'max_tokens': 'Maximum tokens to generate in a response',
      'temperature': 'Randomness of the output (0.0-1.0)',
    };
    
    return descriptions[key] || null;
  }
  
  formatSectionTitle(path) {
    const lastPart = path.split('.').pop();
    return this.formatTitle(lastPart);
  }
  
  formatTitle(str) {
    // Convert snake_case to Title Case
    return str
      .split('_')
      .map(word => word.charAt(0).toUpperCase() + word.slice(1))
      .join(' ');
  }
  
  getNestedProperty(obj, path) {
    return path.split('.').reduce((acc, part) => {
      return acc && acc[part] !== undefined ? acc[part] : null;
    }, obj);
  }
  
  setNestedProperty(obj, path, value) {
    const parts = path.split('.');
    let current = obj;
    
    for (let i = 0; i < parts.length - 1; i++) {
      const part = parts[i];
      if (!(part in current)) {
        current[part] = {};
      }
      current = current[part];
    }
    
    current[parts[parts.length - 1]] = value;
  }
}

// Initialize the config manager when the page loads
document.addEventListener('DOMContentLoaded', function() {
  window.configManager = new ConfigManager();
});

// Helper to show toast notifications
function showToast(message, type = 'info') {
  // Check if toast container exists, if not create it
  let toastContainer = document.getElementById('toast-container');
  if (!toastContainer) {
    toastContainer = document.createElement('div');
    toastContainer.id = 'toast-container';
    toastContainer.style.position = 'fixed';
    toastContainer.style.bottom = '20px';
    toastContainer.style.right = '20px';
    toastContainer.style.zIndex = '9999';
    document.body.appendChild(toastContainer);
  }
  
  // Create toast element
  const toast = document.createElement('div');
  toast.className = `toast toast-${type}`;
  toast.textContent = message;
  toast.style.backgroundColor = type === 'error' ? 'var(--color-danger)' : 
                              type === 'success' ? 'var(--color-success)' :
                              'var(--color-info)';
  toast.style.color = '#fff';
  toast.style.padding = '12px 16px';
  toast.style.borderRadius = '4px';
  toast.style.marginTop = '8px';
  toast.style.boxShadow = 'var(--shadow)';
  toast.style.minWidth = '250px';
  toast.style.opacity = '0';
  toast.style.transition = 'opacity 0.3s ease';
  
  // Add to container
  toastContainer.appendChild(toast);
  
  // Show toast with animation
  setTimeout(() => {
    toast.style.opacity = '1';
  }, 10);
  
  // Remove after 3 seconds
  setTimeout(() => {
    toast.style.opacity = '0';
    setTimeout(() => {
      if (toast.parentNode) {
        toast.parentNode.removeChild(toast);
      }
    }, 300);
  }, 3000);
}
