#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>

#include "src/smartcharger/sm_charger_utils.h"
#include "src/smartcharger/websocket_utils.h"
#include "src/smartcharger/eeprom_utils.h"
#include "src/smartcharger/reboot_utils.h"
#include "src/smartcharger/common_sm.h"

#include "src/ihm/configuration_page.h"
#include "src/ihm/exploitation_page.h"
#include "src/ihm/css_steel_sheet.h"
#include "src/ihm/common.h"

#include "common_utils.h"

//< Déclaration des variables globales
static VOLATILE_CONF_FIELDS_t volatile_conf;
static STATIC_CONF_FIELDS_t static_conf;
static ESP8266WebServer web_server(0);
static TIC_CONF_FIELDS_t tic_conf;
static short wifi_equipments;
static DNSServer dns_server;
static bool dhcp_enable;

/**
 * \fn String ip_stringification(uint8_t *in)
 * \brief Fonction de generation du flux String permettant l'affichage d'une adresse IP
 *        sur le serveur Web
 * \param in le pointeur de l'adresse IP
 * \return le flux String de l'adresse IP
 */
static String ip_stringification(uint8_t *in) {
  return String(in[0]) + '.' + String(in[1]) + '.' + String(in[2]) + '.' + String(in[3]);
}

/**
 * \fn void ip_parser(String in, uint8_t *out)
 * \brief Fonction permettant de parser une adresse IP en éléments uint8_t.
 *        Ce process permet le stockage optimisé d'une IP en mémoire EEPROM
 * \param in, la chaine de caractères IP à formater
 * \param out, le pointeur de sortie
 */
static void ip_parser(String in, uint8_t *out) {
  short index = 0;
  for(short i = 0; i < 4; i++) {
    out[i] = in.substring(index,index + 3).toInt();
    index += 4;
  }
}

/**
 * \fn void erase_eeprom_and_reboot(void)
 * \brief Fonction permettant la suppression des données en mémoire EEPROM
 *        permettant un retour en configuration initiale de l'équippement
 */
static void erase_eeprom_and_reboot(void) {
  // Suppression des données en RAM
  memset(&volatile_conf, 0, sizeof(VOLATILE_CONF_FIELDS_t));
  memset(&static_conf, 0, sizeof(STATIC_CONF_FIELDS_t));
  memset(&tic_conf, 0, sizeof(TIC_CONF_FIELDS_t));
  
  // Sauvegarde des données en mémoire EEPROM
  eeprom_write_data(&volatile_conf,EEPROM_VOLATILE_CFG);
  eeprom_write_data(&static_conf,EEPROM_STATIC_CFG);
  eeprom_write_data(&tic_conf,EEPROM_TIC_CFG);

  // Redémarrage de l'équippement pour la prise en compte
  reboot_set();
}

/**
 * \fn void sendContentChunked(const char* p, bool isProgmem = true, size_t chunkSize = 512)
 * \brief Fonction permettant de splitter une chaîne de caractère (RAM ou PROGMEM) au serveur Web par morceaux.
 * \param in, Le pointeur vers la chaine de caractères
 * \param in, is_progmem lorsque la données est en mémoire Flash (PROGMEM / FPSTR / F()) (En PROGMEM par défaut)
 * \param in, chunk_size la taille des blocs à envoyer (512 octets par défaut)
 */
static void sendContentChunked(const char* p, bool is_progmem = true, size_t chunk_size = 512) {
  if (!p) return;

  size_t len=(is_progmem)?strlen_P(p):strlen(p);
  char buffer[chunk_size];

  for(size_t i=0;i<len;i+=chunk_size) {
    size_t chunk=((len-i)<chunk_size)?(len-i):chunk_size;
    
    if(is_progmem) {
      memcpy_P(buffer,p+i,chunk);
      web_server.sendContent(buffer,chunk);
    } else {
      web_server.sendContent(p+i,chunk);
    }
  }
}

/**
 * \fn void html_generate_exploitation_page(void)
 * \brief Fonction de generation du flux HTML/CSS/JS pour la page d'exploitation.
 */
