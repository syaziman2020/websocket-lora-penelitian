
//---------------------------------------- Include Library.
#include <SPI.h>
#include <LoRa.h>
//----------------------------------------


//---------------------------------------- LoRa Pin / GPIO configuration.
#define ss 5
#define rst 14
#define dio0 2
//----------------------------------------

//float sensor pin
#define water1 34
#define water2 35
#define water3 32
#define water4 33
#define water5 25

//----------------------------------------String variable for LoRa
String Incoming = "";
String Message = "";

//----------------------------------------


// byte LocalAddress = 0x02;       //--> address of this device (Slave 1).
byte LocalAddress = 0x03;        //--> address of this device (Slave 2).
byte Destination_Master = 0x01;  //--> destination to send to Master (ESP32).
//----------------------------------------

//---------------------------------------- Variable declarations for the reading status of the DHT11 sensor, temperature and humidity values.
int sensor = 0;
int totalHeight = 0;
String send_Status_Read_Sensor = "";
//----------------------------------------

//---------------------------------------- Variable declaration for Millis/Timer.
unsigned long previousMillis_Sensor = 0;
const long interval_UpdateSensor = 3000;

unsigned long previousMillis_RestartLORA = 0;
const long interval_RestartLORA = 1000;
//----------------------------------------

// Declaration of variable as counter to restart Lora Ra-02.
byte Count_to_Rst_LORA = 0;

//________________________________________________________________________________ Subroutines for sending data (LoRa Ra-02).
void sendMessage(String Outgoing, byte Destination) {
  LoRa.beginPacket();             //--> start packet
  LoRa.write(Destination);        //--> add destination address
  LoRa.write(LocalAddress);       //--> add sender address
  LoRa.write(Outgoing.length());  //--> add payload length
  LoRa.print(Outgoing);           //--> add payload
  LoRa.endPacket();               //--> finish packet and send it
}
//________________________________________________________________________________

//________________________________________________________________________________ Subroutines for receiving data (LoRa Ra-02).
void onReceive(int packetSize) {
  if (packetSize == 0) return;  // if there's no packet, return

  //---------------------------------------- read packet header bytes:
  int recipient = LoRa.read();        //--> recipient address
  byte sender = LoRa.read();          //--> sender address
  byte incomingLength = LoRa.read();  //--> incoming msg length
  byte master_Send_Mode = LoRa.read();
  //----------------------------------------

  //---------------------------------------- Condition that is executed if message is not from Master.
  if (sender != Destination_Master) {
    Serial.println();
    Serial.println("i");  //--> "i" = Not from Master, Ignore.
    //Serial.println("Not from Master, Ignore");

    // Resets the value of the Count_to_Rst_LORA variable.
    Count_to_Rst_LORA = 0;
    return;  //--> skip rest of function
  }
  //----------------------------------------

  // Clears Incoming variable data.
  Incoming = "";

  //---------------------------------------- Get all incoming data.
  while (LoRa.available()) {
    Incoming += (char)LoRa.read();
  }
  //----------------------------------------

  // Resets the value of the Count_to_Rst_LORA variable.
  Count_to_Rst_LORA = 0;

  //---------------------------------------- Check length for error.
  if (incomingLength != Incoming.length()) {
    Serial.println();
    Serial.println("er");  //--> "er" = error: message length does not match length.
    //Serial.println("error: message length does not match length");
    return;  //--> skip rest of function
  }
  //----------------------------------------

  //---------------------------------------- Checks whether the incoming data or message for this device.
  if (recipient != LocalAddress) {
    Serial.println();
    Serial.println("!");  //--> "!" = This message is not for me.
    //Serial.println("This message is not for me.");
    return;  //--> skip rest of function
  } else {
    // if message is for this device, or broadcast, print details:
    Serial.println();
    Serial.println("Rc from: 0x" + String(sender, HEX));
    Serial.println("Message: " + Incoming);

    // Calls the Processing_incoming_data() subroutine.
    if (master_Send_Mode == 1) Processing_incoming_data();
  }
}
//________________________________________________________________________________

//________________________________________________________________________________ Subroutine to process the data to be sent, after that it sends a message to the Master.
void Processing_incoming_data() {

  // Fill in the "Message" variable with the DHT11 sensor reading status, humidity value, temperature value, state of LED 1 and LED 2.
  Message = "";
  Message = send_Status_Read_Sensor + "," + String(sensor);

  Serial.println();
  Serial.println("Tr to  : 0x" + String(Destination_Master, HEX));
  Serial.println("Message: " + Message);

  // Send a message to Master.
  sendMessage(Message, Destination_Master);
}
//________________________________________________________________________________


