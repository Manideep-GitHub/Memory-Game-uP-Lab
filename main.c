#include <at89c5131.h>
#include "lcd.h"																				//Driver for interfacing lcd
//#include "serial.c"																			//UART interfacing functions

code unsigned char words[32][5] = {"\0", "guru\0", "taxi\0", "iris\0", "jazz\0", "kiwi\0", "lily\0", "lola\0", 
							"mars\0", "hand\0", "nato\0", "neon\0", "orca\0", "paul\0", "quiz\0", "raja\0", 	//31 words' array with corresponding
							"risk\0", "soda\0", "zero\0", "tuna\0", "unit\0", "visa\0", "wave\0", "wolf\0", 	//indices ranging from 1 to 31
							"xray\0", "yoyo\0", "acid\0", "bali\0", "cave\0", "olga\0", "drum\0", "fuel\0"};	
code unsigned char getready[] = "Get Ready";
code unsigned char display_score[] =  " Score:      ";
code unsigned char display_hscore[] = " High Score: ";					//Storing strings in ROM for later use
char game_str[10][5];
unsigned char game_num[10];
bit receive = 0;
unsigned char ch = 0;

void uart_init(void)
{
	TMOD=0x20;			//Configure Timer 1 in Mode 2
	TH1=243;				//Load TH1 to obtain require Baudrate (Refer Serial.pdf for calculations)
	SCON=0x50;			//Configure UART peripheral for 8-bit data transfer 
	TR1=1;					//Start Timer 1
	ES=1;						//Enable Serial Interrupt
	EA=1;						//Enable Global Interrupt
}							
							
//main program starts here
void main(void)
{
	unsigned char lfsr_init = 13;								//non-zero initial state to be loaded into the LFSR
	unsigned char lfsr = lfsr_init;							//Bit accessible Linear Shift Register
	unsigned char hscore = 0;
	lcd_init();																	//LCD initialisation
	uart_init();																//UART module intialisation
	
	while(1)
	{	
		unsigned char score = 0;

		unsigned char i = 0;
		while(i<10)												//Displaying 10 pseudo-random word-digit pairs usinng LFSR
		{
			unsigned char num;
			unsigned char temp=0;
			temp = (lfsr>>3)^(lfsr);
			temp = temp & 1;
			lfsr = (lfsr>>1)|(temp<<4);							//LFSR current state = (b3 xor b0)b4b3b2b1  (prev state = b4b3b2b1b0)
			game_str[i][0] = words[lfsr][0];
			game_str[i][1] = words[lfsr][1];
			game_str[i][2] = words[lfsr][2];				//storing words in the current game in an array
			game_str[i][3] = words[lfsr][3];
			game_str[i][4] = words[lfsr][4];
			num = lfsr & 15;												//num = lowernibble of LFSR
			num = num % 10;													//num = remainder of num/10
			num = num + 48;													//adding 48 to obtain the ascii value of the number
			game_num[i] = num;											//storing the digits corresponding to the words in the current game
			lcd_init();
			lcd_cmd(0x85);													//Move cursor to first row, 5th column
			lcd_write_string(words[lfsr]);					//display on LCD the word corresponding to the value in LFSR
			lcd_write_string(" ");
			lcd_write_char(num);										//displaying the corresponding number
			msdelay(1000);						//1sec delay
			i++;
		}
		msdelay(250);
		lcd_cmd(0x84);									//Move cursor to first row, 5th column
		lcd_write_string(getready);					//Get ready to play
		msdelay(2500);
		lcd_cmd(0x01);			//LCD clear
		msdelay(250);
		i = 0;
		while(i<10)
		{
			lcd_cmd(0x86);								//Move cursor to first row, 7th column
			lcd_write_string(game_str[i]);			//Display words one by one from the recorded sequence
			ch = 0;											//copies SBUF
			receive = 1;								//receive character
			msdelay(3000);							//3sec delay for typing the guess
			if(ch == game_num[i])
			{
				score = score + 1;					//Score incremented if guessed digit is correct
			}
			lcd_cmd(0x01);	// LCD clear
			msdelay(4);
			i++;
		}
		if(hscore<score)
		{
			hscore = score;								//Updating highscore after each game
		}
		lcd_cmd(0x80);									//Move cursor to first row, 1st column
		lcd_write_string(display_score);	//Display currrent score in first row
		if(score==10){
			lcd_write_string("10");
		}
		else{
			lcd_write_char(score+48);
		}
		lcd_cmd(0xC0);									//Move cursor to first row, 1st column
		lcd_write_string(display_hscore);	//Display highest score so far
		if(hscore==10){
			lcd_write_string("10");
		}
		else{
			lcd_write_char(hscore+48);
		}
		msdelay(3000);
	}
}
void serial_ISR(void) interrupt 4
{
	if(RI==1)			//check whether RI is set
	{
		RI = 0;			//Clear RI flag
		if(receive){
			ch = SBUF;
			lcd_cmd(0xC8);
			lcd_write_char(ch);	//Input character digit displayed on LCD
			receive = 0;
		}
	}
}