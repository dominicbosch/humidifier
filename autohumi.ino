#include <LiquidCrystal.h>
#include <idDHT11.h>

int motorPin = 9;
int lcdbuf = 16;
LiquidCrystal lcd(12, 11, 6, 5, 4, 3);

void dht11_wrapper();
idDHT11 DHT11(2, 0, dht11_wrapper); // pin, interrupt, callback
// mapping pin to interrupt should be over: interrupt = digitalPinToInterrupt(pin)

void setup() {
  pinMode(motorPin, OUTPUT);
  
  lcd.begin(lcdbuf, 2);
  lcd.print("Auto Humidifier");
  lcd.setCursor(0, 1);
  lcd.print("Initialising...");
  Serial.begin(9600);
  Serial.println("Initialising automatic humidifier");
  
  delay(2000);
  lcd.clear();
}

void dht11_wrapper() {
  DHT11.isrCallback();
}

void loop() {
  Serial.print("\nRetrieving information from sensor: ");
  Serial.print("Read sensor: ");
  //delay(100);
  
  int result = DHT11.acquireAndWait();
  switch (result) {
    case IDDHTLIB_OK: 
      Serial.println("OK"); 
      break;
    case IDDHTLIB_ERROR_CHECKSUM: 
      Serial.println("Error\n\r\tChecksum error"); 
      break;
    case IDDHTLIB_ERROR_ISR_TIMEOUT: 
      Serial.println("Error\n\r\tISR time out error"); 
      break;
    case IDDHTLIB_ERROR_RESPONSE_TIMEOUT: 
      Serial.println("Error\n\r\tResponse time out error"); 
      break;
    case IDDHTLIB_ERROR_DATA_TIMEOUT: 
      Serial.println("Error\n\r\tData time out error"); 
      break;
    case IDDHTLIB_ERROR_ACQUIRING: 
      Serial.println("Error\n\r\tAcquiring"); 
      break;
    case IDDHTLIB_ERROR_DELTA: 
      Serial.println("Error\n\r\tDelta time to small"); 
      break;
    case IDDHTLIB_ERROR_NOTSTARTED: 
      Serial.println("Error\n\r\tNot started"); 
      break;
    default: 
      Serial.println("Unknown error"); 
      break;
  }
  
  float humi = DHT11.getHumidity();
  float temp = DHT11.getCelsius();

  lcd.setCursor(0, 0);
  lcd.print("Humidity: ");
  lcd.setCursor(12, 0);
  lcd.print(humi);
  Serial.print("Humidity (%): ");
  Serial.println(humi);
  
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.setCursor(12, 1);
  lcd.print(temp);
  Serial.print("Temperature (°C): ");
  Serial.println(temp);
  
  Serial.print("Dew Point (oC): ");
  Serial.println(DHT11.getDewPoint());
  
  if(temp > 26 || humi > 36) {
    digitalWrite(motorPin, HIGH);
  } else {
    digitalWrite(motorPin, LOW);
  }

  delay(2000);
}
