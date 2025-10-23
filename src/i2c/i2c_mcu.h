/*
 * @Author: Ricken
 * @Email: hackricken@gmail.com
 * @Date: 2025-10-01 10:28:26
 * @LastEditTime: 2025-10-23 14:48:22
 * @FilePath: /t5_ai_coffee/src/i2c/i2c_mcu.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by Ricken, All Rights Reserved. 
 * 
**/

#ifndef __i2c_mcu_h__
#define __i2c_mcu_h__

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "hw_i2c_impl.h"

extern "C" {
#include "sw_i2c.h"
}

// typedef enum mcu_i2c_addr
// {
//     TEST_I2C_ADDR = 0xA6,
//     MCU_I2C_ADDR_WRITE = 0x88,
//     MCU_I2C_ADDR_READ = 0x89,
// } mcu_i2c_addr_t;

// 硬件I2C
int i2c_mcu_read(uint8_t addr, uint8_t reg_addr, uint8_t* str, uint32_t len);
int i2c_mcu_write(uint8_t addr, uint8_t reg_addr, uint8_t* data, uint32_t len);

// 软件I2C
int i2c_mcu_read2(uint8_t addr, uint8_t* str, uint32_t len);
int i2c_mcu_write2(uint8_t addr, uint8_t* data, uint32_t len);

#endif /* I2C_MCU */
