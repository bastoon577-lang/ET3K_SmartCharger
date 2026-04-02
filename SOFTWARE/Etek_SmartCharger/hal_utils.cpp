#include "et3k_utils.h"
#include "hal_utils.h"

//< Déclaration des variables globales
static HAL_t hal;

/**
 * \fn void hal_init(void)
 */
void hal_init(void) {
  pinMode(GPIO_DMD_RESET,INPUT_PULLUP);                       // Déclaration de l'entrée GPIO de Reboot
  pinMode(GPIO_RE_DE,OUTPUT);                                 // Configuration du RE/DE BUS RS485
  ET3K_UART.begin(9600);                                      // Initialisation de la communication avec l'ETEK
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
  hal_lock_current();
}

/**
 * \fn void hal_evse_update_input(void)
 */
void hal_evse_update_input(void) {
  // L'IDE Arduino ne gère pas proprement __attribute__((weak))
  // Ne rien faire dans le contexte ET3K
}

/**
 * \fn CHARGE_STATE_EVSE_e hal_evse_get_state(void)
 */
CHARGE_STATE_EVSE_e hal_evse_get_state(void) {
  CHARGE_STATE_ET3K_e et3k_state;
  CHARGE_STATE_EVSE_e ret;
  
  et3k_state = 
        (CHARGE_STATE_ET3K_e)modbus_read_register(1,141);     // Lecture de l'état de l'ET3K

  switch(et3k_state) {                                        // Translation CHARGE_STATE_ET3K_e -> CHARGE_STATE_EVSE_e
    case et3k_ready:
      ret = evse_Not_Connected;
      break;
    case et3k_connected_1:
    case et3k_connected_2:
      ret = evse_Connected;
      break;
    case et3k_charging:
      ret = evse_Charging;
      break;
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
    default:
      ret = evse_Fault;
      break;
  }
  return ret;
}

/**
 * \fn void hal_lock_current(void)
 */
void hal_lock_current(void) {
  if(hal.lock)                                                // L'ETEK est déjà lock
    return;                                                   // Echappement immédiat
    
  modbus_write_register(1,89,2);                              // Envois de la commande de lock
  hal.lock = true;
}

/**
 * \fn void hal_set_current(uint8_t current)
 */
void hal_set_current(uint8_t current) {
  if(current < MINIMAL_CHARGE_CURRENT || current > MAXIMAL_CHARGE_CURRENT)
    return;
    
  if(hal.lock) {                                              // L'ETEK est déjà lock
    modbus_write_register(1,89,1);                            // Unlock de l'ETEK
    hal.lock = false;
    delay(50);
  }

  if(hal.current == current)                                  // Le courant n'a pas changé
    return;                                                   // Echappement
    
  modbus_write_register(1,102,current*100);                   // Envois du courant à l'ET3K
  hal.current = current;                                      // MAJ du courant
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
