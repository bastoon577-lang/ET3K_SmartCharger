#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>

#include "src/smartcharger/sm_charger_utils.h"
#include "src/smartcharger/websocket_utils.h"
#include "src/smartcharger/eeprom_utils.h"
#include "src/smartcharger/common_utils.h"
#include "src/smartcharger/reboot_utils.h"

#include "src/ihm/configuration_page.h"
#include "src/ihm/exploitation_page.h"
#include "src/ihm/css_steel_sheet.h"
#include "src/ihm/common.h"

//< Déclaration des variables globales
static VOLATILE_CONF_FIELDS_t volatile_conf;
static STATIC_CONF_FIELDS_t static_conf;
static ESP8266WebServer web_server(80);
static TIC_CONF_FIELDS_t tic_conf;
static short wifi_equipments;
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
 * \fn void html_generate_exploitation_page(void)
 * \brief Fonction de generation du flux HTML/CSS/JS pour la page d'exploitation.
 */
static void html_generate_exploitation_page() {
  web_server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  
  web_server.sendContent(FPSTR(htmlHeader));
  web_server.sendContent(F("<style>"));
  web_server.sendContent(FPSTR(cssSteelSheet));
  web_server.sendContent(F("</style>\n"));
  web_server.sendContent(F("</head>\n"));
  web_server.sendContent(F("<body>"));
  web_server.sendContent(FPSTR(htmlPageExploit));
  web_server.sendContent(F("  <script>\n"));
  if(!volatile_conf.theme)
    web_server.sendContent(F("  document.body.classList.add('theme-dark');\n"));
  web_server.sendContent("  setLimitCurrent('lim_current',"+String(READ_OFFSET_5B(volatile_conf.limited_current))+");");
  web_server.sendContent(FPSTR(scriptsCommon));
  web_server.sendContent(FPSTR(scriptsPageExploit));
  
  // Visualisation des donnees en table
  String str_to_write = "";
  
  // Paramètres généraux
  web_server.sendContent(F("    createTable(\"tab1\");\n"));
  web_server.sendContent("    addTableRow(\"tab1\",\"Matériel\",\""+String(HW_NAME)+"\");\n");
  web_server.sendContent("    addTableRow(\"tab1\",\"Logiciel\",\""+String(V_LOGICIEL)+"\");\n");
  str_to_write = (static_conf.which_voltage)?"400V Triphasé":"230V Monophasé";
  web_server.sendContent("    addTableRow(\"tab1\",\"Réseau électrique\",\""+str_to_write+"\");\n");

  // Paramètres réseau du SmartCharger
  web_server.sendContent(F("    createTable(\"tab2\");\n"));
  str_to_write = (dhcp_enable)?"Actif":"Inactif";
  web_server.sendContent("    addTableRow(\"tab2\",\"DHCP\",\""+str_to_write+"\");\n");
  web_server.sendContent("    addTableRow(\"tab2\",\"Hostname\",\""+String(static_conf.Hostname)+"\");\n");
  web_server.sendContent("    addTableRow(\"tab2\",\"MAC\",\""+String(WiFi.macAddress())+"\");\n");
  str_to_write = (dhcp_enable)?WiFi.localIP().toString():ip_stringification(static_conf.address);
  uint8_t defaultAdr[4] = {192,168,4,1};
  str_to_write = (!static_conf.is_wifi_network_used)?ip_stringification(defaultAdr):str_to_write;
  web_server.sendContent("    addTableRow(\"tab2\",\"IPv4\",\""+str_to_write+"\");\n");
  str_to_write = (dhcp_enable)?WiFi.subnetMask().toString():ip_stringification(static_conf.subnet);
  uint8_t defaultSub[4] = {255,255,255,0};
  str_to_write = (!static_conf.is_wifi_network_used)?ip_stringification(defaultSub):str_to_write;
  web_server.sendContent("    addTableRow(\"tab2\",\"Masque de sous réseau\",\""+str_to_write+"\");\n");
  str_to_write = (dhcp_enable)?WiFi.gatewayIP().toString():ip_stringification(static_conf.gateway);
  uint8_t defaultGat[4] = {192,168,4,1};
  str_to_write = (!static_conf.is_wifi_network_used)?ip_stringification(defaultGat):str_to_write;
  web_server.sendContent("    addTableRow(\"tab2\",\"Passerelle par défaut\",\""+str_to_write+"\");\n");
  web_server.sendContent("    addTableRow(\"tab2\",\"Port Web\",\""+String(static_conf.port)+"\");\n");
  web_server.sendContent("    addTableRow(\"tab2\",\"Port WebSocket\",\""+String(static_conf.portWs)+"\");\n");
  

  // Paramètres réseau du Module TIC
  web_server.sendContent(F("    createTable(\"tab3\");\n"));
  if(!static_conf.is_tic_module_used)
    str_to_write = "Inactif";
  else
    str_to_write = (ws_client_is_connected())?"Connecté":"Déconnecté";
  web_server.sendContent("    addTableRow(\"tab3\",\"Etat\",\""+str_to_write+"\");\n");
  if(static_conf.is_tic_module_used)
  {
    web_server.sendContent("    addTableRow(\"tab3\",\"Hostname / IPv4\",\""+String(tic_conf.ip_or_hostname)+"\");\n");
    web_server.sendContent("    addTableRow(\"tab3\",\"Port WebSocket\",\""+String(tic_conf.portWs)+"\");\n");
  }

  // Gestion des boutons (Changement dynamique par script JS)
  if(volatile_conf.tic_slaved)
    web_server.sendContent(F("    updateButton(\"slave\",{clicked:true});\n"));
  if(volatile_conf.off_peak_hours)
    web_server.sendContent(F("    updateButton(\"offpe\",{clicked:true});\n"));
  if(!static_conf.is_tic_module_used) {
    web_server.sendContent(F("    updateButton(\"slave\",{hidden:true});\n"));
    web_server.sendContent(F("    updateButton(\"offpe\",{hidden:true});\n"));
  }
  web_server.sendContent("    initWebSocket("+String(static_conf.portWs)+");\n");
  web_server.sendContent(F("  };\n"));
  web_server.sendContent(FPSTR(scriptsPageExploitExtend));
  web_server.sendContent(F("  </script>\n"));
  web_server.sendContent(F("</body>\n"));
  web_server.sendContent(F("</html>"));
  web_server.sendContent("");
}

