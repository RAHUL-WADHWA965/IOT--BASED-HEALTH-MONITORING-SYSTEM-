#include <LiquidCrystal.h> // Library for controlling the LCD display
#include <SoftwareSerial.h> // Library for software serial communication
#include <OneWire.h> // Library for one-wire communication
#include <DallasTemperature.h> // Library for Dallas temperature sensors
#define USE_ARDUINO_INTERRUPTS true // Enable Arduino interrupts for PulseSensor
#include <PulseSensorPlayground.h> // Library for PulseSensor functionality

// Define software serial pins for communication with ESP module
SoftwareSerial esp(10, 11); 

// Initialize the LCD with corresponding pins
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// Define the pin for the one-wire bus
#define ONE_WIRE_BUS 9
#define TEMPERATURE_PRECISION 12 // Precision for temperature readings

OneWire oneWire(ONE_WIRE_BUS); // Initialize one-wire communication
DallasTemperature sensors(&oneWire); // Attach DallasTemperature to the one-wire bus
DeviceAddress tempDeviceAddress; // To store the address of temperature sensors

int numberOfDevices, temp; // Number of devices and temperature reading
int buzzer = 8; // Pin connected to the buzzer

// Pulse sensor settings
const int PulseWire = A0; // Analog pin for PulseSensor
int myBPM; // To store beats per minute
int Threshold = 550; // Threshold value for PulseSensor
PulseSensorPlayground pulseSensor; // Initialize PulseSensor object

// Timer settings for uploading data
unsigned long previousMillis = 0; 
const long interval = 5000; // Interval for sending data to ESP module (5 seconds)

void setup()
{
  lcd.begin(16, 2); // Initialize the LCD (16x2 characters)
  Serial.begin(9600); // Start serial communication for debugging
  esp.begin(115200); // Start software serial communication with ESP

  sensors.begin(); // Initialize temperature sensors
  numberOfDevices = sensors.getDeviceCount(); // Get the count of connected devices

  // Configure PulseSensor settings
  pulseSensor.analogInput(PulseWire);
  pulseSensor.setThreshold(Threshold);
  pulseSensor.begin();

  pinMode(buzzer, OUTPUT); // Set buzzer pin as output
  digitalWrite(buzzer, HIGH); // Turn buzzer on briefly

  // Display welcome message on LCD
  lcd.setCursor(0, 0);
  lcd.print("  IoT Patient");
  lcd.setCursor(0, 1);
  lcd.print(" Monitor System");
  delay(1500); // Pause for the welcome message
  digitalWrite(buzzer, LOW); // Turn buzzer off
  lcd.clear(); // Clear LCD display
}

void loop()
{
  // Get heart rate (BPM) from PulseSensor
  myBPM = pulseSensor.getBeatsPerMinute();
  if (pulseSensor.sawStartOfBeat()) // Check if a beat is detected
  {
    beep(); // Activate buzzer for heartbeat
    lcd.setCursor(0, 1);
    lcd.print("HEART:");
    lcd.print(myBPM); // Display BPM on LCD
    lcd.setCursor(9, 1);
    lcd.print(" BPM");
    delay(20); // Brief delay for display update
  }

  // Request temperature readings from sensors
  sensors.requestTemperatures();
  for (int i = 0; i < numberOfDevices; i++)
  {
    if (sensors.getAddress(tempDeviceAddress, i)) // Check if sensor address is valid
    {
      temp = printTemperature(tempDeviceAddress); // Get temperature
      lcd.setCursor(0, 0);
      lcd.print("BODY:");
      lcd.print(temp); // Display temperature on LCD
      lcd.print(" *C");
    }
  }

  upload(); // Send data to ESP module
}

// Function to get and return temperature in Celsius
int printTemperature(DeviceAddress deviceAddress)
{
  int tempC = sensors.getTempC(deviceAddress); // Get temperature in Celsius
  return tempC;
}

// Function to activate buzzer briefly
void beep()
{
  digitalWrite(buzzer, HIGH); // Turn buzzer on
  delay(150); // Delay for beep duration
  digitalWrite(buzzer, LOW); // Turn buzzer off
}

// Function to upload data (BPM and temperature) to ESP module
void upload()
{
  unsigned long currentMillis = millis(); // Get current time in milliseconds
  if (currentMillis - previousMillis >= interval) // Check if interval has passed
  {
    previousMillis = currentMillis; // Update previousMillis
    esp.print('*'); // Start character for data
    esp.print(myBPM); // Send BPM
    esp.print(temp); // Send temperature
    esp.println('#'); // End character for data
  }
}
