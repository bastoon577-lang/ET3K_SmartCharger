#ifndef __COMMON_UTILS__
#define __COMMON_UTILS__

//< MACRO de version logicielle ETEK
#define V_LOGICIEL                "v1.0.4"              // Version Logicielle

//< MACRO du Mode Access Point
#define AP_SSID                   "SmartCharger"        // SSID SmartCharger Access Point
#define AP_PASS                   AP_SSID               // Password SmartCharger Access Point
#define AP_CHANNEL                0                     // Channel SmartCharger Access Point
#define AP_VISIBILITE             false                 // Visibilite SmartCharger Access Point
#define AP_MAX_CONN               1                     // Nombre de connexion SmartCharger Access Point
#define AP_TCP_PORT               8082                  // Port TCP SmartCharger Access Point
#define AP_DNS_CAPTIVE_PORTAL     53                    // Port DNS du Portail Captif

//< Define des Timeout
#define TIMEOUT_SCAN_NETWORK      300000                // Temps de scrutation des réseaux Wifi disponibles (5 Minutes)
#define TIMEOUT_REBOOT            1000                  // Temps d'anti-rebond pour le reboot (1 Seconde)

#endif
