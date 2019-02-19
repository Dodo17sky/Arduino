#define DEBUG 1

//  ===================  GLOBAL DATA  ======================
String inputString = "";         // a String to hold incoming data
boolean stringComplete = false;  // whether the string is complete

// cmd 1 data ==============
byte outputPwmPin = 5;

// cmd 2 data ==============
byte inputAdcPin  = A1;

// cmd 3 data ==============
byte pwnFreqPin  = 2; // PWM frequency will be read with a digital pin
unsigned long downTime = 0;
unsigned long upTime = 0;
unsigned long downTicks = 0;
unsigned long upTicks = 0;

void processString(String in);
int getStringID(String in);
void showMenu();
void SetPwmVoltage();       // cmd 1
void ReadAdcPin();          // cmd 2
void PwmFrequencyPin5();    // cmd 3
unsigned int getPwmFrequency();
void PwmPeriodPin5();
void PwmUpDownTimePin5();

void setup() {
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  
  pinMode(outputPwmPin, OUTPUT);
  pinMode(pwnFreqPin, INPUT);
  
  Serial.begin(57600); // uart speed; 7200 bytes/sec

  showMenu();
}

void loop() {
    if (stringComplete) {
        processString(inputString);
        stringComplete = false;

        while(Serial.read() >= 0) ; // flush the receive buffer
        inputString = "";
        showMenu();
    }

}

void processString(String in)
{
    switch(getStringID(in))
    {
        case 1: // Set PWM voltage
            #if (DEBUG == 1)
            Serial.println("cmd ID 1");
            #endif
            SetPwmVoltage();
            break;

        case 2:
            #if (DEBUG == 1)
            Serial.println("cmd ID 2");
            #endif
            ReadAdcPin();
            break;

        case 3:
            #if (DEBUG == 1)
            Serial.println("cmd ID 3");
            #endif
            PwmFrequencyPin5();
            break;

        case 4:
            #if (DEBUG == 1)
            Serial.println("cmd ID 4");
            #endif
            PwmPeriodPin5();
            break;

        case 5:
            #if (DEBUG == 1)
            Serial.println("cmd ID 5");
            #endif
            PwmUpDownTimePin5();
            break;
            
        default:
            break;
    }
}

int getStringID(String in)
{
    if(String("1") == in)
        return 1;

    if(String("2") == in)
        return 2;

    if(String("3") == in)
        return 3;
    
    if(String("4") == in)
        return 4;

    if(String("5") == in)
        return 5;
        
    return 0;
}

void showMenu()
{
    Serial.println("\n1. Set PWM voltage");
    Serial.println("2. Read ADC value");
    Serial.println("3. PWM frequency on pin 5");
    Serial.println("4. PWM period pin 5");
    Serial.println("5. PWM up and down period on pin 5");
    
    Serial.print("Command: ");
}

void SetPwmVoltage()
{
    Serial.print("Enter pwm voltage (");
    Serial.print(outputPwmPin);
    Serial.print(") >> ");
    
    while (Serial.available() == 0);
    
    int val = Serial.parseInt();
    if(val >=0 && val <= 255){
      analogWrite(outputPwmPin, val);
    }
    Serial.print("PWM voltage set to ");
    Serial.println(val);
}

void ReadAdcPin()
{
    float adc = 0.0;
    // read ADC value as a average of 20 values
    for(byte i=0;i<20;i++) {
        delay(15);
        adc += (float)analogRead(inputAdcPin);
    }
    adc = adc/20; // average of 20 reads
    adc /= 1024;  // 10 bits ADC resolution
    adc *= 5;     // map to 5 V

    String result = String("Analog pin A1 = ") + adc + "V\n";
    Serial.print(result);
}

void PwmFrequencyPin5()
{
      unsigned long freq = 0;

      for(byte i=0; i<10; i++) {
          freq += getPwmFrequency();
      }

      freq /= 10;
      
      String result = String("PWM freq is ") + freq + " Hz\n";
      Serial.print(result);
}

void PwmPeriodPin5()
{
      getPwmFrequency();
      String result = String("PWM period is ") + (upTime+downTime) + " us\n";
      Serial.print(result);
}

void PwmUpDownTimePin5()
{
      getPwmFrequency();
      String result = String("PWM uptime = ") + upTime + "us; PWM downtime =  " + downTime + "us\n";
      Serial.print(result);
      result = String("PWM uptiks = ") + upTicks + "; PWM downticks =  " + downTicks + "\n";
      Serial.print(result);
}

unsigned int getPwmFrequency()
{
      unsigned long start  = 0;
      upTicks = 0;
      downTicks = 0;
      
      // don't know what pwm phase we match
      // so we skip two high-low phases to start high from begining
      while(digitalRead(pwnFreqPin) == HIGH);
      while(digitalRead(pwnFreqPin) == LOW);
      
      start = micros();               // new high phase started
      while(digitalRead(2) == HIGH)   // wait to finish high state
          upTicks++;
      upTime = micros() - start;      // high phase ended > measure high time

      start += upTime;                // new low phase started
      while(digitalRead(2) == LOW)    // wait to finish low state
          downTicks++;
      downTime = micros() - start;    // low phase ended > measure low time

      // devide 1 second (in microseconds = 1.000.000) to 1 pwm cycle (high+low)
      return (1000000 / (upTime+downTime) );
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
      // get the new byte:
      char inChar = (char)Serial.read();
      
      // if the incoming character is a newline, set a flag so the main loop can
      // do something about it:
      if (inChar == '\n') {
        stringComplete = true;
      }
      else {
        // add it to the inputString:
        inputString += inChar;
      }
   }
}

