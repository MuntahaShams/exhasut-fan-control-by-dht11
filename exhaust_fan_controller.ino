#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <LiquidCrystal.h>
#define DHTPIN 4                    // Digital pin for temperature and humidity sensor
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);           // Initialize temperature and humidity sensor
LiquidCrystal lcd(12, 11, 8, 7, 6, 5);  // Initialize LCD screen

const int tempUpButton = 2;         // Digital pin for temperature up button
boolean lastTempUpButton = LOW;
boolean currentTempUpButton = LOW;

const int tempDownButton = 3;       // Digital pin for temperature down button
boolean lastTempDownButton = LOW;
boolean currentTempDownButton = LOW;

unsigned long currentTime = 0;
unsigned long lastFanCheckTime = 0;

volatile int fanOnTemp = 30;        // Initial temp which fan should turn on
int fanStatus = 0;                  // Initial fan status set to off
int remainingSeconds = 10;          // Count down timer set to 10 seconds
const int fanControl = 9;           // Digital pin for solid state relay for fan

int tempInt = 0;
int humInt = 0;
boolean timeToCheck = HIGH;

// The setup function runs once when you press reset or power the board
void setup() {
  
  pinMode(fanControl, OUTPUT);      // Set fan control digital pin as an output pin
  pinMode(tempUpButton, INPUT);     // Set temperature up digital pin as an input pin
  pinMode(tempDownButton, INPUT);   // Set temperature down digital pin as an input pin
  
  // Turn fan off initially 
  digitalWrite(fanControl, LOW);  

  // Initialize LCD screen
  lcd.begin(20, 4);
  lcd.clear();
  
  // Position cursor and print labels
  lcd.setCursor(0, 0);
  lcd.print("  Fan: OFF");

  lcd.setCursor(0, 1);
  lcd.print(" Temp: ");
  lcd.print(tempInt);
  lcd.print(" c");

  lcd.setCursor(0, 2);
  lcd.print("On at: ");
  lcd.print(fanOnTemp);
  lcd.print(" c");
  
  lcd.setCursor(0, 3);
  lcd.print("Humid: ");
  lcd.print(humInt);
  lcd.print(" %");
  
}

// The loop function runs over and over again forever
void loop() {

  // Get current millis
  currentTime = millis();

  // Check status of fan every second 
  if (currentTime >= (lastFanCheckTime + 1000)) {

    // If current temperature is temp at which fan should be turned on or a higher temp...
    if (tempInt >= fanOnTemp) {
      
      // If fan is currently off then turn it on
      if (fanStatus == 0) {
        fanStatus = 1;
        digitalWrite(fanControl, HIGH);  
      }    
      
      // Reset count down timer to 10 seconds
      remainingSeconds = 10;
      
      // Update display to indicate that fan is on
      lcd.setCursor(7, 0);
      lcd.print("ON           ");
      
    }
    // Else current temp is below fan triggering temp...
    else {
      
      // If fan is currently on...
      if (fanStatus == 1) {
        
        // If count down timer has not run down to zero...
        if (remainingSeconds > 0) {
          
          // Show remaining count down seconds in display
          lcd.setCursor(7, 0);
          lcd.print("Off in ");
          lcd.print(remainingSeconds);
          lcd.print(" ");
          
          // Decrement countdown timer by one second
          remainingSeconds = remainingSeconds - 1;
        }
        
        // Else count down timer has reached zero...
        else {
  
          // Turn the fan off
          fanStatus = 0;
          digitalWrite(fanControl, LOW);  
          
          // Update display to show that fan is now off
          lcd.setCursor(7, 0);
          lcd.print("OFF          ");
        }
      }
    }
    lastFanCheckTime = currentTime;
    timeToCheck = HIGH;
  }

  // Read temperature sensor every 250 milliseconds after fan status has been checked
  // This avoids countdown timer not being updated right on the second since reading sensor takes about
  // 250 milliseconds and can therefore delay refresh of remaining seconds on LCD screen
  if ((currentTime >= (lastFanCheckTime + 250)) && timeToCheck == HIGH) {
  
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    
    // Check if any reads failed and proceed only if none failed
    if (!isnan(h) && !isnan(t)) {

      humInt = h;
      tempInt = t;
  
      // Show current temperature and humidity on LCD screen
      lcd.setCursor(7, 1);
      lcd.print(tempInt);
      lcd.print(" c");
  
      lcd.setCursor(7, 3);
      lcd.print(humInt);
      lcd.print(" %");
  
    }
    timeToCheck = LOW;    
  }

  // Read button to increase temperature at which fan should turn on
  currentTempUpButton = debounce(lastTempUpButton, tempUpButton);
  if (lastTempUpButton == LOW && currentTempUpButton == HIGH) {

    // If temp to turn on fan is less than 35 degrees...
    if (fanOnTemp < 35) {
      
      // Increase temp to turn fan on by one degree
      fanOnTemp = fanOnTemp + 1;
      
      // Update display to show new Fan On temp
      lcd.setCursor(7, 2);
      lcd.print(fanOnTemp);
      lcd.print(" c");
    }
  }
  lastTempUpButton = currentTempUpButton;

  // Read button to decrease temperature at which fan should turn on
  currentTempDownButton = debounce(lastTempDownButton, tempDownButton);
  if (lastTempDownButton == LOW && currentTempDownButton == HIGH) {

    // If temp to turn on fan is greater than 25 degrees...
    if (fanOnTemp > 20) {
      
      // Decrement fan turn on temp by one degree
      fanOnTemp = fanOnTemp - 1;
      
      // Update display to show new Fan On temp
      lcd.setCursor(7, 2);
      lcd.print(fanOnTemp);
      lcd.print(" c");
    }
  }
  lastTempDownButton = currentTempDownButton;
}

//A debouncing function that can be used for any button
boolean debounce(boolean last, int pin) {
  boolean current = digitalRead(pin);
  if (last != current) {
    delay(5);
    current = digitalRead(pin);
  }
  return current;
}
