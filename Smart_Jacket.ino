//FDP = FOR DEBUGGING PURPOSES

//#define DEBUG                           //Uncomment for debugging output!!!!

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
    
#ifdef DEBUG
    Serial.print(".");                                              //FDP
#endif
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
#ifdef DEBUG
    Serial.println((String)r+"  "+g+"  "+b+"  ");
#endif    
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
    rStep=difR/maxSteps;
    gStep=difG/maxSteps;
    bStep=difB/maxSteps;
    for(int i = 0;i<=maxSteps;i++)
    {
      curR-=rStep;
      curG-=gStep;
      curB-=bStep;
      colorize((int)curR,(int)curG,(int)curB);
      delayMicroseconds(1000*(float)miliX/maxSteps);
    }
    //show(r,g,b);
  }
  //r,g,b target values
  void fadeStep(int r, int g, int b, float curStep)
  {
    float curR,curG,curB;
    curR=rVal;
    curG=gVal;
    curB=bVal;
    int difR, difG, difB;
    difR = r-rVal;
    difG = g-gVal;
    difB = b-bVal;
    int maxSteps = max(abs(difR), max(abs(difG), abs(difB)));
    float rStep, gStep, bStep;
    if(maxSteps == 0)
    {
      rStep=0;
      gStep=0;
      bStep=0;
    }
    else
    {
      rStep=((float)difR)/maxSteps;
      gStep=((float)difG)/maxSteps;
      bStep=((float)difB)/maxSteps;
    }
    
    curR+=rStep*curStep;
    curG+=gStep*curStep;
    curB+=bStep*curStep;
    colorize((int)curR,(int)curG,(int)curB);
  }
  int maxSteps(int r, int g, int b)
  {
    int difR, difG, difB;
    difR = r-rVal;
    difG = g-gVal;
    difB = b-bVal;
    return max(abs(difR), max(abs(difG), abs(difB)));
  }
};

//STRUCT END

void fadeSim(struct RGB leds[], bool ledsOn[], int nrleds, int r[], int g[], int b[],  int milis)
{
  int *maxsteps=0;
  maxsteps=new int [nrleds];
  int maximum = -1;
  for(int i = 0;i<nrleds;i++)
  {
    float mex = leds[i].maxSteps(r[i],g[i],b[i]);
    if(ledsOn[i]&&mex>maximum)
      maximum = mex;
    maxsteps[i]=mex;
  }
  //maxstepul fiecaruia/maximum este curstepul fiecaruia

  for(int i = 0;i<=maximum;i++)
  {
    for(int j = 0;j<nrleds;j++)
    {
      if(ledsOn[j])
        leds[j].fadeStep(r[j],g[j],b[j],maxsteps[j]/maximum*i);
    }
    delayMicroseconds(1000*(float)milis/maximum);
    Tlc.update();
    for(int i=0;i<nrleds;i++)
    {
      if(ledsOn[i])
        leds[i].show(r[i],g[i],b[i]);
    }
  }
}
void fadeSim(struct RGB leds[], bool ledsOn[], int nrleds, int r, int g, int b, int milis)
{
  int *maxsteps=0;
  maxsteps=new int [nrleds];
  int maximum = -1;
  for(int i = 0;i<nrleds;i++)
  {
    int mex = leds[i].maxSteps(r,g,b);
    if(ledsOn[i]&&(mex>maximum))
      maximum = mex;
    maxsteps[i]=mex;
  }


#ifdef DEBUG  
  for(int i =0;i<nrleds;i++)
  {
    if(ledsOn[i])
      Serial.println("Led "+(String)i+":"+leds[i].rVal+"  "+leds[i].gVal+"  "+leds[i].bVal);
  }
  Serial.println(""+(String)r+"  "+g+"  "+b);
#endif  

  for(int i = 0;i<=maximum;i++)
  {
    for(int j = 0;j<nrleds;j++)
    {
      if(ledsOn[j])
      {
#ifdef DEBUG        
        Serial.print((String)j+".) ");
#endif      
        leds[j].fadeStep(r,g,b,((float)maxsteps[j]/maximum)*i);
        
      }
    }
#ifdef DEBUG    
    Serial.println();
#endif
    
    delayMicroseconds(1000*(float)milis/maximum);
    Tlc.update();
  }
  for(int i=0;i<nrleds;i++)
  {
    if(ledsOn[i])
      leds[i].show(r,g,b);
  }
}
void blinkSim(struct RGB leds[], bool ledsOn[], int nrleds, int r, int g, int b, int milis, int times=1)
{
  int nrLedsOn=0;
  for(int i = 0;i<nrleds;i++)
    if(ledsOn[i])
      nrLedsOn++;
  for(int j=0;j<times*2;j++)
    for(int i=0;i<nrleds;i++)
    {
      if(ledsOn[i]&&j%2==0)
      {
        leds[i].show(r,g,b);
        delay(milis/(nrLedsOn*times)/4);
      }
      else if(ledsOn[i]&&j%2==1)
      {
        leds[i].show(0,0,0);
        delay(milis/(nrLedsOn*times)*3/4);
      }
    }
}
void blinkSim(struct RGB leds[], bool ledsOn[], int nrleds, int r[], int g[], int b[], int milis, int times=1)
{
  int nrLedsOn=0;
  for(int i = 0;i<nrleds;i++)
    if(ledsOn[i])
      nrLedsOn++;
  for(int j=0;j<times*2;j++)
    for(int i=0;i<nrleds;i++)
    {
      if(ledsOn[i]&&j%2==0)
      {
        leds[i].show(r[i],g[i],b[i]);
        delay(milis/(nrLedsOn*times)/4);
      }
      else if(ledsOn[i]&&j%2==1)
      {
        leds[i].show(0,0,0);
        delay(milis/(nrLedsOn*times)*3/4);
      }
    }
}
void showSim(struct RGB leds[], bool ledsOn[], int nrleds, int r[], int g[], int b[], int milis)
{
  for(int i =0;i<nrleds;i++)
    if(ledsOn[i])
      leds[i].show(r[i],g[i],b[i]);
  delay(milis);
}
void showSim(struct RGB leds[], bool ledsOn[], int nrleds, int r, int g, int b, int milis)
{
  for(int i =0;i<nrleds;i++)
    if(ledsOn[i])
      leds[i].show(r,g,b);
  delay(milis);
}

