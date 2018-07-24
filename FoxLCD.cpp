#include "stm32f0xx.h"                  // Device header

#define WRITE_HIGH(reg, bit) reg |= (1<<bit)
#define WRITE_LOW(reg, bit) reg &= ~(1<<bit)

uint16_t millis;

class FoxLCD { //Инициализация класса FoxLCD
	//Глобальные приватные переменные внутри класса 
	GPIO_TypeDef *GPIOx;
	uint32_t GPIOxEN;
	uint8_t RS;
	uint8_t E;
	uint8_t DB4;
	uint8_t DB5;
	uint8_t DB6;
	uint8_t DB7;
	
	uint8_t displaysettings; //Настройки дисплея - дисплей включен, курсор выключен не мигает 
	private:
		void writedata(uint8_t data);
		void strob(void);
	public:
		//Конструктор класса
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
FoxLCD::FoxLCD(GPIO_TypeDef *GPIOx, uint32_t GPIOxEN, uint8_t RS, uint8_t E, uint8_t DB4, uint8_t DB5, uint8_t DB6, uint8_t DB7) { //Конструктор четырёхбитного интерфейса без чтения флага занятости
	//Присваивание входных значений глобальным переменным
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
	//Инициализация портов ввода/вывода	
	RCC -> AHBENR |= GPIOxEN; //Включение тактирования шины группы портов
	GPIOx -> MODER |= (1<<(RS*2))|(1<<(E*2))|(1<<(DB4*2))|(1<<(DB5*2))|(1<<(DB6*2))|(1<<(DB7*2)); //Настройка портов на выход
	GPIOx -> OTYPER &= ~((1<<RS)|(1<<E)|(1<<DB4)|(1<<DB5)|(1<<DB6)|(1<<DB7)); //Режим push-pull
	GPIOx -> OSPEEDR |= (3<<(RS*2))|(3<<(E*2))|(3<<(DB4*2))|(3<<(DB5*2))|(3<<(DB6*2))|(3<<(DB7*2)); //Максимальная скорость работы портов
	
	//Инициализация системного таймера на прерывание каждые 1 мс
	SysTick_Config(SystemCoreClock/1000); 
	
	//Задержка 40 мс для гарантированного старта дисплея
	delay_ms(40);
	//Отправка команд инициализации дисплея
	//Установка 4-х битного интерфейса
	sendcmd(0x30); //0b00100000
	sendcmd(0x30); //0b00100000
	//Включение дисплея в двухстрочном режиме
	sendcmd(0xC0); //0b11000000
	delay_ms(1); //Ожидание выполнения команды
	//Включение дисплея с выключенным немигающим курсором
	sendcmd(0x00); //0b00000000
	sendcmd(0xC0); //0b11000000
	delay_ms(1); //Ожидание выполнения команды
	//Очистка дисплея
	sendcmd(0x00); //0b00000000
	sendcmd(0x10); //0b00010000
	delay_ms(2); //Ожидание выполнения команды
	//Настройка ввода - инкрементальный режим, смещение дисплея выключено
	sendcmd(0x00); //0b00000000
	sendcmd(0x60); //0b01100000
	delay_ms(1); //Ожидание выполнения команды
	//Перемещение каретки в исходное состояние
	home();
}

//Функция отправки команды в дисплей (RS подключен к нулю)
void FoxLCD::sendcmd(uint8_t cmd) {
	WRITE_LOW(GPIOx -> ODR, RS);//Подключение RS к нулю
	writedata(cmd); //Отправка команды в шину
}

//Функция отправки данных в дисплей (RS подключен к единице)
void FoxLCD::senddata(uint8_t data) {
	WRITE_HIGH(GPIOx -> ODR, RS);//Подключение RS к единице
	writedata(data); //Отправка данных в шину
}

//Функция записи данных в шину
void FoxLCD::writedata(uint8_t data) {
	//Отправка данных по 4х битной шине происходит от старшего полубайта к младшему в порядке от младшего бита к старшему
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

//Функция печати символа на дисплей
void FoxLCD::printchar(char c) {
	senddata(c);
}
//Функция печати текста на дисплее
void FoxLCD::print(char msg[]) {
	char i = 0;
	while(*msg) {
		i = *msg++;
		printchar(i);
	}
}
//Функция очистки дисплея
void FoxLCD::clear(void) {
	sendcmd(0x01);
	delay_ms(2);
}
//Функция возврата каретки в начальное состояние
void FoxLCD::home(void) {
	sendcmd(0x02);
	delay_ms(2);
}

//Функция включения/отключения дисплея
void FoxLCD::display(uint8_t state) {
  state != 0 ? WRITE_HIGH(displaysettings, 2) : WRITE_LOW(displaysettings, 2);
	sendcmd(displaysettings);
	delay_ms(1);
}
//Функция включения/отключения отображения курсора
void FoxLCD::cursor(uint8_t state) {
  state != 0 ? WRITE_HIGH(displaysettings, 1) : WRITE_LOW(displaysettings, 1);
	sendcmd(displaysettings);
	delay_ms(1);
}
//Функция включения/отключения моргания курсора
void FoxLCD::blink(uint8_t state) {
  state != 0 ? WRITE_HIGH(displaysettings, 0) : WRITE_LOW(displaysettings, 0);
	sendcmd(displaysettings);
	delay_ms(1);
}

//Функция перемещения курсора по осям X и Y
void FoxLCD::setcursor(uint8_t x, uint8_t y) {
	//Костыль для поддержки четырёхстрочных дисплеев
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

//Функция стробирования пина E
void FoxLCD::strob(void) {
	WRITE_HIGH(GPIOx -> ODR, E); //Подъём уровня на пине E
	delay_ms(1); //Задержка в 1000 нс
	WRITE_LOW(GPIOx -> ODR, E); //Опускание уровня на пине E
}

//Функция прерывания по срабатыванию системного таймера
extern "C" void SysTick_Handler(){
	millis++;
}

//Функция задержки в миллисекундах
void FoxLCD::delay_ms(uint16_t m) {
	millis = 0;
	while(millis != m);
}
