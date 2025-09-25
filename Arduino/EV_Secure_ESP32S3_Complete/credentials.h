/*
 * WiFi and API Credentials Configuration
 * Update these values for your specific setup
 */

#ifndef CREDENTIALS_H
#define CREDENTIALS_H

// ============================================================================
// WIFI CREDENTIALS - UPDATE THESE FOR YOUR NETWORK
// ============================================================================
#define WIFI_SSID "harshal"                    // Your WiFi network name
#define WIFI_PASSWORD "harshal27"              // Your WiFi password

// ============================================================================
// DASHBOARD API CONFIGURATION
// ============================================================================
#define DASHBOARD_URL "https://ev-secure-dashboard-v2-grf2.vercel.app"  // Your hosted dashboard URL
#define API_KEY "vsr_st001_abc123def456"       // Your station API key (matches dashboard)

// ============================================================================
// DEVICE CONFIGURATION
// ============================================================================
#define DEVICE_ID "ST001"                      // Your station ID (matches API key)
#define STATION_NAME "EV Station 001"          // Human-readable station name
#define LOCATION "Building A, Floor 1"         // Station location

// ============================================================================
// ALTERNATIVE WIFI NETWORKS (Backup)
// ============================================================================
// Uncomment and configure if you have backup networks
// #define BACKUP_WIFI_SSID "backup_network"
// #define BACKUP_WIFI_PASSWORD "backup_password"

// ============================================================================
// API ENDPOINTS
// ============================================================================
#define API_DATA_ENDPOINT "/api/data"
#define API_COMMANDS_ENDPOINT "/api/commands"
#define API_ALERTS_ENDPOINT "/api/alerts"
#define API_STATUS_ENDPOINT "/api/status"

// ============================================================================
// SECURITY SETTINGS
// ============================================================================
#define SSL_ENABLED true                       // Enable HTTPS
#define CERT_VERIFICATION false                // Skip certificate verification for development

#endif // CREDENTIALS_H