static void html_generate_exploitation_page(void) {
  web_server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  
  sendContentChunked(htmlHeader);
  web_server.sendContent(F("<style>"));
  sendContentChunked(cssSteelSheet);
  web_server.sendContent(F("</style>\n</head>\n<body>"));
  sendContentChunked(htmlPageExploit);
  web_server.sendContent(F("  <script>\n"));

  if(!volatile_conf.theme)
    web_server.sendContent(F("  document.body.classList.add('theme-dark');\n"));

  // Envoi direct des valeurs sans instancier d'objets String
  web_server.sendContent(F("  setCurrent('lim_current',"));
  web_server.sendContent(String(READ_OFFSET_5B(volatile_conf.limite_current)));
  web_server.sendContent(F(");\n  setCurrent('deg_current',"));
  web_server.sendContent(String(READ_OFFSET_5B(volatile_conf.degraded_current)));
  web_server.sendContent(F(");\n"));

  sendContentChunked(scriptsCommon);
  sendContentChunked(scriptsPageExploit);

  // Paramètres généraux
  web_server.sendContent(F("    createTable(\"tab1\");\n"));
  web_server.sendContent(F("    addTableRow(\"tab1\",\"Matériel\",\""));
  web_server.sendContent(HW_NAME);
  web_server.sendContent(F("\");\n    addTableRow(\"tab1\",\"Logiciel\",\""));
  web_server.sendContent(V_LOGICIEL);
  web_server.sendContent(F("\");\n    addTableRow(\"tab1\",\"Réseau électrique\",\""));
  web_server.sendContent(static_conf.which_voltage ? F("400V Triphasés") : F("240V Monophasé"));
  web_server.sendContent(F("\");\n"));

  // Paramètres réseau du SmartCharger
  web_server.sendContent(F("    createTable(\"tab2\");\n"));
  web_server.sendContent(F("    addTableRow(\"tab2\",\"DHCP\",\""));
  web_server.sendContent(dhcp_enable ? F("Actif") : F("Inactif"));
  web_server.sendContent(F("\");\n    addTableRow(\"tab2\",\"Hostname\",\""));
  web_server.sendContent(static_conf.Hostname);
  web_server.sendContent(F("\");\n    addTableRow(\"tab2\",\"MAC\",\""));
  web_server.sendContent(WiFi.macAddress());
  web_server.sendContent(F("\");\n"));

  // IP
  web_server.sendContent(F("    addTableRow(\"tab2\",\"IPv4\",\""));
  if(!static_conf.is_wifi_network_used)
    web_server.sendContent(F("192.168.4.1"));
  else
    web_server.sendContent(dhcp_enable ? WiFi.localIP().toString() : ip_stringification(static_conf.address));
    
  web_server.sendContent(F("\");\n"));

  // Masque de sous-réseau
  web_server.sendContent(F("    addTableRow(\"tab2\",\"Masque de sous réseau\",\""));
  if (!static_conf.is_wifi_network_used)
    web_server.sendContent(F("255.255.255.0"));
  else
    web_server.sendContent(dhcp_enable ? WiFi.subnetMask().toString() : ip_stringification(static_conf.subnet));
    
  web_server.sendContent(F("\");\n"));

  // Passerelle
  web_server.sendContent(F("    addTableRow(\"tab2\",\"Passerelle par défaut\",\""));
  if (!static_conf.is_wifi_network_used)
    web_server.sendContent(F("192.168.4.1"));
  else
    web_server.sendContent(dhcp_enable ? WiFi.gatewayIP().toString() : ip_stringification(static_conf.gateway));

  web_server.sendContent(F("\");\n"));

  // Ports
  web_server.sendContent(F("    addTableRow(\"tab2\",\"Port Web\",\""));
  web_server.sendContent(String(static_conf.port));
  web_server.sendContent(F("\");\n    addTableRow(\"tab2\",\"Port WebSocket\",\""));
  web_server.sendContent(String(static_conf.portWs));
  web_server.sendContent(F("\");\n"));

  // Module TIC
  web_server.sendContent(F("    createTable(\"tab3\");\n"));
  web_server.sendContent(F("    addTableRow(\"tab3\",\"Etat\",\""));
  if (!static_conf.is_tic_module_used)
    web_server.sendContent(F("Inactif"));
  else
    web_server.sendContent(ws_client_is_connected() ? F("Connecté") : F("Déconnecté"));

  web_server.sendContent(F("\");\n"));

  if (static_conf.is_tic_module_used) {
    web_server.sendContent(F("    addTableRow(\"tab3\",\"Hostname / IPv4\",\""));
    web_server.sendContent(tic_conf.ip_or_hostname);
    web_server.sendContent(F("\");\n    addTableRow(\"tab3\",\"Port WebSocket\",\""));
    web_server.sendContent(String(tic_conf.portWs));
    web_server.sendContent(F("\");\n"));
  }

  // Boutons
  if (sm_charger_get_force_charge())                  web_server.sendContent(F("    updateButton(\"force\",{clicked:true});\n"));
  if (volatile_conf.off_peak_hours.off_standards)     web_server.sendContent(F("    updateButton(\"offp0\",{clicked:true});\n"));
  if (volatile_conf.off_peak_hours.off_blues)         web_server.sendContent(F("    updateButton(\"offp1\",{clicked:true});\n"));
  if (volatile_conf.off_peak_hours.off_whites)        web_server.sendContent(F("    updateButton(\"offp2\",{clicked:true});\n"));
  if (volatile_conf.off_peak_hours.off_reds)          web_server.sendContent(F("    updateButton(\"offp3\",{clicked:true});\n"));
  if (volatile_conf.off_peak_hours.off_super)         web_server.sendContent(F("    updateButton(\"offp4\",{clicked:true});\n"));
  if (volatile_conf.off_peak_hours.off_weekends)      web_server.sendContent(F("    updateButton(\"offp5\",{clicked:true});\n"));
  if (volatile_conf.off_peak_hours.off_wednesday)     web_server.sendContent(F("    updateButton(\"offp6\",{clicked:true});\n"));
  if (volatile_conf.solar_active)                     web_server.sendContent(F("    updateButton(\"solar\",{clicked:true});\n"));

  if (!static_conf.is_tic_module_used) {
    web_server.sendContent(F("    document.getElementById(\"deg_sect\").style.display = \"None\";\n"));
    web_server.sendContent(F("    updateButton(\"force\",{hidden:true});\n"));
    web_server.sendContent(F("    updateButton(\"solar\",{hidden:true});\n"));
    web_server.sendContent(F("    removeContainer(3);\n"));
  }

  web_server.sendContent(F("    initWebSocket("));
  web_server.sendContent(String(static_conf.portWs));
  web_server.sendContent(F(");\n  };\n"));
  
  sendContentChunked(scriptsPageExploitExtend);
  web_server.sendContent(F("  </script>\n</body>\n</html>"));
}

