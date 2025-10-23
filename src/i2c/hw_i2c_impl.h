/*
 * @Author: Ricken
 * @Email: hackricken@gmail.com
 * @Date: 2025-10-01 10:28:26
 * @LastEditTime: 2025-10-23 14:48:06
 * @FilePath: /t5_ai_coffee/src/i2c/hw_i2c_impl.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by Ricken, All Rights Reserved. 
 * 
**/

#ifndef HW_I2C_IMPL_H
#define HW_I2C_IMPL_H

#include <stdint.h>

int  hw_i2c_init(void);
void hw_i2c_release(void);

int8_t hw_i2c_write(uint8_t address, const uint8_t *data, uint16_t count);
int8_t hw_i2c_read(uint8_t address, uint8_t reg_addr, uint8_t *data, uint16_t count);

void hw_i2c_sleep_usec(uint32_t useconds);

#endif /* HW_I2C_IMPL_H */
