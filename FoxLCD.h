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

class FoxLCD { //������������� ������ FoxLCD
	public:
		FoxLCD(GPIO_TypeDef *GPIOx, uint32_t GPIOxEN, uint8_t RS, uint8_t E, uint8_t DB4, uint8_t DB5, uint8_t DB6, uint8_t DB7);
		void init(void); 											//������� ������������� �������
		void sendcmd(uint8_t cmd); 						//������� �������� ������� � ������� (RS ��������� � ����)
		void senddata(uint8_t data); 					//������� �������� ������ � ������� (RS ��������� � �������)
		void printchar(char c);								//������� ������ ������� �� �������
		void print(char msg[]);								//������� ������ ������ �� �������
		void clear(void);											//������� ������� �������
		void home(void);											//������� �������� ������� � ��������� ���������
		void display(uint8_t state);					//������� ���������/���������� �������
		void cursor(uint8_t state);						//������� ���������/���������� ����������� �������
		void blink(uint8_t state);						//������� ���������/���������� �������� �������
    void setcursor(uint8_t x, uint8_t y);	//������� ����������� ������� �� ���� X � Y
		
		void delay_ms(uint16_t millis); 			//������� �������� � �������������
};
#endif /* __FOXLCD_H__ */
