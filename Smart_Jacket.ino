//FDP = FOR DEBUGGING PURPOSES

/*
    Basic Pin setup:
    ------------                                  ---u----
    ARDUINO   13|-> SCLK (pin 25)           OUT1 |1     28| OUT channel 0
              12|                           OUT2 |2     27|-> GND (VPRG)
              11|-> SIN (pin 26)            OUT3 |3     26|-> SIN (pin 11)
              10|-> BLANK (pin 23)          OUT4 |4     25|-> SCLK (pin 13)
               9|-> XLAT (pin 24)             .  |5     24|-> XLAT (pin 9)
               8|                             .  |6     23|-> BLANK (pin 10)
               7|                             .  |7     22|-> GND
               6|                             .  |8     21|-> VCC (+5V)
               5|                             .  |9     20|-> 2K Resistor -> GND
               4|                             .  |10    19|-> +5V (DCPRG)
               3|-> GSCLK (pin 18)            .  |11    18|-> GSCLK (pin 3)
               2|                             .  |12    17|-> SOUT
               1|                             .  |13    16|-> XERR
               0|                           OUT14|14    15| OUT channel 15
    ------------                                  --------

    -  Put the longer leg (anode) of the LEDs in the +5V and the shorter leg
         (cathode) in OUT(0-15).
    -  +5V from Arduino -> TLC pin 21 and 19     (VCC and DCPRG)
    -  GND from Arduino -> TLC pin 22 and 27     (GND and VPRG)
    -  digital 3        -> TLC pin 18            (GSCLK)
    -  digital 9        -> TLC pin 24            (XLAT)
    -  digital 10       -> TLC pin 23            (BLANK)
    -  digital 11       -> TLC pin 26            (SIN)
    -  digital 13       -> TLC pin 25            (SCLK)
    -  The 2K resistor between TLC pin 20 and GND will let ~20mA through each
       LED.  To be precise, it's I = 39.06 / R (in ohms).  This doesn't depend
       on the LED driving voltage.
    - (Optional): put a pull-up resistor (~10k) between +5V and BLANK so that
                  all the LEDs will turn off when the Arduino is reset.

    If you are daisy-chaining more than one TLC, connect the SOUT of the first
    TLC to the SIN of the next.  All the other pins should just be connected
    together:
        BLANK on Arduino -> BLANK of TLC1 -> BLANK of TLC2 -> ...
        XLAT on Arduino  -> XLAT of TLC1  -> XLAT of TLC2  -> ...
    The one exception is that each TLC needs it's own resistor between pin 20
    and GND.
    */

#include "Tlc5940.h"
#include <LiquidCrystal.h>
#define ON 1
#define OFF 0

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(22, 7, 5, 4, 6, 2);


struct RGB{
  int r_pin, g_pin, b_pin;
  int rVal, gVal, bVal;
  //int prevRVal, prevGVal, prevBVal;
  bool common_anode;
  RGB(int red=-1, int green=-1, int blue=-1, bool CommAnode=false) :
  r_pin(red),
  g_pin(green),
  b_pin(blue),
  common_anode(CommAnode)
  {
    init();
  }
  void init()
  {
    //show(0,0,0);
  }
  void show(int r, int g, int b)
  {
    //prevRVal = rVal;
    //prevGVal = gVal;
    //prevBVal = bVal;
    rVal = r;
    gVal = g;
    bVal = b;
    r*=16;
    g*=16;
    b*=16;
    if(common_anode)
    {
      r=4095-r;
      g=4095-g;
      b=4095-b;
    }
    Tlc.set(r_pin, r);
    Tlc.set(g_pin, g);
    Tlc.set(b_pin, b);
    Tlc.update();
    //Serial.print(".");                                              //FDP
  }
  void Blink(int r, int g, int b, int dur, int times)
  {
    for(int i = 0; i<times;i++)
    {
      show(0,0,0);
      delay((float)dur/times*3/4);
      show(r,g,b);
      delay((float)dur/times/4);
    }
    show(0,0,0);
  }
  void colorize(int r, int g, int b)
  {
    r*=16;
    g*=16;
    b*=16;
    if(common_anode)
    {
      r=4095-r;
      g=4095-g;
      b=4095-b;
    }
    Tlc.set(r_pin, r);
    Tlc.set(g_pin, g);
    Tlc.set(b_pin, b);
    Tlc.update();
  }
  void fade(int r, int g, int b, int miliX)
  {
    float curR,curG,curB;
    curR=rVal;
    curG=gVal;
    curB=bVal;
    int difR, difG, difB;
    difR = rVal-r;
    difG = gVal-g;
    difB = bVal-b;
    float maxSteps = max(abs(difR), max(abs(difG), abs(difB)));
    float rStep, gStep, bStep;
    rStep=maxSteps/difR;
    gStep=maxSteps/difG;
    bStep=maxSteps/difB;
    for(int i = 0;i<maxSteps;i++)
    {
      curR-=rStep;
      curG-=gStep;
      curB-=bStep;
      colorize((int)curR,(int)curG,(int)curB);
      delayMicroseconds(1000*(float)miliX/maxSteps);
    }
    //show(r,g,b);
  }
};

