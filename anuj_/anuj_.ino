#include <Wire.h>
#include "AD7747.h"

AD7747 cap(_AD7747_ADDR_READ_, _AD7747_ADDR_WRITE_);


void setup()
{
  cap.debug_mode = false; //use it to activate log
  Wire.begin();
  Serial.begin(9600);
  bool isConnect = cap.isDeviceConnected();//check device AD7747 is connect?

  

  if (isConnect)
  {
    cap.loadSettings();//load settings in AD7747
    cap.write8_AD77(0x0A, 0x21);
    Serial.println("connected");
  }
}

void loop()
{
  char charBuf[16];
  uint32_t raw = cap.getCap(CAP_DATA_START);

  if(raw != 0)
  {
    double cap_pf = cap.capPF(raw);
    Serial.println(cap_pf, 6);
  }
}

