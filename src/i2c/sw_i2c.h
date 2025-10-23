/*
 * @Author: Ricken
 * @Email: hackricken@gmail.com
 * @Date: 2025-10-01 10:28:26
 * @LastEditTime: 2025-10-23 14:48:40
 * @FilePath: /t5_ai_demo/src/i2c/sw_i2c.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by Ricken, All Rights Reserved. 
 * 
**/

#ifndef __SW_I2C_H__
#define __SW_I2C_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "sw_i2c_gpio.h"

#define NO_ERROR 0
#define STATUS_OK 0
#define STATUS_FAIL -1

#define I2C_CLOCK_PERIOD_USEC 10

int16_t sw_i2c_select_bus(uint8_t bus_idx);

void sw_i2c_init(void);

void sw_i2c_release(void);

int8_t sw_i2c_read(uint8_t address, uint8_t *data, uint16_t count);

int8_t sw_i2c_write(uint8_t address, const uint8_t *data, uint16_t count);

void sw_i2c_sleep_usec(uint32_t useconds);

#endif
