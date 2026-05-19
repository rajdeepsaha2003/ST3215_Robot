#include <SCServo.h>

SMS_STS st;

#define S_RXD 16
#define S_TXD 17

int old_id = 3;
int new_id = 4;

void setup()
{
  Serial.begin(115200);

  Serial1.begin(1000000, SERIAL_8N1, S_RXD, S_TXD);

  st.pSerial = &Serial1;

  delay(1000);

  Serial.println("Changing Servo ID...");

  // Change ID 1 -> 2
  st.unLockEprom(old_id);

  st.writeByte(old_id, SMS_STS_ID, new_id);

  st.LockEprom(new_id);

  Serial.println("Done");
}

void loop()
{
} 