/*
 * HD44780LCDDriver.h
 *
 *  Created on: May 28, 2023
 *      Author: algraciam
 */

#include <stm32f4xx.h>
#include "I2CDriver.h"

#ifndef INC_HD44780LCDDRIVER_H_
#define INC_HD44780LCDDRIVER_H_

#define LCD_ADDRESS_A1JUMPER			0x25

#define LCD_DATA_1BYTE					1
#define LCD_DATA_2BYTE					2

#define LCD_BUSYFLAGBIT					0x80

#define I2C_WRITE_INSTRUCTIONS			0b0100
#define I2C_WRITE_CHARACTERS			0b0101
#define I2C_READBF						0b0110
#define I2C_INSTRUCTION_4BITOPERATION	0b00100100
#define I2C_INSTRUCTION_INIT_LCD		0b00110100

#define LCD_INSTRUCTION_1LINE_5X8FONT	0b00100000

#define LCD_INSTRUCTION_DISPLAY_ON		0b00001100
#define LCD_INSTRUCTION_DISPLAY_OFF		0b00001000

#define LCD_INSTRUCTION_CURSOR_ON		0b00001010
#define LCD_INSTRUCTION_CURSOR_OFF		0b00001100

#define LCD_INSTRUCTION_CBLINK_ON		0b00001001
#define LCD_INSTRUCTION_CBLINK_OFF		0b00001110

#define LCD_INSTRUCTION_ENTRYMODE_R		0b00000110

#define LCD_INSTRUCTION_DISPLAY_CLEAN	0b00000001

void LCD_Config(I2C_Handler_t *ptrHandlerI2C);
void WriteLCDInstruction(I2C_Handler_t *ptrHandlerI2C,uint8_t instruccion);
void WriteI2CModule(I2C_Handler_t *ptrHandlerI2C,uint8_t *i2cData, uint8_t numByte);
uint8_t ReadLCDbusyFlag(I2C_Handler_t *ptrHandlerI2C);

#endif /* INC_HD44780LCDDRIVER_H_ */
