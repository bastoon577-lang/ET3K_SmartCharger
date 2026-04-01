#include "et3k_utils.h"

/*
 * L'ET3K étant pourvue d'un BUS de communication RS485, l'ESP01 communique au travers de celui-ci pour : 
 *  - Obtenir l'état de charge,
 *  - Permettre le pilotage du courant.
 */
/**
 * \fn uint16_t modbus_compute_crc(uint8_t *buf, uint8_t len)
 */
uint16_t modbus_compute_crc(uint8_t *buf, uint8_t len) {
  uint16_t crc = 0xFFFF;
  for(int pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)buf[pos];
    for(int i = 0; i < 8; i++) {
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
  uint8_t frame[8];
  
  frame[0] = slave;
  frame[1] = 0x03;
  frame[2] = reg >> 8;
  frame[3] = reg & 0xFF;
  frame[4] = 0x00;
  frame[5] = 0x01;
  
  uint16_t crc = modbus_compute_crc(frame,6);
  frame[6] = crc & 0xFF;
  frame[7] = crc >> 8;
  
  hal_uart_write((uint8_t *)frame,8);
  
  delay(50);
  
  uint8_t resp[7];
  int len = hal_uart_read_byte((uint8_t *)resp,7);
  
  if(len == 7 && resp[1] == 0x03) {
    return (resp[3] << 8) | resp[4];
  }
  
  return 0xFFFF;
}

/**
 * \fn void modbus_write_register(uint8_t slave, uint16_t reg, uint16_t value)
 */
void modbus_write_register(uint8_t slave, uint16_t reg, uint16_t value) {
  uint8_t frame[8];
  
  frame[0] = slave;
  frame[1] = 0x06;
  frame[2] = reg >> 8;
  frame[3] = reg & 0xFF;
  frame[4] = value >> 8;
  frame[5] = value & 0xFF;
  
  uint16_t crc = modbus_compute_crc(frame, 6);
  frame[6] = crc & 0xFF;
  frame[7] = crc >> 8;
  
  hal_uart_write((uint8_t *)frame,8);
}