/**
 * \fn void html_generate_configuration_page(void)
 * \brief Fonction de generation du flux HTML/CSS/JS pour la page de configuration.
 */
static void html_generate_configuration_page() {
  web_server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  
  web_server.sendContent(FPSTR(htmlHeader));
  web_server.sendContent(F("<style>"));
  web_server.sendContent(FPSTR(cssSteelSheet));
  web_server.sendContent(F("</style>\n"));
  web_server.sendContent(F("</head>\n"));
  web_server.sendContent(F("<body>"));
  web_server.sendContent(FPSTR(htmlPageConfig));
  web_server.sendContent(F("  <script>\n"));
  web_server.sendContent(F("  document.body.classList.add('theme-dark');"));
  web_server.sendContent(FPSTR(scriptsCommon));
  web_server.sendContent(FPSTR(scriptsPageConfig));
  for(uint8_t i=0;i<wifi_equipments;i++)
    web_server.sendContent("    addWifiSpot(\""+WiFi.SSID(i)+"\");\n");
  web_server.sendContent(F("  </script>\n"));
  web_server.sendContent(F("</body>\n"));
  web_server.sendContent(F("</html>"));
}

/**
 * \fn void handle_action_exploitation_button()
 * \brief Fonction permettant la gestion des input de type BOUTON des requetes POST en mode exploitation.
 */
