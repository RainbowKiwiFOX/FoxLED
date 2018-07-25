#include "stm32f0xx.h"                  // Device header

#define WRITE_HIGH(reg, bit) reg |= (1<<bit)
#define WRITE_LOW(reg, bit) reg &= ~(1<<bit)

uint16_t millis;

class FoxLCD { //������������� ������ FoxLCD
	//���������� ��������� ���������� ������ ������ 
	GPIO_TypeDef *GPIOx;
	uint32_t GPIOxEN;
	uint8_t RS;
	uint8_t E;
	uint8_t DB4;
	uint8_t DB5;
	uint8_t DB6;
	uint8_t DB7;
	
	uint8_t displaysettings; //��������� ������� - ������� �������, ������ �������� �� ������ 
	private:
		void writedata(uint8_t data);
		void strob(void);
	public:
		//����������� ������
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
		void chartocgram(uint8_t number, uint8_t chararray[8]); //������� ��������� ������������ ������� � CGRAM. ����� �������� ������� ��� �������� �� 0 �� 7
		void printfromcgram(uint8_t number); 	//������� ������ ������������ ������� �� CGRAM. ����� �������� ������� ��� �������� �� 0 �� 7
		
		void delay_ms(uint16_t millis); 			//������� �������� � �������������
};
FoxLCD::FoxLCD(GPIO_TypeDef *GPIOx, uint32_t GPIOxEN, uint8_t RS, uint8_t E, uint8_t DB4, uint8_t DB5, uint8_t DB6, uint8_t DB7) { //����������� ������������� ���������� ��� ������ ����� ���������
	//������������ ������� �������� ���������� ����������
	this -> GPIOx = GPIOx;
	this -> GPIOxEN = GPIOxEN;
	this -> RS = RS;
	this -> E = E;
	this -> DB4 = DB4;
	this -> DB5 = DB5;
	this -> DB6 = DB6;
	this -> DB7 = DB7;
}
void FoxLCD::init(void) {
	displaysettings = 0x0C;
	//������������� ������ �����/������	
	RCC -> AHBENR |= GPIOxEN; //��������� ������������ ���� ������ ������
	GPIOx -> MODER |= (1<<(RS*2))|(1<<(E*2))|(1<<(DB4*2))|(1<<(DB5*2))|(1<<(DB6*2))|(1<<(DB7*2)); //��������� ������ �� �����
	GPIOx -> OTYPER &= ~((1<<RS)|(1<<E)|(1<<DB4)|(1<<DB5)|(1<<DB6)|(1<<DB7)); //����� push-pull
	GPIOx -> OSPEEDR |= (3<<(RS*2))|(3<<(E*2))|(3<<(DB4*2))|(3<<(DB5*2))|(3<<(DB6*2))|(3<<(DB7*2)); //������������ �������� ������ ������
	
	//������������� ���������� ������� �� ���������� ������ 1 ��
	SysTick_Config(SystemCoreClock/1000); 
	
	//�������� 40 �� ��� ���������������� ������ �������
	delay_ms(40);
	//�������� ������ ������������� �������
	//��������� 4-� ������� ����������
	sendcmd(0x30); //0b00100000
	sendcmd(0x30); //0b00100000
	//��������� ������� � ������������ ������
	sendcmd(0xC0); //0b11000000
	delay_ms(1); //�������� ���������� �������
	//��������� ������� � ����������� ���������� ��������
	sendcmd(0x00); //0b00000000
	sendcmd(0xC0); //0b11000000
	delay_ms(1); //�������� ���������� �������
	//������� �������
	sendcmd(0x00); //0b00000000
	sendcmd(0x10); //0b00010000
	delay_ms(2); //�������� ���������� �������
	//��������� ����� - ��������������� �����, �������� ������� ���������
	sendcmd(0x00); //0b00000000
	sendcmd(0x60); //0b01100000
	delay_ms(1); //�������� ���������� �������
	home();
	delay_ms(2);
	sendcmd(0x0C);
	delay_ms(1);
	clear();
	delay_ms(2);
}

//������� �������� ������� � ������� (RS ��������� � ����)
void FoxLCD::sendcmd(uint8_t cmd) {
	WRITE_LOW(GPIOx -> ODR, RS);//����������� RS � ����
	writedata(cmd); //�������� ������� � ����
}

