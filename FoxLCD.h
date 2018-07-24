// Header: FoxLCD
// File Name: FoxLCD.h
// Author: Victor Nikitchuk
// Date: 23.07.2018

#ifndef __FOXLCD_H__
#define __FOXLCD_H__

#define CHAR_10 			0x7B
#define CHAR_12 			0x7C
#define CHAR_15 			0x7D
#define CHAR_ENTER		0x7E
#define CHAR_UP				0xD9
#define CHAR_DOWN			0xDA
#define CHAR_BELL			0xED
#define CHAR_1DEVIDE4	0xF0
#define CHAR_1DEVIDE3	0xF1
#define CHAR_1DEVIDE2	0xF2

class FoxLCD { //Инициализация класса FoxLCD
	public:
		FoxLCD(GPIO_TypeDef *GPIOx, uint32_t GPIOxEN, uint8_t RS, uint8_t E, uint8_t DB4, uint8_t DB5, uint8_t DB6, uint8_t DB7);
		void init(void); 											//Функция инициализации дисплея
		void sendcmd(uint8_t cmd); 						//Функция отправки команды в дисплей (RS подключен к нулю)
		void senddata(uint8_t data); 					//Функция отправки данных в дисплей (RS подключен к единице)
		void printchar(char c);								//Функция печати символа на дисплей
		void print(char msg[]);								//Функция печати текста на дисплее
		void clear(void);											//Функция очистки дисплея
		void home(void);											//Функция возврата каретки в начальное состояние
		void display(uint8_t state);					//Функция включения/отключения дисплея
		void cursor(uint8_t state);						//Функция включения/отключения отображения курсора
		void blink(uint8_t state);						//Функция включения/отключения моргания курсора
    void setcursor(uint8_t x, uint8_t y);	//Функция перемещения курсора по осям X и Y
		
		void delay_ms(uint16_t millis); 			//Функция задержки в миллисекундах
};
#endif /* __FOXLCD_H__ */
