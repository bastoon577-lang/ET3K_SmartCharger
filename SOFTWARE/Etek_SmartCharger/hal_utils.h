#ifndef __HAL_UTILS__
#define __HAL_UTILS__

#include <ESP8266WiFi.h>
#include <Arduino.h>

#include "et3k_utils.h"

//< Define du HARDWARE
#define HW_NAME                   "ET3K"

//< Define des In/Out
#define ET3K_UART                 Serial                // BUS de communication avec l'ET3K
#define GPIO_RE_DE                0                     // GPIO du RE/DE BUS RS485
#define GPIO_DMD_RESET            2                     // GPIO du bouton Reset

//< Define de bornes de courants de charge VE
#define MINIMAL_CHARGE_CURRENT    6                     // Courant de charge minimal
#define MAXIMAL_CHARGE_CURRENT    32                    // Courant de charge maximal

//< Enumération de l'état de charge de l'EVSE
typedef enum 
{
  evse_Not_Connected,
  evse_Connected,
  evse_Charging,
  evse_Fault,
}CHARGE_STATE_EVSE_e;

//< Structure de données HAL
typedef struct
{
  uint8_t current;                                      // La dernière valeur de courant envoyée
  bool lock;                                            // L'état du blocage ETEK
}HAL_t;

/**
 * \fn void hal_init(void)
 * \brief Fonction permettant d'initialiser la HAL
 */
void hal_init(void);

/**
 * \fn void hal_disable_interrupt(void)
 * \brief Fonction permettant de désactiver les interruptions
 */
void hal_disable_interrupt(void);

/**
 * \fn void hal_enable_interrupt(void)
 * \brief Fonction permettant d'activer les interruptions
 */
void hal_enable_interrupt(void);

/**
 * \fn void hal_evse_init(void)
 * \brief Fonction permettant l'initialisation du service ETEK
 */
void hal_evse_init(void);

/**
 * \fn void hal_evse_update_input(void)
 * \brief Non utilisé sur ET3K
 */
void hal_evse_update_input(void);

/**
 * \fn CHARGE_STATE_EVSE_e hal_evse_get_state(void)
 * \brief Fonction de lecture de l'état de charge de l'ET3K
 */
CHARGE_STATE_EVSE_e hal_evse_get_state(void);

/**
 * \fn void hal_lock_current(void)
 * \brief Fonction permettant le pilotage de la sortie PWM IC à l'état haut
 *        pour permettre le blocage du Viridian
 */
void hal_lock_current(void);

/**
 * \fn void hal_set_current(int current)
 * \brief Fonction permettant le pilotage de la sortie PWM IC du Viridian
 * \param in, la valeur de courant souhaitée
 */
void hal_set_current(uint8_t current);

/**
 * \fn bool hal_get_reboot(void)
 * \brief Fonction permettant de lire l'état de la GPIO de Reboot
 * \return
 *        L'état de la GPIO Reboot
 */
bool hal_get_reboot(void);
 
/**
 * \fn void hal_uart_write(uint8_t *frame, uint8_t size)
 * \brief Fonction permettant d'écrire sur le BUS de communication ET3K
 * \param in, le pointeur vers le buffer de données
 * \param in, la taille des données à transmettre
 */
void hal_uart_write(uint8_t *frame, uint8_t size);

/**
 * \fn void hal_uart_write(uint8_t *frame, uint8_t size)
 * \brief Fonction permettant de lire sur le BUS de communication ET3K
 * \param out, le pointeur vers le buffer de données
 * \param in, la taille des données à lire
 */
int hal_uart_read_byte(uint8_t *frame, uint8_t size);

#endif
