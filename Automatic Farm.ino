#include <dht.h>
dht DHT;
#define DHT11_PIN 10
int Contrast = 75;
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
int soilSensor = A0;
int methaneSensor = A1;
LiquidCrystal lcd(13, 12, 5, 4, 3, 2);
int buzzer = 9;
int pump = 11;
#define RX 7
#define TX 8
String AP = "arduino";
String PASS = "bitcoinh";
String API = "OTTV2ZD3KWL4N5BW";   // Write API KEY
String HOST = "api.thingspeak.com";
String PORT = "80";
String temperatureField = "field1";
String humidityField = "field2";
String soilMoistureField = "field3";
String metahneField = "field4";
int countTrueCommand;
int countTimeCommand;
boolean found = false;
String info;


int soil_value;
int  temp_value;
int hum_value;
int gas_value;
int tempSensor = 1;


SoftwareSerial esp8266(RX, TX);


void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  pinMode(buzzer, OUTPUT);
  pinMode(methaneSensor, INPUT);
  pinMode(soilSensor, INPUT);
  pinMode(pump, OUTPUT);
  digitalWrite(pump, HIGH);
  analogWrite(6, Contrast);
  esp8266.begin(115200);
  writeLcd("Warming Up", "Gimme a sec");
  delay(3000);
  writeLcd("Starting Please", "Hold on");
  delay(1500);
  writeLcd("Setting Up", "Network");
  delay(1500);
  sendCommand("AT", 5, "OK");

  writeLcd("Enabling wifi", "Mode");
  delay(1500);
  sendCommand("AT+CWMODE=1", 5, "OK");

  writeLcd("Connecting to  ", AP + "'s wifi");
  delay(1500);
  sendCommand("AT+CWJAP=\"" + AP + "\",\"" + PASS + "\"", 20, "OK");
}

void loop() {
  
  updateAllData();
  writeToLcd();
  String getTempData = "GET /update?api_key=" + API + "&" + temperatureField + "=" + String(temp_value);
  String getHumData = "GET /update?api_key=" + API + "&" +  humidityField + "=" + String(hum_value);
  String getSoilData = "GET /update?api_key=" + API + "&" +  soilMoistureField + "=" + String(soil_value);
  String getGasData = "GET /update?api_key=" + API + "&" +  metahneField + "=" + String(gas_value);

  sendCommand("AT+CIPMUX=1", 5, "OK");

  sendToCloud(getTempData);
  writeToLcd();
  updateAllData();
  
  sendToCloud(getHumData);
  writeToLcd();
  updateAllData();
  
  sendToCloud(getSoilData);
  writeToLcd();
  updateAllData();
  
  sendToCloud(getGasData);
  writeToLcd();
  

  delay(2000);
  countTrueCommand++;
  sendCommand("AT+CIPCLOSE=0", 5, "OK");
}
void sprayWater() {
  digitalWrite(pump, LOW);
  delay(500);
  digitalWrite(pump, HIGH);
 
}
void buzzHandler(){
  if (soil_value>200 ||soil_value<200 ||gas_value>200 || temp_value>60){
    Serial.print("Warning = ");
    Serial.println("ON");
    buzz();
  }else{
    Serial.print("Warning = ");
    Serial.println("OFF");
    unbuzz();
  }
}
void buzz() {
  tone(buzzer, 1000);
  delay(100);
}
void unbuzz() {
  noTone(buzzer);
  delay(100);
}
void writeToLcd() {
  String displayDataLine1 = "SM:" + String((int)soil_value) + " Meth:" +  String((int)gas_value);
  String displayDataLine2 = "T:" +  String((int)temp_value) + " H:" +  String((int)hum_value) + " " + info;
  writeLcd(displayDataLine1, displayDataLine2);
}
void sendToCloud(String api) {
  sendCommand("AT+CIPSTART=0,\"TCP\",\"" + HOST + "\"," + PORT,     15      ,                "OK");
  sendCommand("AT+CIPSEND=0," + String(api.length() + 4),      4        , ">");
  esp8266.println(api);
}

void updateHumidityData() {
  //  return random(1000);
  int chk = DHT.read11(DHT11_PIN);

  Serial.print("Temperature = ");
  temp_value = (int)DHT.temperature;
  Serial.println(temp_value);

  Serial.print("Humidity = ");
  hum_value = (int)DHT.humidity;
  Serial.println( hum_value);
  // Replace with your own sensor code
}
void updateAllData() {
  updateHumidityData();
  updateSoilData();
  updateGas();
  buzzHandler();
  if(soil_value<200){
    sprayWater();
  }
  
  
}
void updateSoilData() {
  soil_value = analogRead(soilSensor);
  soil_value = map(soil_value, 550, 10, 0, 100);
  Serial.print("Soil Moisture = ");
  Serial.println( soil_value);

}
void updateGas() {
  //gas_value = digitalRead(methaneSensor);
  gas_value = analogRead(methaneSensor);
  //Serial.println(gas_value);
  Serial.print("Gas = ");
  Serial.println( gas_value);

}

void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while (countTimeCommand < (maxTime * 1))
  {
    esp8266.println(command);//at+cipsend
    if (esp8266.find(readReplay)) //ok
    {
      found = true;
      break;
    }

    countTimeCommand++;
  }

  if (found == true)
  {
    if (command != "AT+CIPCLOSE=0") {
      info = "OK   ";
    }
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }

  if (found == false)
  {
    if (command != "AT+CIPCLOSE=0") {
      info = "BAD   ";
    }
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }

  found = false;
}
void writeLcd(String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
  delay(500);
}