void animate(struct RGB led[], int times,int animSteps, const int anim[][10])
{
  int animat[10];
#ifdef DEBUG
  Serial.println("##############################################"+(String)(times%animSteps)); 
#endif       
  for(int i = 0;i<10;i++)
  {
    animat[i] = anim[times%animSteps][i];
#ifdef DEBUG            
    Serial.print(animat[i]);
    Serial.print(" ");
#endif            
  }
#ifdef DEBUG          
  Serial.println();
#endif          
  bool ledson[]= {animat[5],animat[6],animat[7],animat[8],animat[9]};
  if(animat[4]==0)
    showSim(led,ledson,5,animat[0],animat[1],animat[2],animat[3]);
  else
    fadeSim(led,ledson,5,animat[0],animat[1],animat[2],animat[3]);
}



//container variables
String str;
int Backlights=0, Frontlights=0,prevTouch=-1,curlcd=1;
int mode=0,curanim=-1, modeincr=0;
RGB led[5];


const int anim1[][10] = {
//  R   G   B  ms mde l1,l2,l3,l4,l5
  {255,255,255, 500,1, 1, 1, 1, 1, 1},
  
  {255, 0, 255,1000,1, 0, 0, 1, 0, 0},
  {255, 0, 255,1000,1, 0, 1, 1, 1, 0},
  {255, 0, 255,1000,1, 1, 1, 1, 1, 1},
  
  {128,128,128,1000,1, 1, 1, 1, 1, 1},
  {128,128,128, 500,0, 1, 1, 1, 1, 1},
  {128,128,128, 500,0, 1, 1, 1, 1, 1},
  
  { 0, 255,255,1500,1, 1, 0, 1, 0, 1},
  { 0, 255,255,1500,0, 1, 0, 1, 0, 1},
  
  { 0,   0,  0, 500,1, 1, 1, 1, 1, 1}
  };
int anim1Steps=10;

