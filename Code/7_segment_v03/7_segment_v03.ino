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

Adafruit_PWMServoDriver pwmM = Adafruit_PWMServoDriver(0x40);   //Create an object of Minute driver (No Adress Jumper)
Adafruit_PWMServoDriver pwmC = Adafruit_PWMServoDriver(0x41);    //Create an object of H of hour driver (A0 Address Jumper)
Adafruit_PWMServoDriver pwmH = Adafruit_PWMServoDriver(0x42);    //Create an object of Hour driver (A1 Address Jumper)




// Wifi Setup
const char* ssid = "Your wifi ssid";
const char* password = "your wifi password";

//Creation of WIFI UDP object and NTP client
WiFiUDP ntpUDP;


//Define the server 
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


long prevMillis = 0;

int servoFrequency = 50;

int segmentHOff[14] = {400, 205, 205, 200, 395, 400, 405, 405, 200, 200, 210, 405, 400, 410}; //On positions for each HOUR servo
int segmentHOn[14] = {200, 405, 405, 400, 195, 200, 205, 205, 400, 400, 410, 205, 200, 210}; //Off positions for each HOUR servo



int segmentCOff[7] = {400, 200, 210, 210, 400, 410, 385}; //On positions for each H servo
int segmentCOn[7] = {200, 400, 410, 410, 200, 210, 185}; //Off positions for each H servo



int segmentMOff[14] = {415, 190, 192, 205, 380, 385, 415, 385, 200, 200, 190, 410, 400, 415}; //On positions for each MINUTE servo
int segmentMOn[14] = {215, 390, 392, 405, 180, 185, 215, 185, 400, 400, 390, 210, 200, 215}; //Off positions for each MINUTE servo

int digits[10][7] = {{1, 1, 1, 1, 1, 1, 0}, {0, 1, 1, 0, 0, 0, 0}, {1, 1, 0, 1, 1, 0, 1}, {1, 1, 1, 1, 0, 0, 1}, {0, 1, 1, 0, 0, 1, 1}, {1, 0, 1, 1, 0, 1, 1}, {1, 0, 1, 1, 1, 1, 1}, {1, 1, 1, 0, 0, 0, 0}, {1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 0, 1, 1}}; //Position values for each digit

int h; // Create a variable to store the current hour
int m; // Create a variable to store the current minute

int hourTens = 0;           //Create variables to store each 5 module numeral's to
int hourUnits = 0;
int minuteTens = 0;
int minuteUnits = 0;

int prevHourTens = 8;           //Create variables to store the previous numeral displayed on each module
int prevHourUnits = 8;
int prevMinuteTens = 8;
int prevMinuteUnits = 8;

int midOffset = 100;            //Amount by which adjacent segments to mid move away when required





void setup(){
  //Sarting serial protocol at 9600 bauds
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


  // Servo init
  // The following sequence will ensure no collisions can occur with the middle segment
  Serial.println("init servo");
  for (int i = 0 ; i <= 6 ; i++) { //Set all of the servos to on or up (88:88 displayed)
    // Debug when the H board is added change config to display 88 H 88

    pwmM.setPWM(i, 0, segmentMOff[i]);
    pwmM.setPWM(i + 7, 0, segmentMOff[i + 7]);
    delay(200);
  }
  for (int i = 0 ; i <= 6 ; i++) { //Set all of the servos to on or up (88:88 displayed)
    // Debug when the H board is added change config to display 88 H 88

    pwmH.setPWM(i, 0, segmentHOff[i]);
    pwmH.setPWM(i + 7, 0, segmentHOff[i + 7]);
    delay(200);
  }
  for (int i = 0 ; i <= 6 ; i++) { //Set all of the servos to on or up (88:88 displayed)
    // Debug when the H board is added change config to display 88 H 88

    pwmC.setPWM(i, 0, segmentCOff[i]);
    delay(200);
  }

  delay(500);
  pwmM.setPWM(6, 0, segmentMOn[6]);
  pwmM.setPWM(13, 0, segmentMOn[13]);
  pwmH.setPWM(6, 0, segmentHOn[6]);
  pwmH.setPWM(13, 0, segmentHOn[13]);
  pwmC.setPWM(6, 0, segmentCOn[6]);
  delay(1000);



  for (int i = 0 ; i <= 5 ; i++) { //Set all of the servos to on or up (88:88 displayed)
    // Debug when the H board is added change config to display 88 H 88

    pwmM.setPWM(i, 0, segmentMOn[i]);
    pwmM.setPWM(i + 7, 0, segmentMOn[i + 7]);
    delay(200);
  }
  for (int i = 0 ; i <= 5 ; i++) { //Set all of the servos to on or up (88:88 displayed)
    // Debug when the H board is added change config to display 88 H 88

    pwmH.setPWM(i, 0, segmentHOn[i]);
    pwmH.setPWM(i + 7, 0, segmentHOn[i + 7]);
    delay(200);
  }
  for (int i = 0 ; i <= 5 ; i++) { //Set all of the servos to on or up (88:88 displayed)
    // Debug when the H board is added change config to display 88 H 88

    pwmC.setPWM(i, 0, segmentCOn[i]);
    delay(200);
  }
  delay(1000);
  pwmC.setPWM(0, 0, segmentCOff[0]);
  pwmC.setPWM(3, 0, segmentCOff[3]);
}

