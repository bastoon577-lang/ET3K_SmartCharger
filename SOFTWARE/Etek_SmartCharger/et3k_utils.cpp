#include "et3k_utils.h"

/**
 * \fn uint16_t modbus_compute_crc(uint8_t *buf, uint8_t len)
 */
uint16_t modbus_compute_crc(uint8_t *buf, uint8_t len) {
  uint16_t crc = 0xFFFF;
  for(uint8_t pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)buf[pos];
    for(uint8_t i = 0; i < 8; i++) {
      if (crc & 0x0001) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

/**
 * \fn uint16_t modbus_read_register(uint8_t slave, uint16_t reg)
 */
uint16_t modbus_read_register(uint8_t slave, uint16_t reg) {
  uint16_t computed_crc;
  uint8_t frame[8];
  uint8_t resp[7];
  int len;

  frame[0] = slave;                                           // Ajout de l'ID Slave
  frame[1] = 0x03;                                            // Ajout de l'Opcode de Lecture Registre
  frame[2] = reg >> 8;                                        // Ajout du numéro de Registre (MSB)
  frame[3] = reg & 0xFF;                                      // Ajout du numéro de Registre (LSB)
  frame[4] = 0x00;                                            // Ajout de la taille à lire (MSB)
  frame[5] = 0x01;                                            // Ajout de la taille à lire (LSB)
  
  computed_crc = modbus_compute_crc(frame,6);                 // Calcul du CRC sur le jeu de données
  frame[6] = computed_crc & 0xFF;                             // Ajout du CRC (MSB)
  frame[7] = computed_crc >> 8;                               // Ajout du CRC (LSB)
  
  hal_uart_write((uint8_t *)frame,8);                         // Envois sur le BUS RS485
  
  len = hal_uart_read_byte((uint8_t *)resp,7);                // Lecture du BUS RS485
  
  if(len == 7 && resp[1] == 0x03) {                           // Contrôle de taille et d'Opcode
    return (resp[3] << 8) | resp[4];                          // Retour de la réponse
  }
  return 0xFFFF;                                              // Retour d'erreur
}

/**
 * \fn void modbus_write_register(uint8_t slave, uint16_t reg, uint16_t value)
 */
void modbus_write_register(uint8_t slave, uint16_t reg, uint16_t value) {
  uint16_t computed_crc;
  uint8_t frame[8];
  
  frame[0] = slave;                                           // Ajout de l'ID Slave
  frame[1] = 0x06;                                            // Ajout de l'Opcode d'Ecriture Registre
  frame[2] = reg >> 8;                                        // Ajout du numéro de Registre (MSB)
  frame[3] = reg & 0xFF;                                      // Ajout du numéro de Registre (LSB)
  frame[4] = value >> 8;                                      // Ajout de la valeur à écrire (MSB)
  frame[5] = value & 0xFF;                                    // Ajout de la valeur à écrire (LSB)
  
  computed_crc = modbus_compute_crc(frame, 6);                // Calcul du CRC sur le jeu de données
  frame[6] = computed_crc & 0xFF;                             // Ajout du CRC (MSB)
  frame[7] = computed_crc >> 8;                               // Ajout du CRC (LSB)
  
  hal_uart_write((uint8_t *)frame,8);                         // Envois sur le BUS RS485
}
