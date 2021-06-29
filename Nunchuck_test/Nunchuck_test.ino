///////////////////////////////////////////////////////////////////////////////////////
//Terms of use
///////////////////////////////////////////////////////////////////////////////////////
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////////
#include <Wire.h>

byte error, MPU_6050_found, nunchuck_found, lowByte, highByte;
int address;
int nDevices;

void setup()
{
  Wire.begin();
  TWBR = 12;
  Serial.begin(9600);
}

void loop()
{
  Serial.println("Scanning I2C bus...");

  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16)Serial.print("0");
      Serial.println(address,HEX);
      nDevices++;

      if(address == 0x52){
        Serial.println("This could be a Nunchuck");
        Serial.println("Trying to initialise the device...");
        Wire.beginTransmission(0x52);
        Wire.write(0xF0);
        Wire.write(0x55);
        Wire.endTransmission();
        delay(20);
        Wire.beginTransmission(0x52);
        Wire.write(0xFB);
        Wire.write(0x00);
        Wire.endTransmission();
        delay(20);
        Serial.println("Sending joystick data request...");
        Wire.beginTransmission(0x52);
        Wire.write(0x00);
        Wire.endTransmission();
        Wire.requestFrom(0x52,1);
        while(Wire.available() < 1);
        lowByte = Wire.read();
        if(lowByte > 100 && lowByte < 160){
          Serial.print("Data response normal: ");
          Serial.println(lowByte);
          nunchuck_found = 1;
        }
        else{
          Serial.print("Data response is not normal: ");
          Serial.println(lowByte);
        }
      }
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
    
  if(nunchuck_found){
    Serial.println("Printing raw Nunchuck values");
    for(address = 0; address < 2000; address++ ){ 
      Wire.beginTransmission(0x52);
      Wire.write(0x00);
      Wire.endTransmission();
      Wire.requestFrom(0x52,2);
      while(Wire.available() < 2);
      Serial.print("Joystick X = "); 
      Serial.print(Wire.read());
      Serial.print(" Joystick y = ");
      Serial.println(Wire.read());
      delay(300);
    }
  }
  else Serial.println("No Nunchuck device found at address 0x52");
  while(1);
}