/**
 * \fn void html_generate_configuration_page(void)
 * \brief Fonction de generation du flux HTML/CSS/JS pour la page de configuration.
 */
static void html_generate_configuration_page(void) {
  web_server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  
  web_server.sendContent(FPSTR(htmlHeader));
  web_server.sendContent(F("<style>"));
  sendContentChunked(cssSteelSheet);
  web_server.sendContent(F("</style>\n</head>\n<body>"));
  
  sendContentChunked(htmlPageConfig);

  web_server.sendContent(F("  <script>\n  document.body.classList.add('theme-dark');\n"));
  
  sendContentChunked(scriptsCommon);
  sendContentChunked(scriptsPageConfig);

  // Scan et ajout des SSIDs sans aucune instanciation de String
  for(uint8_t i=0;i<wifi_equipments;i++) {
    web_server.sendContent(F("  addWifiSpot(\""));
    web_server.sendContent(WiFi.SSID(i));
    web_server.sendContent(F("\");\n"));
  }

  web_server.sendContent(F("  </script>\n</body>\n</html>"));
}

/**
 * \fn void handle_action_exploitation_button()
 * \brief Fonction permettant la gestion des input de type BOUTON des requetes POST en mode exploitation.
 */
static void handle_action_exploitation_button() {
  String post_data = web_server.arg("data");
  if(post_data.endsWith("theme"))
    volatile_conf.theme = !volatile_conf.theme;
  else if(post_data.startsWith("offp")) {
    OFF_PEAK_HOURS_t off_peak_hours = volatile_conf.off_peak_hours;     // Utilisation d'un variable temporaire évitant de travailler avec volatile_conf
    uint8_t bit_index = post_data.charAt(post_data.length() - 1) - '0'; // Extraction du dernier octet offp '0', '1', '2', ...
    *(uint16_t*)&off_peak_hours ^= (1U << bit_index);                   // Positionnement du bit concerné dans la structure OFF_PEAK_HOURS_t
    off_peak_hours.RUF = 0;                                             // Positionnement du RUF à 0 (Evite les reliquats)
    volatile_conf.off_peak_hours = off_peak_hours;                      // Re-écriture dans la volatile_conf
  } else if(post_data.endsWith("force"))
    sm_charger_set_force_charge(!sm_charger_get_force_charge()); 
  else if(post_data.endsWith("solar"))
    volatile_conf.solar_active = !volatile_conf.solar_active;
  else if(post_data.startsWith("degc")) {
    if(post_data.charAt(post_data.length()-1) == 'm') {
      if(READ_OFFSET_5B(volatile_conf.degraded_current) > MINIMAL_CHARGE_CURRENT)
        volatile_conf.degraded_current--;
    } else {
      if(READ_OFFSET_5B(volatile_conf.degraded_current) < MAXIMAL_CHARGE_CURRENT) 
        volatile_conf.degraded_current++;
     }
  } else if(post_data.startsWith("limc")) {
    if(post_data.charAt(post_data.length()-1) == 'm') {
      if(READ_OFFSET_5B(volatile_conf.limite_current) > MINIMAL_CHARGE_CURRENT)
        volatile_conf.limite_current--;
    } else {
      if(READ_OFFSET_5B(volatile_conf.limite_current) < MAXIMAL_CHARGE_CURRENT) 
        volatile_conf.limite_current++;
     }
  } else if(post_data.endsWith("reset"))
  {
    erase_eeprom_and_reboot();
  }
  // Sauvegarde de la configuration en mémoire EEPROM, ceci n'est pas optimisé car chaque modification entraine une ecriture !
  eeprom_write_data(&volatile_conf,EEPROM_VOLATILE_CFG);  

  web_server.send(200,F("text/html"),"");
}

