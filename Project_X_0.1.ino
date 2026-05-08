/** PÄÄOHJELMA
  * Tämä tiedosto hoitaa Bluetooth-yhteyden, komentojen vastaanoton
  * ja laitteen toimintalogiikan.
  */

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "motor.h" 
#include "solenoid.h"
#include "fan.h"
#include "sensor.h"

// Bluetooth-komennot
#define CMD_ON     0x01 // Moottorin palautus
#define CMD_OFF    0x02 // Pallon lataus
#define CMD_LEFT   0x03 // Latausmoottori eteenpäin
#define CMD_RIGHT  0x04 // Latausmoottori taaksepäin
#define CMD_FIRE   0x05 // Tuuletin + solenoidi
#define CMD_LOAD   0x06 // Jousen(mailan) viritys

BLEServer* server;
BLECharacteristic* commandChar;

// BLE-palvelimen tilan seuranta (yhteys laitteeseen)
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    Serial.println("Device connected!");
  }
  void onDisconnect(BLEServer* pServer) {
    Serial.println("Device disconnected!");
    BLEDevice::startAdvertising();
  }
};

// Bluetooth-komentojen käsittely
class CommandCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *characteristic) {
    String value = characteristic->getValue(); // ← String iso S
    if (value.length() == 0) return;

    uint8_t cmd = value[0]; // Ensimmäinen tavu komennoksi

    switch (cmd) {

      case CMD_ON:
        Serial.println("CMD: ON");
        returnMotor();
        break;

      case CMD_OFF:
        Serial.println("CMD: OFF");
        feedBall();
        break;

      case CMD_LEFT:
        feederForward();
        delay(500);
        feederStop();
        Serial.println("CMD: LEFT");
        break;

      case CMD_RIGHT:
        feederBackward();
        delay(500);
        feederStop();
        Serial.println("CMD: RIGHT");
        break;

      case CMD_FIRE:
        Serial.println("CMD: Fire");
        pulseFan(10000);
        break;

      case CMD_LOAD:
        Serial.println("CMD: LOAD");
        loadSpring();
        break;

      default:
        Serial.print("CMD: Unknown (");
        Serial.print(cmd);
        Serial.println(")");
        break;
    } // <-- tämä sulkee switchin oikein

  } // <-- tämä sulkee onWrite-funktion

}; // <-- tämä sulkee classin


void setup() {
  delay(300);
  Serial.begin(115200);
  Serial.println("BLE starting...");

  // Alijärjestelmien alustus
  fanInit();
  motorInit();
  solenoidInit();
  sensorInit();

  BLEDevice::init("ESP32 Controller");
  delay(200);

  server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService *service = server->createService("00001234-0000-1000-8000-00805f9b34fb");

  commandChar = service->createCharacteristic(
    "0000abcd-0000-1000-8000-00805f9b34fb",
    BLECharacteristic::PROPERTY_WRITE
  );
  commandChar->setCallbacks(new CommandCallback());

  service->start();

  BLEAdvertising *adv = BLEDevice::getAdvertising();
  adv->setScanResponse(true);

  BLEDevice::startAdvertising();

  Serial.println("BLE ready. Waiting for device...");
}

void loop() {
  if (sensor_ballDetected()){
    Serial.println("SENSORI: Pallo havaittu!");
    delay(50);          // säätövara lyöntihetkeen
   
    Serial.println("2. LAUKAISU: Solenoidi vapauttaa jousen...");
    setSolenoid(true);  
    delay(150);
    setSolenoid(false); 
    Serial.println("   > Solenoidi palautettu lepotilaan.");

    delay(1000); // Tauko

    // 2. VALMISTELU VIRITYSTÄ VARTEN
    Serial.println("3. VIRITYS: Avataan lukitus viritystä varten...");
    setSolenoid(true);  
    delay(200);

    // 3. MAILAN VIRITYS
    Serial.println("4. MOOTTORI: Viritetään jousi");
    loadSpring();
    Serial.println("   > Viritys suoritettu.");

    // 4. LUKITUS
    Serial.println("5. LUKITUS: Suljetaan solenoidi lukitusasentoon.");
    setSolenoid(false); 
    delay(500);

    // 5. MOOTTORIN VAPAUTUS
    Serial.println("6. MOOTTORI: Palautetaan viritysmoottori");
    returnMotor(); // Vapautetaan mekaaninen vastus
    Serial.println("   > Moottori palautettu.");

    // 6. PALLON SYÖTTÖ
    delay(1000);
    Serial.println("7. SYÖTTÖ: Ladataan uusi pallo");
    feedBall();
    
    Serial.println("--- TESTISEKVENSSI VALMIS ---\n");
    Serial.println("Odotetaan uutta havaintoa");
    
    // Pitkä suoja-aika sekvenssin jälkeen, ettei laite "hätkyile"
    delay(5000); 
  }
  
  // Pieni viive, jotta CPU ei käy täysillä
  delay(100); 
}
