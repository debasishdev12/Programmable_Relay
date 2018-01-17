#include<LiquidCrystal.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>

LiquidCrystal lcd(12,11,5,4,3,2);

#define CURRENT_SENSOR A0

const char characters[][8] PROGMEM = {
  {0b00000,0b00100,0b01110,0b11111,0b00100,0b00100,0b00100,0b00000},    //up  characters[0]
  {0b00000,0b00100,0b00100,0b00100,0b11111,0b01110,0b00100,0b00000},    //down characters[1]
  {0b00000,0b00000,0b00100,0b01100,0b11111,0b01100,0b00100,0b00000},    //left characters[2]
  {0b00000,0b00000,0b00100,0b00110,0b11111,0b00110,0b00100,0b00000},    //right characters[3]
  {0b00000,0b00000,0b10001,0b01010,0b00100,0b01010,0b10001,0b00000},    //cross characters[4]
  {0b00000,0b00000,0b00001,0b00010,0b10100,0b01000,0b00000,0b00000}     //tick characters[5]
};
/*
input pins

set_button = 10;          
left_button = 9;
right_button = 8;
exit_button = 7;

output pin with which the relay will be connected
output_pin = 6;

*/
bool set_button_pressed = false;        //1 byte
bool set_button_pressed_again = false;  //1 byte

uint8_t coloumn = 0;                    //1 byte

float max_current = 00.00f;             //4 byte

uint8_t after_floating_1 = 0;           //1 byte
uint8_t after_floating_2 = 0;           //1 byte
uint8_t before_floating_1 = 0;          //1 byte
uint8_t before_floating_2 = 0;          //1 byte

byte bb[8];                             //8 byte

void setup() 
{
  //       set          left        right
  //-------------------------------------------------------------
  DDRB |= (0<<DDB2) | (0<<DDB1) | (0<<DDB0);                  //pinMode(set_button,INPUT); pinMode(left_button,INPUT);
  PORTB |= (1<<PB2) | (1<<PB1) | (1<<PB0);                    //digitalWrite(set_button,HIGH);digitalWrite(left_button,HIGH);
  
  //        exit        relay
  //-------------------------------------------------------------
  DDRD |= (0<<DDD7) | (1<<DDD6);                              //pinMode(right_button,INPUT);pinMode(exit_button,INPUT);pinMode(output_pin,OUTPUT);
  PORTD |= (1<<PD7) | (1<<PD6);                               //digitalWrite(right_button,HIGH);digitalWrite(exit_button,HIGH);digitalWrite(output_pin,LOW);

  for(byte i=0;i<6;i++)
  {
    for(byte j=0;j<8;j++)
    {
      bb[j] = pgm_read_byte( &characters[i][j]);
    }
    lcd.createChar(i,bb);
  }

  lcd.begin(20,4);
  
  max_current_separate_value();
}

void loop()
{
  uint8_t temp_digit;
  if(bit_is_clear(PINB,2))              //set_button
  {
    delay(500);
    set_button_pressed = true;
    lcd.clear();
    first_layer();
  }

  if(set_button_pressed == false)
  {
    homepage();
  }
  else if(set_button_pressed == true)
  { 
    while(1)
    {
      lcd.setCursor(coloumn,0);
      lcd.cursor();
      if(bit_is_clear(PINB,2))      //set_button
      {
        delay(500);
        set_button_pressed_again = true;
        second_layer();
        changed_data();

        if(coloumn == 0) temp_digit = before_floating_1;
        else if(coloumn == 1) temp_digit = before_floating_2;
        else if(coloumn == 3) temp_digit = after_floating_1;
        else if(coloumn == 4) temp_digit = after_floating_2;
      }
      //************************************
//      if(set_button_pressed_again == false)
//      {
//        first_layer();
//      }
      if(set_button_pressed_again == true)
      {
        while(1)
        {
          lcd.setCursor(coloumn,0);
          if((bit_is_clear(PINB,2)) && set_button_pressed_again == true)      //set_button
          {
            delay(500);
            if(coloumn == 0)  
            {
              before_floating_1 = temp_digit;
            }
            else if(coloumn == 1)
            {
              before_floating_2 = temp_digit;
            }
            else if(coloumn == 3)
            {
              after_floating_1 = temp_digit;
            }
            else if(coloumn == 4)
            {
              after_floating_2 = temp_digit;
            }
            temp_digit = 0;
            
            first_layer();
            changed_data();
            
            set_button_pressed_again = false;
            break;
          }
          if((bit_is_clear(PINB,1)) && set_button_pressed_again == true)      //left button
          {
            delay(500);
            
            temp_digit++;
            if(temp_digit == 10) temp_digit = 0;
            lcd.setCursor(coloumn,0);
            lcd.print(temp_digit);
          }
          if((bit_is_clear(PINB,0)) && set_button_pressed_again == true)      //right button
          {
            delay(500);

            temp_digit--;
            if(temp_digit == 255) temp_digit = 9;
            lcd.setCursor(coloumn,0);
            lcd.print(temp_digit); 
          }
          if((bit_is_clear(PIND,7)) && set_button_pressed_again == true)      //exit button
          {
            delay(500);
            lcd.clear();
            
            first_layer();
            set_button_pressed_again = false;
            break;
          }
        }
      }
      //************************************
      if((bit_is_clear(PINB,1)) && set_button_pressed_again == false)         //left button
      {
        delay(500);
        coloumn--;
        if(coloumn == 0) coloumn = 0;
        if(coloumn == 2) coloumn = 1;
        lcd.setCursor(coloumn,0);
        lcd.cursor();
      }
      if((bit_is_clear(PINB,0)) && set_button_pressed_again == false)         //right button
      {
        delay(500);
        coloumn++;
        if(coloumn >= 4) coloumn = 4;
        if(coloumn == 2) coloumn = 3;
        lcd.setCursor(coloumn,0);
        lcd.cursor();
      }
      if((bit_is_clear(PIND,7)))                                              //exit button
      {
        lcd.clear();
        lcd.noCursor();
        max_current = float((1000*before_floating_1)+(100*before_floating_2)+(10*after_floating_1)+(1*after_floating_2));
        max_current = max_current/100;
        if(max_current >= 30.00) 
        {
          lcd.clear();
          lcd.print(F("Max is 30.00 A"));
          delay(3000);
          max_current = 30.00;
          lcd.clear();
        }
        EEPROM.put(0,max_current);
        set_button_pressed = false;
        coloumn = 0;
        break;
      }
    }
  }
}