/**
 * \fn void handle_action_configuration_button()
 * \brief Fonction permettant la gestion des input de type BOUTON des requetes POST en mode configuration.
 */
static void handle_action_configuration_button() {

  String post_data = web_server.arg("data");
  uint8_t conf = post_data.charAt(3)-'0';
  if(post_data.startsWith("vol"))
    static_conf.which_voltage = (conf)?1:0;
  else if(post_data.startsWith("tic"))
    static_conf.is_tic_module_used = (conf)?1:0;
  else if(post_data.startsWith("wif"))
    static_conf.is_wifi_network_used = (conf)?1:0;
  else if(post_data.endsWith("end"))                                    // Signal de fin de configuration
  {
    static_conf.is_configured = 1;                                      // Positionnement du flag de configuration
                                                                        // Sauvegarde des données en mémoire EEPROM
    eeprom_write_data(&static_conf,EEPROM_STATIC_CFG);                  
    eeprom_write_data(&tic_conf,EEPROM_TIC_CFG);
    reboot_set();                                                       // Redémarrage de l'équippement pour la prise en compte
  }
  web_server.send(200,F("text/html"),"");
}

/**
 * \fn void handle_action_configuration_input()
 * \brief Fonction permettant la gestion des input de type TEXT des requetes POST en mode configuration.
 */
static void handle_action_configuration_input() {
  if(web_server.arg("ssid").length() > 0)
    strncpy(static_conf.SmSsid,web_server.arg("ssid").c_str(),sizeof(static_conf.SmSsid)-1);
  else if(web_server.arg("pass").length() > 0)
    strncpy(static_conf.SmPass,web_server.arg("pass").c_str(),sizeof(static_conf.SmPass)-1);
  else if(web_server.arg("hot1").length() > 0)
    strncpy(static_conf.Hostname,web_server.arg("hot1").c_str(),sizeof(static_conf.Hostname)-1);
  else if(web_server.arg("por1").length() > 0)
    static_conf.port = web_server.arg("por1").toInt();
  else if(web_server.arg("por2").length() > 0)
    static_conf.portWs = web_server.arg("por2").toInt();
  else if(web_server.arg("Cha").length() > 0)
  {
    if(web_server.arg("ip").length() == 15)
      ip_parser(web_server.arg("ip"),static_conf.address);
    if(web_server.arg("mask").length() == 15)
      ip_parser(web_server.arg("mask"),static_conf.subnet);
    if(web_server.arg("gateway").length() == 15)
      ip_parser(web_server.arg("gateway"),static_conf.gateway);
  }
  else if(web_server.arg("Tic").length() > 0)
  {
    if(web_server.arg("host").length() > 0)
      strncpy(tic_conf.ip_or_hostname,web_server.arg("host").c_str(),sizeof(tic_conf.ip_or_hostname)-1);
    if(web_server.arg("port").length() > 0)
      tic_conf.portWs = web_server.arg("port").toInt();
  }
  web_server.send(200,F("text/html"),"");
}