//________________________________________________________________________________ String function to process the data received
// I got this from : https://www.electroniclinic.com/reyax-lora-based-multiple-sensors-monitoring-using-arduino/
String GetValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
//________________________________________________________________________________

//________________________________________________________________________________ Subroutine to reset Lora Ra-02.
void Rst_LORA() {
  LoRa.setPins(ss, rst, dio0);

  Serial.println();
  Serial.println(F("Restart LoRa..."));
  Serial.println(F("Start LoRa init..."));
  if (!LoRa.begin(433E6)) {  // initialize ratio at 915 or 433 MHz
    Serial.println(F("LoRa init failed. Check your connections."));
    while (true)
      ;  // if failed, do nothing
  }
  Serial.println(F("LoRa init succeeded."));

  // Resets the value of the Count_to_Rst_LORA variable.
  Count_to_Rst_LORA = 0;
}
//________________________________________________________________________________

//setup waterfloat sensor
void waterfloatSetup() {
  delay(500);
  pinMode(water1, INPUT_PULLUP);
  pinMode(water2, INPUT_PULLUP);
  pinMode(water3, INPUT_PULLUP);
  pinMode(water4, INPUT_PULLUP);
  pinMode(water5, INPUT_PULLUP);
  delay(1000);
}


int waterHeight(int pinWaterHeigh1, int pinWaterHeigh2, int pinWaterHeigh3, int pinWaterHeigh4, int pinWaterHeigh5) {
  int pins[5] = {pinWaterHeigh1, pinWaterHeigh2, pinWaterHeigh3, pinWaterHeigh4, pinWaterHeigh5};
  int totalHeight = 100;

  for (int i = 0; i < 5; i++) {
    int count = 5;
    Serial.print("ini pin ke : ");
    Serial.println(i);
    // Membaca nilai dari pin sebanyak 5 kali
    for (int j = 0; j < 5; j++) {
      int reading = analogRead(pins[i]);
      Serial.println(reading);
      
      if (reading < 4095) {
        count--;
      }
      
      
      delay(100); // Delay 100 ms antara setiap pembacaan
    }

    Serial.println("===========");
    // Jika nilai pembacaan lebih dari 4000 sebanyak 5 kali, tambahkan 20 cm ke totalHeight
    if (count <= 0) {
      totalHeight -= 20;
      // Serial.print("Pin ");
      // Serial.print(pins[i]);
      // Serial.println(" memenuhi kriteria, menambah 20 cm");
    } else {
      // Serial.print("Pin ");
      // Serial.print(pins[i]);
      // Serial.println(" tidak memenuhi kriteria, menghentikan pembacaan");
      break; // Hentikan loop jika pin tidak memenuhi kriteria
    }
  }

  return totalHeight;
}


//________________________________________________________________________________ VOID SETUP
void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  waterfloatSetup();
  // Calls the Rst_LORA() subroutine.
  Rst_LORA();
}
//________________________________________________________________________________

//________________________________________________________________________________ VOID LOOP
void loop() {
  // put your main code here, to run repeatedly:

  //---------------------------------------- Millis / Timer to update the temperature and humidity values ​​from the DHT11 sensor every 2 seconds (see the variable interval_UpdateSensor).
  unsigned long currentMillis_UpdateSensor = millis();

  if (currentMillis_UpdateSensor - previousMillis_Sensor >= interval_UpdateSensor) {
    previousMillis_Sensor = currentMillis_UpdateSensor;

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    
    // sensor = random(100);
    sensor = waterHeight(water1, water2, water3, water4, water5);

    Serial.print("Total ketinggian air: ");
    Serial.print(sensor);
    Serial.println(" cm");

    // Check if any reads failed and exit early (to try again).
    if (isnan(sensor)) {
      sensor = totalHeight;
      send_Status_Read_Sensor = "f";
      Serial.println(F("Failed to read from sensor!"));
      return;
    } else {
      send_Status_Read_Sensor = "s";
    }
  }
  //----------------------------------------

  //---------------------------------------- Millis/Timer to reset Lora Ra-02.
  // Please see the Master program code for more detailed information about the Lora reset method.

  unsigned long currentMillis_RestartLORA = millis();

  if (currentMillis_RestartLORA - previousMillis_RestartLORA >= interval_RestartLORA) {
    previousMillis_RestartLORA = currentMillis_RestartLORA;

    Count_to_Rst_LORA++;
    if (Count_to_Rst_LORA > 30) {
      LoRa.end();
      Rst_LORA();
    }
  }
  //----------------------------------------

  //---------------------------------------- parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
  //----------------------------------------
}
