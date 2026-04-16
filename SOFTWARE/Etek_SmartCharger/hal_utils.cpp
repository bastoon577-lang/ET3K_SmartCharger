#include "et3k_utils.h"
#include "hal_utils.h"

/**
 * \fn void hal_init(void)
 */
void hal_init(void) {
  pinMode(GPIO_DMD_RESET,INPUT_PULLUP);                       // Déclaration de l'entrée GPIO de Reboot
  pinMode(GPIO_RE_DE,OUTPUT);                                 // Configuration du RE/DE BUS RS485
  ET3K_UART.begin(9600);                                      // Initialisation de la communication avec l'ETEK
  ET3K_UART.setTimeout(250);                                  // Initialisation du TimeOut de non reception de donnée
}

/**
 * \fn void hal_disable_interrupt(void)
 */
void hal_disable_interrupt(void) {
  noInterrupts();
}

/**
 * \fn void hal_enable_interrupt(void)
 */
void hal_enable_interrupt(void) {
  interrupts();
}

/**
 * \fn void hal_evse_init(void)
 */
void hal_evse_init(void) {
  hal_evse_update_output(0);
}

/**
 * \fn void hal_evse_update_input(void)
 */
void hal_evse_update_input(void) {
  // L'IDE Arduino ne gère pas proprement __attribute__((weak))
  // Ne rien faire dans le contexte ET3K
}

/**
 * \fn void hal_evse_update_output(uint8_t current)
 */
void hal_evse_update_output(uint8_t current) {
  static uint8_t previous_current;
  
  if(current == 0) {                                          // Un blocage est demandé
    modbus_write_register(ET3K_ID,ET3K_SET_START_STOP,
                          et3k_stop);                         // Envois de la commande de blocage à l'ET3K
  } else {                                                    // Autrement, attribution du courant de consigne
    
    // Contrôle du courant de consigne
    if(current < MINIMAL_CHARGE_CURRENT || current > MAXIMAL_CHARGE_CURRENT)
      return;                                                 // Echappement

	  // Déblocage de l'ET3K
    modbus_write_register(ET3K_ID,ET3K_SET_START_STOP,
                          et3k_start);                        // Envois de la commande de déblocage à l'ET3K

    // Attribution du courant de consigne
    if(previous_current != current) {                         // Le courant à changé
	    delay(5);                                               // Attente avant envois d'une nouvelle commande à l'ET3K
      modbus_write_register(ET3K_ID,ET3K_SET_CURRENT,
                            ET3K_CURRENT(current));           // Envois de la commande du courant de consigne à l'ET3K
	    previous_current = current;                             // Sauvegarde du courant
    }
  }
}

/**
 * \fn CHARGE_STATE_EVSE_e hal_evse_get_state(void)
 */
CHARGE_STATE_EVSE_e hal_evse_get_state(void) {
  CHARGE_STATE_ET3K_e et3k_state;
  CHARGE_STATE_EVSE_e ret;
  
  et3k_state = 
        (CHARGE_STATE_ET3K_e)
          modbus_read_register(ET3K_ID,ET3K_GET_STATE);       // Lecture de l'état de l'ET3K

  // Interprêtation de l'état issue de l'énumération CHARGE_STATE_ET3K_e par la translation vers l'énumération CHARGE_STATE_EVSE_e
  switch(et3k_state) {
    case et3k_ready:
      return evse_Not_Connected;
    case et3k_connected_1:
    case et3k_connected_2:
      return evse_Connected;
    case et3k_charging:
      return evse_Charging;
    case et3k_rfid_waiting:
    case et3k_fault_1:
    case et3k_fault_2:
    case et3k_fault_3:
    case et3k_fault_4:
    case et3k_fault_5:
    case et3k_fault_6:
    case et3k_fault_7:
    case et3k_fault_8:
    case et3k_fault_9:
	    return evse_Fault;
    default:
      return evse_Com_Fault;
  }
  return evse_Fault;
}

/**
 * \fn bool hal_get_reboot(void)
 */
bool hal_get_reboot(void) {
  return digitalRead(GPIO_DMD_RESET);
}

/**
 * \fn void hal_uart_write(uint8_t *frame, uint8_t size)
 */
void hal_uart_write(uint8_t *frame, uint8_t size) {
  ET3K_UART.flush();                                          // Flush du buffer d'émission
  while(ET3K_UART.available()) ET3K_UART.read();              // Flush du buffer de réception
  digitalWrite(GPIO_RE_DE,HIGH);                              // Positionnement du RE/DE pour TX
  ET3K_UART.write(frame,size);                                // Envois des données sur le BUS RS485
  delay(10);
  digitalWrite(GPIO_RE_DE,LOW);                               // Positionnement du RE/DE pour RX
}

/**
 * \fn int hal_uart_read_byte(uint8_t *frame, uint8_t size)
 */
int hal_uart_read_byte(uint8_t *frame, uint8_t size) {
  return ET3K_UART.readBytes(frame,size);
}