void loop(){

  if (millis() - prevMillis > 1000) { // Wait a second between eatch request
    prevMillis = millis();

    time_t Heure;
    //Get time from server
    Heure = ConvertHour.toLocal(timeClient.getUnixTime());


    //Extract Units and Tens for Hour and Minute
    h = hour(Heure);
    m = minute(Heure);


    hourTens = hour(Heure) / 10;
    hourUnits = hour(Heure) % 10 ;


    minuteTens = minute(Heure) / 10;
    minuteUnits = minute(Heure) % 10 ;
  }

  if (minuteUnits != prevMinuteUnits) {  //If minute units has changed, update display
    updateDisplay();

  // Debug Only to be remouve for producion
  Serial.println("display updated");

  prevHourTens = hourTens;           //Update previous displayed numerals
  prevHourUnits = hourUnits;
  prevMinuteTens = minuteTens;
  prevMinuteUnits = minuteUnits;

    // Debug Only to be remove for production

  Serial.println(hourTens);
  Serial.println(hourUnits);
  Serial.println(minuteTens);
  Serial.println(minuteUnits);
  Serial.println ("");

  }
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





void updateMid() //Avoids contact between segments g and c e
{
  if (digits[minuteUnits][6] != digits[prevMinuteUnits][6])   //Move adjacent segments for Minute units
  {
    if (digits[prevMinuteUnits][1] == 1)
      pwmM.setPWM(1, 0, segmentMOn[1] - midOffset);
    if (digits[prevMinuteUnits][6] == 1)
      pwmM.setPWM(5, 0, segmentMOn[5] + midOffset);
  }
  delay(100);
  if (digits[minuteUnits][6] == 1)
    pwmM.setPWM(6, 0, segmentHOn[6]);
  else
    pwmM.setPWM(6, 0, segmentHOff[6]);
  if (digits[minuteTens][6] != digits[prevMinuteTens][6])   //Move adjacent segments for Minute tens
  {
    if (digits[prevMinuteTens][1] == 1)
      pwmM.setPWM(8, 0, segmentMOn[8] - midOffset);
    if (digits[prevMinuteTens][6] == 1)
      pwmM.setPWM(12, 0, segmentMOn[12] + midOffset);
  }
  delay(100);
  if (digits[minuteTens][6] == 1)
    pwmM.setPWM(13, 0, segmentHOn[13]);
  else
    pwmM.setPWM(13, 0, segmentHOff[13]);
  if (digits[hourUnits][6] != digits[prevHourUnits][6])   //Move adjacent segments for Hour units
  {
    if (digits[prevHourUnits][1] == 1)
      pwmH.setPWM(1, 0, segmentHOn[1] - midOffset);
    if (digits[prevHourUnits][6] == 1)
      pwmH.setPWM(5, 0, segmentHOn[5] + midOffset);
  }
  delay(100);
  if (digits[hourUnits][6] == 1)
    pwmH.setPWM(6, 0, segmentHOn[6]);
  else
    pwmH.setPWM(6, 0, segmentHOff[6]);
  if (digits[hourTens][6] != digits[prevHourTens][6])   //Move adjacent segments for Hour tens
  {
    if (digits[prevHourTens][1] == 1)
      pwmH.setPWM(8, 0, segmentHOn[8] - midOffset);
    if (digits[prevHourTens][6] == 1)
      pwmH.setPWM(12, 0, segmentHOn[12] + midOffset);
  }
  delay(100);
  if (digits[hourTens][6] == 1)
    pwmH.setPWM(13, 0, segmentHOn[13]);
  else
    pwmH.setPWM(13, 0, segmentHOff[13]);
}
