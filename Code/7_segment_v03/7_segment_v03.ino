// AdiClock V0.3


// Libraries
// PCA8596
#include <Adafruit_PWMServoDriver.h>    //https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library

// NTP
#include <EasyNTPClient.h>    //https://github.com/aharshac/EasyNTPClient
#include <WiFi.h>   //Included in the ESP pakage
// WiFi
#include <WiFiUdp.h> //Included in the ESP pakage
//Time management
#include <Timezone.h> //https://github.com/JChristensen/Timezone



//The following table contains some of the address combinations possible
// +----------+---------+----+----+----+----+----+----+
// | Board N° | Address | A0 | A1 | A2 | A3 | A4 | A5 |
// +----------+---------+----+----+----+----+----+----+
// |        0 | Ox40    |  0 |  0 |  0 |  0 |  0 |  0 |
// |        1 | Ox41    |  1 |  0 |  0 |  0 |  0 |  0 |
// |        2 | Ox42    |  0 |  1 |  0 |  0 |  0 |  0 |
// |        3 | Ox43    |  1 |  1 |  0 |  0 |  0 |  0 |
// +----------+---------+----+----+----+----+----+----+

Adafruit_PWMServoDriver pwmH = Adafruit_PWMServoDriver(0x40);    //Create an object of Hour driver (No Adress Jumper)
Adafruit_PWMServoDriver pwmM = Adafruit_PWMServoDriver(0x41);    //Create an object of Minute driver (A0 Address Jumper)
Adafruit_PWMServoDriver pwmC = Adafruit_PWMServoDriver(0x42);    //Create an object of H of hour driver (A1 Address Jumper)





// Wifi Setup
const char* ssid = "YNCREA_LAB";
const char* password = "813nV3nue@";

//Creation of WIFI UDP object and NTP client
WiFiUDP ntpUDP;
EasyNTPClient timeClient(ntpUDP, "fr.pool.ntp.org");

//Time change rules
//TimeChangeRule myRule = {abbrev, week, dow, month, hour, offset};
//myRule name of the rule
//week : is the week of the month when the rule begins.
//dow : is the day of the week when the rule begins.
//hour : is the hour in local time when the rule starts (0-23).
//offset is the UTC offset in minutes for the time zone being defined.

//For convenience, the following symbolic names can be used:
//week: First, Second, Third, Fourth, Last
//dow: Sun, Mon, Tue, Wed, Thu, Fri, Sat
//month: Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec


TimeChangeRule summerTime = {"RHEE", Last, Sun, Mar, 2, 120};
TimeChangeRule winterTime = {"RHHE", Last, Sun, Oct, 3, 60};

Timezone ConvertHour(summerTime, winterTime);



int servoFrequency = 50;      //Set servo operating frequency

int segmentHOn[14] = {200, 200, 200, 400, 400, 400, 200, 200, 200, 200, 400, 400, 400, 400}; //On positions for each HOUR servo
int segmentMOn[14] = {200, 200, 200, 400, 400, 400, 200, 200, 200, 200, 400, 400, 400, 400}; //On positions for each MINUTE servo
int segmentHOff[14] = {400, 400, 400, 200, 200, 200, 400, 400, 400, 400, 200, 200, 200, 200}; //Off positions for each HOUR servo
int segmentMOff[14] = {400, 400, 400, 200, 200, 200, 400, 400, 400, 400, 200, 200, 200, 200}; //Off positions for each MINUTE servo
int digits[10][7] = {{1,1,1,1,1,1,0},{0,1,1,0,0,0,0},{1,1,0,1,1,0,1},{1,1,1,1,0,0,1},{0,1,1,0,0,1,1},{1,0,1,1,0,1,1},{1,0,1,1,1,1,1},{1,1,1,0,0,0,0},{1,1,1,1,1,1,1},{1,1,1,1,0,1,1}}; //Position values for each digit

int h; // Create a variable to store the current hour
int m; // Create a cariable to store the current minut

int hourTens = 0;           //Create variables to store each 5 module numeral's to
int hourUnits = 0;
int minuteTens = 0;
int minuteUnits = 0;

int prevHourTens = 8;           //Create variables to store the previous numeral displayed on each module
int prevHourUnits = 8;
int prevMinuteTens = 8;
int prevMinuteUnits = 8;

int midOffset = 100;            //Amount by which adjacent segments to mid move away when required

void setup()
{
  //Sarting the serial 
  Serial.begin(9600);


  //Starting WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connection to wifi not established");
  }


  // PCA9685 setup
  pwmH.begin();   //Start each board
  pwmM.begin();
  pwmC.begin();
  pwmH.setOscillatorFrequency(27000000);    //Set the PWM oscillator frequency, used for fine calibration
  pwmM.setOscillatorFrequency(27000000);
  pwmC.setOscillatorFrequency(27000000);
  pwmH.setPWMFreq(servoFrequency);          //Set the servo operating frequency
  pwmM.setPWMFreq(servoFrequency);
  pwmC.setPWMFreq(servoFrequency);

  // Servo test
  for (int i = 0 ; i <= 13 ; i++) //Set all of the servos to on or up (88:88 displayed)
  // Debug when the H board is added change config to display 88 H 88
  {
    pwmH.setPWM(i, 0, segmentHOn[i]);
    delay(1000);
    pwmH.setPWM(i, 0, segmentHOff[i]);
    delay(1000);
  }
  delay(2000);
}