static void handle_action_exploitation_button() {
  String post_data = web_server.arg("data");
  if(post_data.endsWith("theme"))
    volatile_conf.theme = !volatile_conf.theme;
  else if(post_data.endsWith("slave"))
    volatile_conf.tic_slaved = !volatile_conf.tic_slaved;
  else if(post_data.endsWith("offpe"))
    volatile_conf.off_peak_hours = !volatile_conf.off_peak_hours;
  else if(post_data.endsWith("limcm")) {
    if(READ_OFFSET_5B(volatile_conf.limited_current) > MINIMAL_CHARGE_CURRENT)
      volatile_conf.limited_current--;
  }
  else if(post_data.endsWith("limcp")) {
    if(READ_OFFSET_5B(volatile_conf.limited_current) < MAXIMAL_CHARGE_CURRENT) 
      volatile_conf.limited_current++;
  }
  else if(post_data.endsWith("reset"))
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
  const uint8_t null_ip[4] = {0,0,0,0};
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
      WiFi.softAP(static_conf.SmSsid,static_conf.SmPass,AP_CHANNEL,AP_VISIBILITE,AP_MAX_CONN); // Creation du Hotspot    

    // Initialisation de la page d'exploitation
    web_server.on("/", HTTP_GET, []() {
      html_generate_exploitation_page();
    });
    web_server.on("/slave",HTTP_POST,handle_action_exploitation_button);
    web_server.on("/offpe",HTTP_POST,handle_action_exploitation_button);
    web_server.on("/limcm",HTTP_POST,handle_action_exploitation_button);
    web_server.on("/limcp",HTTP_POST,handle_action_exploitation_button);
    web_server.on("/theme",HTTP_POST,handle_action_exploitation_button);
    web_server.on("/reset",HTTP_POST,handle_action_exploitation_button);

    ws_server_init(static_conf.portWs);                                 // Initialisation du service WebSocket Server
    if(static_conf.is_tic_module_used) {                                // Un Module TIC est-il configuré ?
      ws_client_init(&tic_conf);                                        // Initialisation du service WebSocket Client
    }
  }
  else                                                                  // L'equipement est vierge
  {
    WiFi.softAP(AP_SSID,AP_PASS,AP_CHANNEL,AP_VISIBILITE,AP_MAX_CONN);  // Creation du Hotspot SmartCharger
    memcpy(&static_conf.Hostname,AP_SSID,sizeof(AP_SSID));              // Sauvegarde du Hostname par defaut
    memcpy(&static_conf.SmSsid,AP_SSID,sizeof(AP_SSID));                // Sauvegarde du SSID par defaut
    memcpy(&static_conf.SmPass,AP_PASS,sizeof(AP_PASS));                // Sauvegarde du Password par defaut
    static_conf.portWs = 81;                                            // Sauvegarde du Port WebSocket par defaut
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
    web_server.on("/ssi1",HTTP_POST,handle_action_configuration_input);
    web_server.on("/pas1",HTTP_POST,handle_action_configuration_input);
    web_server.on("/end",HTTP_POST,handle_action_configuration_button);

    wifi_equipments = WiFi.scanNetworks();                              // Scan des reseaux Wifi disponibles
  }
  
  sm_charger_init(&static_conf,&volatile_conf);                         // Initialisation du service Charger
  ArduinoOTA.setHostname(static_conf.Hostname);                         // Initialisation du Hostname OTA
  WiFi.hostname(static_conf.Hostname);                                  // Initialisation du Hostname Web
  web_server.begin();                                                   // Activation du serveur Web
  ArduinoOTA.begin();                                                   // Activation du service OTA
}

void loop() {
  static unsigned long timer_scan_network = millis();
  
  if(!static_conf.is_configured) {                                      // L'équipement n'est pas configuré
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
  ArduinoOTA.handle();                                                  // Handler du service OTA
  reboot_handler();                                                     // Handler du service Reboot
}
