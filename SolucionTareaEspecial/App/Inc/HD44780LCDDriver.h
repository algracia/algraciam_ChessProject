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

#define LCD_BUSYFLAGBIT					0x80

#define LCD_I2C_WRITE_INSTRUCTIONS		0b1100
#define LCD_I2C_WRITE_CHARACTERS		0b1101
#define LCD_I2C_READBF					0b1110
#define LCD_I2C_ENABLE_BIT				0b0100

#define LCD_I2C_4BITOPERATION			0b00101100
#define LCD_I2C_INSTRUCTION_INIT_LCD	0b00111100

#define LCD_INSTRUCTION_2LINE_5X8FONT	0b00101000

#define LCD_INSTRUCTION_DISPLAY_ON		0b00001100
#define LCD_INSTRUCTION_DISPLAY_OFF		0b00001000

#define LCD_INSTRUCTION_CURSOR_ON		0b00001010
#define LCD_INSTRUCTION_CURSOR_OFF		0b00001100

#define LCD_INSTRUCTION_CBLINK_ON		0b00001001
#define LCD_INSTRUCTION_CBLINK_OFF		0b00001110

#define LCD_INSTRUCTION_ENTRYMODE_R		0b00000110

#define LCD_INSTRUCTION_DISPLAY_CLEAN	0b00000001
#define LCD_INSTRUCTION_HOME			0b00000010

#define LCD_INSTRUCTION_CURSOR_RIGHT	0b00010100
#define LCD_INSTRUCTION_CURSOR_LEFT		0b00010000



void LCD_Config(I2C_Handler_t *ptrHandlerI2C);
void WriteLCDInstruction(I2C_Handler_t *ptrHandlerI2C,uint8_t instruccion);
void WriteLCDCharacter(I2C_Handler_t *ptrHandlerI2C,uint8_t character);
void WriteI2CModule(I2C_Handler_t *ptrHandlerI2C,uint8_t i2cData);
void WriteLCDMsg(I2C_Handler_t *ptrHandlerI2C,char *i2cMsg);
uint8_t ReadLCDbusyFlag(I2C_Handler_t *ptrHandlerI2C);

void MoveLCDCursor(I2C_Handler_t *ptrHandlerI2C, uint8_t cursorLine, uint8_t cursorPos);

#endif /* INC_HD44780LCDDRIVER_H_ */
