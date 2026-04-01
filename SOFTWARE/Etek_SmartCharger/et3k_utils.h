#ifndef __ET3K_UTILS__
#define __ET3K_UTILS__

#include <stdint.h>
#include "hal_utils.h"

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
