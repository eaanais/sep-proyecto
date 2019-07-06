/*
 * IncFile1.h
 *
 * Created: 29-11-2018 11:33:55
 *  Author: Isidora
 */ 


#ifndef LCD_H_
#define LCD_H_

void start();
void command(char);
void data(char);
void Send_A_String(char *StringOfCharacters);
void cut(char *str);
void Send_An_Integer(int x);
void setCursor(int row,int column);
void clearScreen();
void home();
void cursor();
void noCursor();
void blink();
void noBlink();
void display();
void noDisplay();
void scrollDisplayLeft();
void scrollDisplayRight();
void autoscroll();
void noAutoscroll();
void createChar(int num,unsigned int *charArray);



#endif /* LCD_H_ */