//container variables
String str;
int Backlights=0, Frontlights=0,prevTouch=-1,curlcd=1;
RGB led[3];

//pin variables
RGB frontlight(9,10,11);
RGB backlight(12,13,14);
const int temp1Pin = A0, temp2Pin = A1, lightPin = A2, touchPin=23,lcdPin=24;
void setup()
{

  //LCD Inits
  pinMode(lcdPin,OUTPUT);
  digitalWrite(lcdPin,HIGH);
  lcd.begin(16, 2);
  lcd.print("Initialized!");
  
  //Comunication Inits
  Serial1.begin(9600);
  Serial.begin(9600);

  //Leds and TLC Inits
  led[0]=RGB(0,1,2);
  led[1]=RGB(3,4,5);
  led[2]=RGB(6,7,8);
  Tlc.init();
  Tlc.clear();
  Tlc.update();

  //Final preparations
  analogReference(EXTERNAL);
  delay(500);
}
unsigned short incr = 0;
void loop()
{
  int touch=digitalRead(touchPin);
                                          //Serial.println((String)prevTouch+" - "+(String)touch);      //FDP
  if(prevTouch==-1)
  {
    if(touch==ON)
    {
      if(curlcd)
      {
        lcd.noDisplay();
        digitalWrite(lcdPin,LOW);
        curlcd=!curlcd;
      }
      else
      {
        lcd.display();
        digitalWrite(lcdPin,HIGH);
        curlcd=!curlcd;
      }
    }
    prevTouch=touch;
  } 
  else
  {
    if(prevTouch==OFF&&touch==ON)
    {
      if(curlcd)
      {
        lcd.noDisplay();
        digitalWrite(lcdPin,LOW);
        curlcd=!curlcd;
      }
      else
      {
        lcd.display();
        digitalWrite(lcdPin,HIGH);
        curlcd=!curlcd;
      }
    }
    prevTouch=touch;
  }
  int direction = 1;
  if(Serial1.available() > 0)
    {
        str = Serial1.readStringUntil('S');
        //Serial.println(str);
        if(str=="F")
          for(int i=0;i<3;i++)
            led[i].show(255,255,0);
        else if(str=="B")
          for(int i=0;i<3;i++)
            led[i].show(0,0,0);
        else if(str=="L")
          for(int i=0;i<3;i++)
            led[i].show(255,0,255);
        else if(str=="R")
          for(int i=0;i<3;i++)
            led[i].show(0,255,255);
        else if(str == "U")
        {
          Backlights = 1;
          backlight.show(255,0,0);
        }
        else if(str == "u")
        {
          Backlights = 0;
          backlight.Blink(128,128,0,400,2);
        }
        else if(str == "W")
        {
          Frontlights = 1;
          frontlight.show(255,255,255);
        }
        else if(str == "w")
        {
          Frontlights = 0;
          //frontlight.fade(0,128,128,500);
          frontlight.fade(0,0,0,1000);
        }
        else
        {
          for(int i=0;i<3;i++)
            led[i].show(0,0,0);
          //Serial1.println("Error");                                               //FDP
        }
        
    }
    lcd.clear();
    lcd.setCursor(0, 1);

    delay(50);
    
    int val = analogRead(temp1Pin);   
    float temperature = (5.0 * val * 100.0)/1024.0;
    //Serial.println(temperature);                                                  //FDP
   
    lcd.print(temperature);
    lcd.setCursor(7, 1);

    delay(50);
    
    val = analogRead(temp2Pin);   
    float temperature2 = (5.0 * val * 100.0)/1024.0;
    
    lcd.print(temperature2);
    lcd.setCursor(0,0);
    
    delay(50);
    int light = analogRead(lightPin);
    
    lcd.print(light);
    
    if(incr%5==0)
    {
      String sends = (String)temperature+" "+temperature2+" "+light+" "+curlcd;
      Serial1.println(sends);
      incr=0;
    }
    Tlc.update();
    delay(50);
    incr++;
}

