#include "et3k_utils.h"
#include "hal_utils.h"

//< Déclaration des variables globales
static HAL_t hal;

/**
 * \fn void hal_init(void)
 */
void hal_init(void) {
  pinMode(GPIO_DMD_RESET,INPUT_PULLUP);                       // Déclaration de l'entrée GPIO de Reboot
  pinMode(GPIO_LED_IHM,OUTPUT);                               // Configuration de la LED

  ET3K_UART.begin(9600);                                      // Initialisation de la communication avec l'ETEK
}

/**
 * \fn void hal_toggle_led(void)
 */
void hal_toggle_led(void) {
  digitalWrite(GPIO_LED_IHM,!digitalRead(GPIO_LED_IHM));      // Toggle de la LED
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
  // IDE arduino ne permet pas __attribute__((weak))
  // Ne rien faire
}

/**
 * \fn CHARGE_STATE_EVSE_e hal_evse_get_state
 */
CHARGE_STATE_EVSE_e hal_evse_get_state(void) {
  CHARGE_STATE_EVSE_e ret;
  uint16_t state = modbus_read_register(1,141);

  switch(state) {
    case 0:
      return evse_Not_Connected;
    case 1:
    case 2:
      return evse_Connected;
    case 3:
      return evse_Charging;
    default:
      return evse_Fault;
  }
  return evse_Not_Connected;
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

  if(hal.current != current) {                                // Le courant à changé
    modbus_write_register(1,102,current*100);                 // Envois du nouveau courant
    hal.current = current;
  }
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
  ET3K_UART.write(frame,size);
}

/**
 * \fn int hal_uart_read_byte(uint8_t *frame, uint8_t size)
 */
int hal_uart_read_byte(uint8_t *frame, uint8_t size) {
  return ET3K_UART.readBytes(frame,size);
}