//������� �������� ������ � ������� (RS ��������� � �������)
void FoxLCD::senddata(uint8_t data) {
	WRITE_HIGH(GPIOx -> ODR, RS);//����������� RS � �������
	writedata(data); //�������� ������ � ����
}

//������� ������ ������ � ����
void FoxLCD::writedata(uint8_t data) {
	//�������� ������ �� 4� ������ ���� ���������� �� �������� ��������� � �������� � ������� �� �������� ���� � ��������
	(data & (1<<4)) != 0 ? WRITE_HIGH(GPIOx -> ODR, DB4) : WRITE_LOW(GPIOx -> ODR, DB4);
	(data & (1<<5)) != 0 ? WRITE_HIGH(GPIOx -> ODR, DB5) : WRITE_LOW(GPIOx -> ODR, DB5);
	(data & (1<<6)) != 0 ? WRITE_HIGH(GPIOx -> ODR, DB6) : WRITE_LOW(GPIOx -> ODR, DB6);
	(data & (1<<7)) != 0 ? WRITE_HIGH(GPIOx -> ODR, DB7) : WRITE_LOW(GPIOx -> ODR, DB7);
	strob();
	(data & (1<<0)) != 0 ? WRITE_HIGH(GPIOx -> ODR, DB4) : WRITE_LOW(GPIOx -> ODR, DB4);
	(data & (1<<1)) != 0 ? WRITE_HIGH(GPIOx -> ODR, DB5) : WRITE_LOW(GPIOx -> ODR, DB5);
	(data & (1<<2)) != 0 ? WRITE_HIGH(GPIOx -> ODR, DB6) : WRITE_LOW(GPIOx -> ODR, DB6);
	(data & (1<<3)) != 0 ? WRITE_HIGH(GPIOx -> ODR, DB7) : WRITE_LOW(GPIOx -> ODR, DB7);
	strob();
}

//������� ������ ������� �� �������
void FoxLCD::printchar(char c) {
	senddata(c);
}
//������� ������ ������ �� �������
void FoxLCD::print(char msg[]) {
	char i = 0;
	while(*msg) {
		i = *msg++;
		//��������� ������� �������� "�� ����" (������ ��� �������� � ���������� � ���������������)
		switch(i) {
			case '�':
				printchar('A');
			  break;
			case '�':
				printchar('a');
			  break;
			case '�':
				printchar(0xA0);
			  break;
			case '�':
				printchar(0xB2);
			  break;
			case '�':
				printchar('B');
			  break;
			case '�':
				printchar(0xB3);
			  break;
			case '�':
				printchar(0xA1);
			  break;
			case '�':
				printchar(0xB4);
			  break;
			case '�':
				printchar(0xE0);
			  break;
			case '�':
				printchar(0xE3);
			  break;
			case '�':
				printchar('E');
			  break;
			case '�':
				printchar('e');
			  break;
			case '�':
				printchar(0xA2);
			  break;
			case '�':
				printchar(0xB5);
			  break;
			case '�':
				printchar(0xA3);
			  break;
			case '�':
				printchar(0xB6);
			  break;
			case '�':
				printchar(0xA4);
			  break;
			case '�':
				printchar(0xB7);
			  break;
			case '�':
				printchar(0xA5);
			  break;
			case '�':
				printchar(0xB8);
			  break;
			case '�':
				printchar(0xA6);
			  break;
			case '�':
				printchar(0xB9);
			  break;
			case '�':
				printchar('K');
			  break;
			case '�':
				printchar(0xBA);
			  break;
			case '�':
				printchar(0xA7);
			  break;
			case '�':
				printchar(0xBB);
			  break;
			case '�':
				printchar('M');
			  break;
			case '�':
				printchar(0xBC);
			  break;
			case '�':
				printchar('H');
			  break;
			case '�':
				printchar(0xBD);
			  break;
			case '�':
				printchar('O');
			  break;
			case '�':
				printchar('o');
			  break;
			case '�':
				printchar(0xA8);
			  break;
			case '�':
				printchar(0xBE);
			  break;
			case '�':
				printchar('P');
			  break;
			case '�':
				printchar('p');
			  break;
			case '�':
				printchar('C');
			  break;
			case '�':
				printchar('c');
			  break;
			case '�':
				printchar('T');
			  break;
			case '�':
				printchar(0xBF);
			  break;
			case '�':
				printchar(0xA9);
			  break;
			case '�':
				printchar('y');
			  break;
			case '�':
				printchar(0xAA);
			  break;
			case '�':
				printchar(0xE4);
			  break;
			case '�':
				printchar('X');
			  break;
			case '�':
				printchar('x');
			  break;
			case '�':
				printchar(0xE1);
			  break;
			case '�':
				printchar(0xE5);
			  break;
			case '�':
				printchar(0xAB);
			  break;
			case '�':
				printchar(0xC0);
			  break;
			case '�':
				printchar(0xAC);
			  break;
			case '�':
				printchar(0xC1);
			  break;
			case '�':
				printchar(0xE2);
			  break;
			case '�':
				printchar(0xE6);
			  break;
			case '�':
				printchar(0xAD);
			  break;
			case '�':
				printchar(0xC2);
			  break;
			case '�':
				printchar(0xAE);
			  break;
			case '�':
				printchar(0xC3);
			  break;
			case '�':
				printchar('b');
			  break;
			case '�':
				printchar(0xC4);
			  break;
			case '�':
				printchar(0xAF);
			  break;
			case '�':
				printchar(0xC5);
			  break;
			case '�':
				printchar(0xB0);
			  break;
			case '�':
				printchar(0xC6);
			  break;
			case '�':
				printchar(0xB1);
			  break;
			case '�':
				printchar(0xC7);
			  break;
			default:
				printchar(i);
				break;
		}
		delay_ms(1);
	}
}
//������� ������� �������
void FoxLCD::clear(void) {
	sendcmd(0x01);
	delay_ms(2);
}
//������� �������� ������� � ��������� ���������
void FoxLCD::home(void) {
	sendcmd(0x02);
	delay_ms(2);
}

