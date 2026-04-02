#ifndef __ET3K_UTILS__
#define __ET3K_UTILS__

#include <stdint.h>
#include "hal_utils.h"

//< Enumération de l'état de charge de l'ET3K
typedef enum 
{
  et3k_fault_1          = 0,    // (FAULT) Power self-check failed
  et3k_ready            = 1,
  et3k_rfid_waiting     = 2,    // (FAULT) Waiting RFID validation
  et3k_connected_1      = 3,
  et3k_connected_2      = 4,
  et3k_charging         = 5,
  et3k_fault_2          = 6,    // (FAULT) Need ventilation
  et3k_fault_3          = 7,    // (FAULT) CP/PE short circuits
  et3k_fault_4          = 8,    // (FAULT) RCMU leakage or self-inspection failure
  et3k_fault_5          = 9,    // (FAULT) EV Charging Socket fault
  et3k_fault_6          = 10,   // (FAULT) Split PP Wire
  et3k_fault_7          = 11,   // (FAULT) Electronic lock disable
  et3k_fault_8          = 12,   // (FAULT) RFID Card is not Valid
  et3k_fault_9          = 13,   // (FAULT) Circuit Overload
}CHARGE_STATE_ET3K_e;

/**
 * \fn uint16_t modbus_compute_crc(uint8_t *buf, uint8_t len)
 * \brief Fonction permettant le calcul du CRC d'une trame MODBUS
 * \param in, le Buffer à intégrer
 * \param in, la taille des données à intégrer
 * \return
 *    Le CRC calculé sur la trame
 */
uint16_t modbus_compute_crc(uint8_t *buf, uint8_t len);

/**
 * \fn uint16_t modbus_read_register(uint8_t slave, uint16_t reg)
 * \brief Fonction permettant de lire un registre de l'ETEK
 * \param in, le slave ID
 * \param in, le numéro de registre
 * \return
 *    La valeur contenue dans le registre
 */
uint16_t modbus_read_register(uint8_t slave, uint16_t reg);

/**
 * \fn void modbus_write_register(uint8_t slave, uint16_t reg, uint16_t value)
 * \brief Fonction permettant d'écrire dans un registre de l'ETEK
 * \param in, le slave ID
 * \param in, le numéro de registre
 * \param in, la valeur à écrire
 */
void modbus_write_register(uint8_t slave, uint16_t reg, uint16_t value);

#endif
