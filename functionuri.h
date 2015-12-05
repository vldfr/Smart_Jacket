

#ifndef functionuri_h
#define functionuri_h

#include "Arduino.h"

struct RGB{
  RGB(int red=-1, int green=-1, int blue=-1, bool CommAnode=false);
  void Blink(int r, int g, int b, int dur, int times);
  void show(int r, int g, int b);
  void fade(int r, int g, int b, int miliX);
  void colorize(int r, int g, int b);
}

#endif