//������� ���������/���������� �������
void FoxLCD::display(uint8_t state) {
  state != 0 ? WRITE_HIGH(displaysettings, 2) : WRITE_LOW(displaysettings, 2);
	sendcmd(displaysettings);
	delay_ms(1);
}
//������� ���������/���������� ����������� �������
void FoxLCD::cursor(uint8_t state) {
  state != 0 ? WRITE_HIGH(displaysettings, 1) : WRITE_LOW(displaysettings, 1);
	sendcmd(displaysettings);
	delay_ms(1);
}
//������� ���������/���������� �������� �������
void FoxLCD::blink(uint8_t state) {
  state != 0 ? WRITE_HIGH(displaysettings, 0) : WRITE_LOW(displaysettings, 0);
	sendcmd(displaysettings);
	delay_ms(1);
}

//������� ����������� ������� �� ���� X � Y
void FoxLCD::setcursor(uint8_t x, uint8_t y) {
	//������� ��� ��������� �������������� ��������
	switch(y) {
		case 0:
			sendcmd(x | 0x80);
			break;
		case 1:
			sendcmd((0x40 + x) | 0x80);
			break;
		case 2:
			sendcmd((0x14 + x) | 0x80);
			break;
		case 3:
			sendcmd((0x54 + x) | 0x80);
			break;
	}
	delay_ms(1);
}

//������� ��������� ������������ ������� � CGRAM
void FoxLCD::chartocgram(uint8_t number, uint8_t chararray[8]) {
	sendcmd(0x40 + number*8);
	for (uint8_t i = 0; i < 8; i++) {
		senddata(chararray[i]);
	}
}
//������� ������ ������������ ������� �� CGRAM. ����� �������� ������� ��� �������� �� 0 �� 7
void FoxLCD::printfromcgram(uint8_t number) {
	printchar(number*8);
}
//������� ������������� ���� E
void FoxLCD::strob(void) {
	WRITE_HIGH(GPIOx -> ODR, E); //������ ������ �� ���� E
	delay_ms(1); //�������� � 1000 ��
	WRITE_LOW(GPIOx -> ODR, E); //��������� ������ �� ���� E
}

//������� ���������� �� ������������ ���������� �������
extern "C" void SysTick_Handler(){
	millis++;
}

//������� �������� � �������������
void FoxLCD::delay_ms(uint16_t m) {
	millis = 0;
	while(millis != m);
}
