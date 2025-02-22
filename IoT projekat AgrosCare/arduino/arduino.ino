#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

// WiFi podaci
const char* ssid = "";
const char* password = "";

// Firebase kredencijali
#define FIREBASE_HOST "agroscareiot-default-rtdb.europe-west1.firebasedatabase.app"
#define FIREBASE_AUTH "cNOWM2kRlaDuUsfDtzeDadMn8rjN609VJrbmHSG4"

// Pin definicije
#define SOIL_MOISTURE_PIN A0
#define RELAY_PIN D3
#define LED_PIN D4  // Dodatni pin za svjetlo

FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;

void setup() {
  Serial.begin(115200);
  Serial.println("Inicijalizacija...");

  // Povezivanje na WiFi
  WiFi.begin(ssid, password);
  Serial.print("Povezivanje na WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi povezan!");

  // Firebase konfiguracija
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Provjera veze s Firebase-om
  if (!Firebase.ready()) {
    Serial.println("Greška u vezi s Firebase-om: " + firebaseData.errorReason());
    return;
  }
  Serial.println("Veza sa Firebase-om uspješna!");

  // Postavljanje pinova
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);  // LED pin kao izlaz
  digitalWrite(RELAY_PIN, HIGH); // Po defaultu pumpa isključena
  digitalWrite(LED_PIN, LOW);    // Po defaultu svjetlo ugašeno
}

void loop() {
  int moistureValue = analogRead(SOIL_MOISTURE_PIN);  // Čitanje vlažnosti tla
  Serial.print("Vlažnost tla: ");
  Serial.println(moistureValue);

  // Automatsko upravljanje pumpom na osnovu vlažnosti tla
  String moistureStatus;
  if (moistureValue > 700) {
    moistureStatus = "Suho";   // Suho tlo
  } else {
    moistureStatus = "Mokro";  // Mokro tlo
  }

  Serial.println("Status tla: " + moistureStatus);

  // Upravljanje pumpom i svjetlom
  if (moistureStatus == "Suho") {
    if (digitalRead(RELAY_PIN) == HIGH) { // Ako pumpa nije uključena
      digitalWrite(RELAY_PIN, LOW);  // Uključi pumpu
      digitalWrite(LED_PIN, HIGH);   // Upali svjetlo
      Firebase.setString(firebaseData, "/pump", "ON");
      Serial.println("Pumpa ukljucena i svjetlo upaljeno!");
    }
  } else {
    if (digitalRead(RELAY_PIN) == LOW) { // Ako pumpa nije isključena
      digitalWrite(RELAY_PIN, HIGH); // Isključi pumpu
      digitalWrite(LED_PIN, LOW);    // Ugasi svjetlo
      Firebase.setString(firebaseData, "/pump", "OFF");
      Serial.println("Pumpa iskljucena i svjetlo ugaseno!");
    }
  }

  // Slanje podataka u Firebase
  if (Firebase.setString(firebaseData, "/SoilMoisture", moistureStatus)) {
    Serial.println("Status vlažnosti poslat: " + moistureStatus);
  } else {
    Serial.println("Greška pri slanju statusa vlažnosti: " + firebaseData.errorReason());
  }

  if (Firebase.setInt(firebaseData, "/SoilMoistureValue", moistureValue)) {
    Serial.println("Podaci vlažnosti poslani");
  } else {
    Serial.println("Greška pri slanju podataka vlažnosti: " + firebaseData.errorReason());
  }

  delay(5000); // Pauza od 5 sekundi
}