void setup() {
  const uint8_t null_ip[4] = {0,0,0,0};                                 // Vecteur d'IP NULL
  dhcp_enable = 0;

  hal_init();                                                           // Initialisation de la HAL
  reboot_init(TIMEOUT_REBOOT);                                          // Initialisation du service Reboot
  eeprom_init();                                                        // Initialisation du service EEPROM
                                                                        // Initialisation des structures en RAM à 0
  memset(&volatile_conf, 0, sizeof(VOLATILE_CONF_FIELDS_t));
  memset(&static_conf, 0, sizeof(STATIC_CONF_FIELDS_t));
  memset(&tic_conf, 0, sizeof(TIC_CONF_FIELDS_t));
                                                                        // Initialisation des structures des données issues de la mémoire EEPROM
  eeprom_read_data(&static_conf,EEPROM_STATIC_CFG);
  eeprom_read_data(&volatile_conf,EEPROM_VOLATILE_CFG);
  eeprom_read_data(&tic_conf,EEPROM_TIC_CFG);
                                                                        // La configuration réseau du SmartCharger est-elle DHCP ?
  if(memcmp(&static_conf.address,null_ip,4) == 0 && 
      memcmp(&static_conf.subnet,null_ip,4) == 0 && 
      memcmp(&static_conf.gateway,null_ip,4) == 0)
    dhcp_enable = 1;
  
  if(static_conf.is_configured)                                         // L'equipement est-il configuré ?
  {
    if(static_conf.is_wifi_network_used)                                // Un reseau Wifi est-il configuré ?
    {
      if(!dhcp_enable)                                                  // Une configuration manuelle est presente ?
        WiFi.config(static_conf.address,static_conf.gateway,static_conf.subnet); // Application de la configuration                                 
      WiFi.begin(static_conf.SmSsid,static_conf.SmPass);                // Connexion au réseau
    }
    else                                                                // Aucun reseau Wifi n'est configué                                              
    {
      WiFi.mode(WIFI_AP);                                               // Forçage en mode Access Point
      WiFi.softAP(static_conf.SmSsid,static_conf.SmPass,AP_CHANNEL,AP_VISIBILITE,AP_MAX_CONN); // Creation du Hotspot    
    }

    // Initialisation de la page d'exploitation
    web_server.on("/", HTTP_GET, []() {
      html_generate_exploitation_page();
    });
    web_server.on("/offp0",HTTP_POST,handle_action_exploitation_button);
    web_server.on("/offp1",HTTP_POST,handle_action_exploitation_button);
    web_server.on("/offp2",HTTP_POST,handle_action_exploitation_button);
    web_server.on("/offp3",HTTP_POST,handle_action_exploitation_button);
    web_server.on("/offp4",HTTP_POST,handle_action_exploitation_button);
    web_server.on("/offp5",HTTP_POST,handle_action_exploitation_button);
    web_server.on("/offp6",HTTP_POST,handle_action_exploitation_button);
    web_server.on("/force",HTTP_POST,handle_action_exploitation_button);
    web_server.on("/solar",HTTP_POST,handle_action_exploitation_button);
    web_server.on("/degcm",HTTP_POST,handle_action_exploitation_button);
    web_server.on("/degcp",HTTP_POST,handle_action_exploitation_button);
    web_server.on("/limcm",HTTP_POST,handle_action_exploitation_button);
    web_server.on("/limcp",HTTP_POST,handle_action_exploitation_button);
    web_server.on("/theme",HTTP_POST,handle_action_exploitation_button);
    web_server.on("/reset",HTTP_POST,handle_action_exploitation_button);

    ws_server_init(static_conf.portWs);                                 // Initialisation du service WebSocket Server
    if(static_conf.is_tic_module_used) {                                // Un Module TIC est-il configuré ?
      ws_client_init(&tic_conf,&static_conf);                           // Initialisation du service WebSocket Client
    }
  }
  else                                                                  // L'equipement est vierge
  {
    IPAddress ap_ip(192,168,4,1);                                       // Forçage de l'IP AP en 192.168.4.1
    WiFi.mode(WIFI_AP);                                                 // Forçage en mode Access Point
    WiFi.softAPConfig(ap_ip,ap_ip,IPAddress(255,255,255,0));            // Configuration de l'AP
    WiFi.softAP(AP_SSID,AP_PASS,AP_CHANNEL,AP_VISIBILITE,AP_MAX_CONN);  // Creation du Hotspot SmartCharger

    dns_server.start(AP_DNS_CAPTIVE_PORTAL,"*",ap_ip);                  // Démarrage du serveur DNS (Redirige * vers l'IP de l'AP)
    
    memcpy(&static_conf.Hostname,AP_SSID,sizeof(AP_SSID));              // Sauvegarde du Hostname par defaut
    memcpy(&static_conf.SmSsid,AP_SSID,sizeof(AP_SSID));                // Sauvegarde du SSID par defaut
    memcpy(&static_conf.SmPass,AP_PASS,sizeof(AP_PASS));                // Sauvegarde du Password par defaut
    static_conf.portWs = 443;                                           // Sauvegarde du Port WebSocket par defaut
    static_conf.port   = 80;                                            // Sauvegarde du Port Web par defaut

    // Initialisation de la page de configuration
    web_server.on("/", HTTP_GET, []() {
      html_generate_configuration_page();
    });
    web_server.on("/vol0",HTTP_POST,handle_action_configuration_button);
    web_server.on("/vol1",HTTP_POST,handle_action_configuration_button);
    web_server.on("/tic0",HTTP_POST,handle_action_configuration_button);
    web_server.on("/tic1",HTTP_POST,handle_action_configuration_button);
    web_server.on("/wif0",HTTP_POST,handle_action_configuration_button);
    web_server.on("/wif1",HTTP_POST,handle_action_configuration_button);
    web_server.on("/adv0",HTTP_POST,handle_action_configuration_input);
    web_server.on("/hot1",HTTP_POST,handle_action_configuration_input);
    web_server.on("/por1",HTTP_POST,handle_action_configuration_input);
    web_server.on("/por2",HTTP_POST,handle_action_configuration_input);
    web_server.on("/ssi1",HTTP_POST,handle_action_configuration_input);
    web_server.on("/pas1",HTTP_POST,handle_action_configuration_input);
    web_server.on("/end",HTTP_POST,handle_action_configuration_button);

    // Redirection des requêtes de détection de portail captif
    web_server.onNotFound([]() {
      web_server.sendHeader("Location","http://192.168.4.1/",true);
      web_server.send(302,"text/plain","");
    });

    wifi_equipments = WiFi.scanNetworks();                              // Scan des reseaux Wifi disponibles
  }
  
  sm_charger_init(&static_conf,&volatile_conf);                         // Initialisation du service Charger
  ArduinoOTA.setHostname(static_conf.Hostname);                         // Initialisation du Hostname OTA
  WiFi.hostname(static_conf.Hostname);                                  // Initialisation du Hostname Web
  web_server.begin(static_conf.port);                                   // Activation du serveur Web
  ArduinoOTA.begin();                                                   // Activation du service OTA
}

void loop() {
  static unsigned long timer_scan_network = millis();
  
  if(!static_conf.is_configured) {                                      // L'équipement n'est pas configuré
    dns_server.processNextRequest();                                    // Traitement des requêtes DNS du portail captif
    if(millis() - timer_scan_network >= TIMEOUT_SCAN_NETWORK) {
      wifi_equipments = WiFi.scanNetworks();                            // Scan des reseaux Wifi disponibles
      timer_scan_network = millis();
    }  
  } else {                                                              // L'équipement est configuré
    if(reboot_get_button_state() == true)                               // Le bouton Reset est appuyé
      erase_eeprom_and_reboot();
    sm_charger_handler();                                               // Handler du service Charger
    ws_handler();                                                       // Handler du service WebSocket
  }
  
  // Néanmoins, dans tous les cas, il est nécessaire d'effectuer les services suivants ...
  web_server.handleClient();                                            // Handler du service Web
  if(!sm_charger_prevent_updates())                                     // La mise à jour est possible ?
    ArduinoOTA.handle();                                                // Handler du service OTA
  reboot_handler();                                                     // Handler du service Reboot
}