void loop()
{
  //NTP
  //Variable de la fonction
  time_t Heure;

  Heure = ConvertHour.toLocal(timeClient.getUnixTime());


  h = hour(Heure);
  m = minute(Heure);
  

  hourTens = hour(Heure) / 10;
  hourUnits = hour(Heure) % 10 ; 
  
  
  minuteTens = minute(Heure) / 10;
  minuteUnits = minute(Heure) % 10 ; 
  

  if (minuteUnits != prevMinuteUnits)   //If minute units has changed, update display
    updateDisplay();

    // Debug Only to be remouve for producion
    Serial.println("display updated");

    prevHourTens = hourTens;           //Update previous displayed numerals
    prevHourUnits = hourUnits;
    prevMinuteTens = minuteTens;
    prevMinuteUnits = minuteUnits;

    // Debug Only to be remouve for producion

    Serial.println(hourTens);
    Serial.println(hourUnits);
    Serial.println(minuteTens);
    Serial.println(minuteUnits);
    Serial.println ("");

  delay(1000);
}

void updateDisplay ()
{
  updateMid();
  for (int i = 0 ; i <= 5 ; i++)
  {
    if (digits[hourTens][i] == 1)
      pwmH.setPWM(i + 7, 0, segmentHOn[i + 7]);
    else
      pwmH.setPWM(i + 7, 0, segmentHOff[i + 7]);
    delay(10);
    if (digits[hourUnits][i] == 1)
      pwmH.setPWM(i, 0, segmentHOn[i]);
    else
      pwmH.setPWM(i, 0, segmentHOff[i]);
    delay(10);
    if (digits[minuteTens][i] == 1)
      pwmM.setPWM(i + 7, 0, segmentMOn[i + 7]);
    else
      pwmM.setPWM(i + 7, 0, segmentMOff[i + 7]);
    delay(10);
    if (digits[minuteUnits][i] == 1)
      pwmM.setPWM(i, 0, segmentMOn[i]);
    else
      pwmM.setPWM(i, 0, segmentMOff[i]);
    delay(10);
  }
}





void updateMid()//evite les contacts entre g et c e 
{
  if (digits[minuteUnits][6] != digits[prevMinuteUnits][6])   //Move adjacent segments for Minute units
  {
    if (digits[prevMinuteUnits][1] == 1)
      pwmM.setPWM(1, 0, segmentMOn[1] + midOffset);
    if (digits[prevMinuteUnits][6] == 1)                  //debug : + et - inversé 
      pwmM.setPWM(5, 0, segmentMOn[5] - midOffset);
  }
  delay(100);
  if (digits[minuteUnits][6] == 1)
    pwmM.setPWM(6, 0, segmentHOn[6]);
  else
    pwmM.setPWM(6, 0, segmentHOff[6]);
  if (digits[minuteTens][6] != digits[prevMinuteTens][6])   //Move adjacent segments for Minute tens
  {
    if (digits[prevMinuteTens][1] == 1)
      pwmM.setPWM(8, 0, segmentMOn[8] + midOffset);
    if (digits[prevMinuteTens][6] == 1)               //debug : + et - inversé 
      pwmM.setPWM(12, 0, segmentMOn[12] - midOffset);
  }
  delay(100);
  if (digits[minuteTens][6] == 1)
    pwmM.setPWM(13, 0, segmentHOn[13]);
  else
    pwmM.setPWM(13, 0, segmentHOff[13]);
  if (digits[hourUnits][6] != digits[prevHourUnits][6])   //Move adjacent segments for Hour units
  {
    if (digits[prevHourUnits][1] == 1)
      pwmH.setPWM(1, 0, segmentHOn[1] + midOffset);
    if (digits[prevHourUnits][6] == 1)           //debug : + et - inversé 
      pwmH.setPWM(5, 0, segmentHOn[5] - midOffset);
  }
  delay(100);
  if (digits[hourUnits][6] == 1)
    pwmH.setPWM(6, 0, segmentHOn[6]);
  else
    pwmH.setPWM(6, 0, segmentHOff[6]);
  if (digits[hourTens][6] != digits[prevHourTens][6])   //Move adjacent segments for Hour tens
  {
    if (digits[prevHourTens][1] == 1)
      pwmH.setPWM(8, 0, segmentHOn[8] + midOffset);
    if (digits[prevHourTens][6] == 1)                   //debug : + et - inversé 
      pwmH.setPWM(12, 0, segmentHOn[12] - midOffset);
  }
  delay(100);
  if (digits[hourTens][6] == 1)
    pwmH.setPWM(13, 0, segmentHOn[13]);
  else
    pwmH.setPWM(13, 0, segmentHOff[13]);
}