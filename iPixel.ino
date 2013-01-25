#include <SPI.h>
#include <WS2801.h>
#include <TM1638.h>


//to draw a frame we need arround 20ms to send an image. the serial baudrate is
//NOT the bottleneck. 
#define BAUD_RATE 115200

#define PANELS 1

#define PPP 64


//initialize pixels
// using default SPI pins
WS2801 strip = WS2801(64);

byte prev;

// define a module on data pin 8, clock pin 9 and strobe pin 7
TM1638 module(8, 9, 7);

byte error;

void setup()
{
  
  //im your slave and wait for your commands, master!
  Serial.begin(BAUD_RATE); //Setup high speed Serial
  Serial.flush();


  strip.begin();        // Start up the LED Strip
  showWaitingPic();
  error = 0;
  
  module.setDisplayToString("Start");
  
}

void loop()
{
  char data[192];
  
  if (Serial.available() > 0)
  {
    byte type = Serial.read();
    
    if(type == 'D')
    {
      int count = Serial.readBytes( data, 192);
      if(count == 192)
      {
        for(int i = 0; i < PPP; i++)
        {
          strip.setPixelColor(i, Color(data[3*i],data[3*i+1],data[3*i+2]));
        }
        
        strip.show();
      }
    }
    else if (type == 'I')
    {
      byte response[6];
      response[0] = 'A';
      response[1] = 'K';
      response[2] = PANELS;
      response[3] = PPP;
      response[4] = Serial.available();
      response[5] = error;
      Serial.write(response, 6);
    }
    else if (type == 'S')
   {
     char action[18];
     int count = Serial.readBytes(action, 18);
     if(count == 18)
     {
       if(action[0] == 'S') //Want to set the display content?
       {
         module.clearDisplay();
         char text[8];
         for (int i = 1; i < 9; i++)
         {
           text[i-1] = action[i];
         }
         module.setDisplayToString(text);
       }
       if(action[9] == 'S') //Want to set the leds?
       {
         for (int i = 10; i < 18; i++)
         {
           switch (action[i])
           {
             case '0':
               module.setLED(TM1638_COLOR_NONE, i-10);
               break;
             case '1':
               module.setLED(TM1638_COLOR_GREEN, i-10);
               break;
             case '2':
               module.setLED(TM1638_COLOR_RED, i-10);
               break;
             case '3':
               module.setLED(TM1638_COLOR_GREEN + TM1638_COLOR_RED, i-10);
               break;
           }
         }
       }
     }
   }
    else if(type == 'M')
     {
     char modedata[6];
     int count = Serial.readBytes(modedata, 6);
     if(count == 6)
     {
       switch(modedata[0])
       {
         case 'B':
           blinken(modedata[1], modedata[2], modedata[3], modedata[4], modedata[5]);
           break;
         case 'F':
           fade(modedata[1], modedata[2], modedata[3], modedata[4], modedata[5]);
           break;
       }
     }   
  }
}

  byte buttons = module.getButtons();
  if(buttons != prev)
  {
    Serial.write('B');
    Serial.write(buttons);
    prev = buttons;
    
  }

}

//modes

void blinken(int r, int g, int b, int del, int times) {
  int dil = del*100;
  for(int x = 0; x < times; x++)
  {
    for (int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Color(r,g,b));
    }
    strip.show();
    delay(dil);
    for (int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Color(0,0,0));
    }
    strip.show();
    delay(dil);

  }
}

void fade(int r, int g, int b, int del, int times)
{
  
}
// --------------------------------------------
//     create initial image
// --------------------------------------------
void showWaitingPic() {
  for (int i=0; i < strip.numPixels(); i++) {
    if((i % 2) == 0)
    {
      strip.setPixelColor(i, Color(255,0,0));
    }
        strip.show();
    }
    delay(500);
    for (int i=0; i < strip.numPixels(); i++) {
    if((i % 2) != 0)
    {
      strip.setPixelColor(i, Color(0,0,255));
    }
    else
    {
      strip.setPixelColor(i, Color(0,0,0));
    }
     strip.show();
    }
}

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t Wheel(byte WheelPos)
{
  if (WheelPos < 85) {
   return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
   WheelPos -= 85;
   return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170; 
   return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}



// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}