void first_layer(void)
{ 
  lcd.setCursor(0,0);
  lcd.print(EEPROM.get(0,max_current),2);         //EEPROM.get(0, max_current));
  lcd.setCursor(0,1);
  lcd.write((uint8_t)5);       //tick
  lcd.setCursor(5,1);
  lcd.write((uint8_t)2);       //left
  lcd.setCursor(10,1);
  lcd.write((uint8_t)3);       //right
  lcd.setCursor(15,1);
  lcd.write((uint8_t)4);       //cross
}

void second_layer(void)
{
  lcd.setCursor(0,0);
  lcd.print(EEPROM.get(0, max_current),2);              //(EEPROM.get(0, max_current));
  lcd.cursor();
  lcd.setCursor(0,1);
  lcd.write((uint8_t)5);          //tick
  lcd.setCursor(5,1);
  lcd.write((uint8_t)0);          //up
  lcd.setCursor(10,1);
  lcd.write((uint8_t)1);          //down
  lcd.setCursor(15,1);
  lcd.write((uint8_t)4);          //cross
}

void homepage(void)
{
  int sensor_max = getMaxValue();
  float current = (float)((sensor_max-508)*5)/(1023*0.066);
  
  //RMS value of current in Ampere unit
  current = current/sqrt(2);    
  //current = current - .05;
  //minimum current that can be measured is 18.7 mA.
  //(1*5.0)/(1023*0.066)*1000
  if(current <= 0.05) current = 0.00;

  //if instantaneous current cross the max_current then relay is off
  if(current >= max_current)
  {
    PORTD |= (0<<PD6);
  }
  lcd.clear();
  lcd.setCursor(0,0);           //set cursor point to zero position
  lcd.print(F("Load(A)"));         //load current in ampere
  lcd.setCursor(10,0);          //set cursor to another position to show the maximum
  lcd.print(F("Max(A)"));          //maximum current in which the relay will be off
  lcd.setCursor(1,1);           //set cursor below the Load(A) headline
  lcd.print(current,2);
  lcd.setCursor(7,1);
  lcd.print(F("set"));
  lcd.setCursor(11,1);
  lcd.print(EEPROM.get(0, max_current),2);
}

void changed_data(void)
{
  lcd.setCursor(0,0);
  lcd.print(before_floating_1);
  lcd.setCursor(1,0);
  lcd.print(before_floating_2);
  lcd.setCursor(2,0);
  lcd.print(F("."));
  lcd.setCursor(3,0);
  lcd.print(after_floating_1);
  lcd.setCursor(4,0);
  lcd.print(after_floating_2);
}

void max_current_separate_value(void)
{
  int data = int(EEPROM.get(0,max_current)*100);
  after_floating_2 = data%10;
  data = data/10;
  after_floating_1 = data%10;
  data = data/10;
  before_floating_2 = data%10;
  data = data/10;
  before_floating_1 = data%10;
}

int getMaxValue()
{
  int sensor_value;
  int sensor_max = 0;
  uint32_t start_time = millis();

  while((millis() - start_time) <= 200)
  {
    sensor_value = analogRead(CURRENT_SENSOR);

    //record the maximum sensor value
    if(sensor_value > sensor_max)
      sensor_max = sensor_value;
  }

  return sensor_max;
}