const int anim2[][10]={
  {  0,255,  0,200,1, 1, 1, 1, 1, 1},
  {  0,200,255,200,1, 1, 0, 0, 0, 1},
  {255,128,  0,200,1, 0, 1, 0, 1, 0},
  {  0,  0,255,200,1, 0, 0, 1, 0, 0},
  {255,255,255,200,1, 1, 1, 1, 1, 1},
  {  0,  0,255,200,1, 1, 1, 1, 1, 1},
};
const int anim2Steps=6;

//pin variables

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
  Serial.begin(115200);

  //Leds and TLC Inits
  led[0]=RGB(0,1,2);
  led[1]=RGB(3,4,5);
  led[2]=RGB(6,7,8);
  led[3]=RGB(9,10,11);
  led[4]=RGB(12,13,14);
  Tlc.init();
  Tlc.clear();
  Tlc.update();

  //Final preparations
  analogReference(EXTERNAL);
  delay(500);
}
unsigned long incr = 0;

void loop()
{
  int touch=digitalRead(touchPin);
  
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
#ifdef DEBUG  
  Serial.println("Touched: "+(String)touch);
#endif
  int direction = 1;
  if(Serial1.available() > 0)
    {
        str = Serial1.readStringUntil('~');
        //Serial.println(str);
        if(str=="An1")
        {
          mode = 1;
          curanim=1;
          modeincr=incr;
        }
        else if(str=="An2")
        {
          mode = 1;
          curanim=2;
          modeincr=incr;
        }
        else if(str=="B")
        {
          for(int i=0;i<5;i++)
            led[i].show(0,0,0);
          mode = 0;
        }
        else if(str=="L")
        {
          for(int i=0;i<5;i++)
            led[i].show(255,0,255);
          mode = 0;
        }
        else if(str=="R")
        {
          for(int i=0;i<5;i++)
            led[i].show(0,255,255);
          mode = 0;
        }
        else if(str == "U")
        {
          Backlights = 1;
          led[3].show(255,0,0);
          led[4].show(255,0,0);
          mode = 0;
        }
        else if(str == "u")
        {
          Backlights = 0;
          led[3].Blink(128,128,0,400,2);
          led[4].Blink(128,128,0,400,2);
          mode = 0;
        }
        else if(str == "W")
        {
          Frontlights = 1;
          led[3].show(255,255,255);
          led[4].show(255,255,255);
          mode = 0;
        }
        else if(str == "w")
        {
          Frontlights = 0;
          //frontlight.fade(0,128,128,500);
          bool ledsoned[]={0,0,0,1,1};
          fadeSim(led,ledsoned,5,0,0,0,1000);
          mode = 0;
        }
        else
        {
          for(int i=0;i<5;i++)
            led[i].show(0,0,0);
          //Serial1.println("Error");                                               //FDP
          mode = 0;
        }
        
    }

    if(mode)
    {
      unsigned int times = incr-modeincr;
      switch(curanim)
      {
        case 1:
          animate(led, times, anim1Steps, anim1);
          break;
        case 2:
          animate(led, times, anim2Steps, anim2);
          break;
      }
    }

    delay(50);
    
    int val = analogRead(temp1Pin);   
    float temperature = (5.0 * val * 100.0)/1024.0;
#ifdef DEBUG    
    Serial.println("Temp1: "+(String)temperature+"°");                                                  //FDP
#endif    
    delay(50);
    
    val = analogRead(temp2Pin);   
    float temperature2 = (5.0 * val * 100.0)/1024.0;
#ifdef DEBUG    
    Serial.println("Temp2: "+(String)temperature2+"°");                                                  //FDP
#endif   
    
    delay(50);
    int light = analogRead(lightPin);
#ifdef DEBUG    
    Serial.println("Light: "+(String)light);                                                  //FDP
#endif   
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(temperature);
    lcd.setCursor(7, 1);
    lcd.print(temperature2);
    lcd.setCursor(0,0);    
    lcd.print(light);
    
    if(incr%5==0)
    {
      String sends = (String)temperature+" "+temperature2+" "+light+" "+curlcd;
      Serial1.println(sends);
    }
    
    Tlc.update();
    delay(50);
    incr++;
}

