#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>

// Configuration WiFi
const char* ssid = "*******";
const char* password = "*****";

// Configuration MQTT
const char* mqtt_server = "broker.hivemq.com";  // ou votre broker MQTT
const int mqtt_port = 1883;
const char* mqtt_client_id = "SolarGuard_ESP32";
const char* mqtt_topic = "solarguard/sensors";

WiFiClient espClient;
PubSubClient client(espClient);

// Configuration DHT11
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Configuration Servo
#define SERVO_PIN 18
Servo monServo;

// Configuration capteurs électriques
#define ACS712_PIN 34        // Pin analogique pour ACS712 (current sensor)
#define VOLTAGE_SENSOR_PIN 35 // Pin analogique pour capteur de tension

// Configuration LDR (Light Dependent Resistors)
#define LDR1_PIN 36          // Pin analogique pour LDR supérieur (top)
#define LDR2_PIN 39          // Pin analogique pour LDR inférieur (bottom)

// Constantes de calibration
const float ACS712_SENSITIVITY = 0.185;  // 185mV/A pour ACS712-05A
const float ACS712_OFFSET = 2.5;         // Offset à 0A (2.5V pour alimentation 5V)
const float VOLTAGE_DIVIDER_RATIO = 11.0; // Ratio du diviseur de tension (R1+R2)/R2
const float ADC_REFERENCE = 3.3;         // Tension de référence ADC ESP32
const int ADC_RESOLUTION = 4095;         // Résolution ADC 12 bits

// Paramètres de tracking automatique basé sur LDR
const int LDR_THRESHOLD = 50;            // Seuil de différence pour ajustement
const int SERVO_STEP = 3;                // Pas d'ajustement en degrés (réduit pour plus de précision)
const int SERVO_MIN_ANGLE = 0;           // Angle minimum du servo
const int SERVO_MAX_ANGLE = 180;         // Angle maximum du servo

// Variables pour le timing
unsigned long derniereLectureDHT = 0;
unsigned long dernierEnvoiMQTT = 0;
unsigned long dernierTrackingLDR = 0;
const long intervalDHT = 2000;
const long intervalMQTT = 3000;          // Envoi MQTT toutes les 3 secondes
const long intervalTracking = 500;       // Vérification LDR toutes les 500ms pour réactivité

int angleServo = 90;  // Position initiale centrale
bool modeAutoTracking = true;  // Mode de suivi automatique activé par défaut

// Variables pour moyennage des mesures
const int NB_SAMPLES = 10;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== Démarrage ESP32 SolarGuard ===");
  Serial.println("=== Tracking Solaire Automatique par LDR ===\n");
  
  // Configuration des pins analogiques
  pinMode(ACS712_PIN, INPUT);
  pinMode(VOLTAGE_SENSOR_PIN, INPUT);
  pinMode(LDR1_PIN, INPUT);
  pinMode(LDR2_PIN, INPUT);
  
  // Initialisation DHT11
  dht.begin();
  Serial.println("✓ DHT11 initialisé");
  
  // Initialisation Servo en position centrale
  monServo.attach(SERVO_PIN);
  monServo.write(angleServo);
  Serial.print("✓ Servo initialisé à ");
  Serial.print(angleServo);
  Serial.println("° (position centrale)");
  
  // Initialisation LDR
  Serial.println("✓ LDR1 (Top) configuré sur GPIO36");
  Serial.println("✓ LDR2 (Bottom) configuré sur GPIO39");
  
  // Connexion WiFi
  connecterWiFi();
  
  // Configuration MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callbackMQTT);
  
  Serial.println("\n=== Système prêt ===");
  Serial.println("Mode: Tracking automatique par LDR ACTIVÉ");
  Serial.println("Le servo suit automatiquement la position du soleil\n");
}

void loop() {
  // Maintenir la connexion MQTT
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();
  
  unsigned long maintenant = millis();
  
  // Lecture DHT11 toutes les 2 secondes
  if (maintenant - derniereLectureDHT >= intervalDHT) {
    derniereLectureDHT = maintenant;
    lireDHT11();
  }
  
  // Tracking solaire automatique basé sur LDR
  if (modeAutoTracking && (maintenant - dernierTrackingLDR >= intervalTracking)) {
    dernierTrackingLDR = maintenant;
    trackingSolaireLDR();
  }
  
  // Envoi des données MQTT toutes les 3 secondes
  if (maintenant - dernierEnvoiMQTT >= intervalMQTT) {
    dernierEnvoiMQTT = maintenant;
    envoyerDonneesMQTT();
  }
}

void connecterWiFi() {
  Serial.println("\n--- Connexion WiFi ---");
  Serial.print("SSID: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int tentatives = 0;
  while (WiFi.status() != WL_CONNECTED && tentatives < 20) {
    delay(500);
    Serial.print(".");
    tentatives++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ WiFi connecté!");
    Serial.print("Adresse IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Force du signal: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("\n✗ Échec de connexion WiFi");
  }
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Connexion au broker MQTT...");
    
    if (client.connect(mqtt_client_id)) {
      Serial.println(" ✓ Connecté!");
      client.subscribe("solarguard/commands");
    } else {
      Serial.print(" ✗ Échec, rc=");
      Serial.print(client.state());
      Serial.println(" Nouvelle tentative dans 5 secondes");
      delay(5000);
    }
  }
}

void callbackMQTT(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message reçu sur le topic: ");
  Serial.println(topic);
  
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Contenu: ");
  Serial.println(message);
  
  // Traiter les commandes JSON du cloud
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);
  
  if (!error) {
    // Commande: activer/désactiver auto-tracking
    if (doc.containsKey("auto_tracking")) {
      modeAutoTracking = doc["auto_tracking"];
      Serial.print("Mode auto-tracking LDR: ");
      Serial.println(modeAutoTracking ? "ACTIVÉ" : "DÉSACTIVÉ");
    }
    
    // Commande: ajuster l'angle manuellement (seulement si auto-tracking désactivé)
    if (doc.containsKey("servo_angle") && !modeAutoTracking) {
      int nouvelAngle = doc["servo_angle"];
      if (nouvelAngle >= SERVO_MIN_ANGLE && nouvelAngle <= SERVO_MAX_ANGLE) {
        angleServo = nouvelAngle;
        monServo.write(angleServo);
        Serial.print("Servo ajusté manuellement à: ");
        Serial.print(angleServo);
        Serial.println("°");
      }
    }
    
    // Commande: ajuster le seuil LDR
    if (doc.containsKey("ldr_threshold")) {
      // Mise à jour du seuil si nécessaire
      Serial.print("Nouveau seuil LDR: ");
      Serial.println(doc["ldr_threshold"].as<int>());
    }
  }
}

void lireDHT11() {
  float humidite = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  if (!isnan(humidite) && !isnan(temperature)) {
    Serial.println("--- DHT11 ---");
    Serial.print("Température: ");
    Serial.print(temperature, 1);
    Serial.print(" °C | Humidité: ");
    Serial.print(humidite, 1);
    Serial.println(" %");
  }
}

int lireLDR(int pin) {
  // Lecture multiple pour moyennage et réduction du bruit
  long somme = 0;
  for (int i = 0; i < NB_SAMPLES; i++) {
    somme += analogRead(pin);
    delay(5);
  }
  return somme / NB_SAMPLES;
}

void trackingSolaireLDR() {
  // Lecture des deux capteurs LDR
  int ldr1Value = lireLDR(LDR1_PIN);
  int ldr2Value = lireLDR(LDR2_PIN);
  
  // Calcul de la différence (positive si LDR1 > LDR2)
  int difference = ldr1Value - ldr2Value;
  
  // Affichage périodique (toutes les 10 lectures pour ne pas surcharger le serial)
  static int compteur = 0;
  compteur++;
  
  if (compteur >= 10) {
    Serial.println("--- Tracking Solaire LDR ---");
    Serial.print("LDR1 (Top): ");
    Serial.print(ldr1Value);
    Serial.print(" | LDR2 (Bottom): ");
    Serial.print(ldr2Value);
    Serial.print(" | Diff: ");
    Serial.print(difference);
    Serial.print(" | Angle actuel: ");
    Serial.print(angleServo);
    Serial.println("°");
    compteur = 0;
  }
  
  // Logique de tracking: ajuster le servo selon la différence LDR
  if (abs(difference) > LDR_THRESHOLD) {
    int nouvelAngle = angleServo;
    
    if (difference > 0) {
      // LDR1 (Top) reçoit plus de lumière que LDR2 (Bottom)
      // Le soleil est plus haut → incliner le panneau vers le haut
      nouvelAngle += SERVO_STEP;
      Serial.print("→ Soleil détecté en haut: Inclinaison vers le haut (+");
      Serial.print(SERVO_STEP);
      Serial.println("°)");
    } else {
      // LDR2 (Bottom) reçoit plus de lumière que LDR1 (Top)
      // Le soleil est plus bas → incliner le panneau vers le bas
      nouvelAngle -= SERVO_STEP;
      Serial.print("→ Soleil détecté en bas: Inclinaison vers le bas (-");
      Serial.print(SERVO_STEP);
      Serial.println("°)");
    }
    
    // Limiter l'angle dans la plage autorisée
    nouvelAngle = constrain(nouvelAngle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
    
    // Appliquer le nouvel angle si changement nécessaire
    if (nouvelAngle != angleServo) {
      angleServo = nouvelAngle;
      monServo.write(angleServo);
      Serial.print("✓ Servo ajusté à: ");
      Serial.print(angleServo);
      Serial.println("°");
    } else {
      Serial.println("⚠ Limite d'angle atteinte");
    }
  } else {
    // Différence faible = position optimale
    if (compteur == 0) {
      Serial.println("✓ Position optimale maintenue");
    }
  }
}

float lireCourant() {
  float sommeCourant = 0;
  
  for (int i = 0; i < NB_SAMPLES; i++) {
    int valeurADC = analogRead(ACS712_PIN);
    float tension = (valeurADC * ADC_REFERENCE) / ADC_RESOLUTION;
    float courant = (tension - ACS712_OFFSET) / ACS712_SENSITIVITY;
    sommeCourant += courant;
    delay(10);
  }
  
  float courantMoyen = sommeCourant / NB_SAMPLES;
  
  if (abs(courantMoyen) < 0.1) {
    courantMoyen = 0;
  }
  
  return abs(courantMoyen);
}

float lireTension() {
  float sommeTension = 0;
  
  for (int i = 0; i < NB_SAMPLES; i++) {
    int valeurADC = analogRead(VOLTAGE_SENSOR_PIN);
    float tensionADC = (valeurADC * ADC_REFERENCE) / ADC_RESOLUTION;
    float tensionReelle = tensionADC * VOLTAGE_DIVIDER_RATIO;
    sommeTension += tensionReelle;
    delay(10);
  }
  
  return sommeTension / NB_SAMPLES;
}

void envoyerDonneesMQTT() {
  Serial.println("\n--- Envoi MQTT ---");
  
  // Lecture de tous les capteurs
  float temperature = dht.readTemperature();
  float humidite = dht.readHumidity();
  float courant = lireCourant();
  float tension = lireTension();
  float puissance = tension * courant;
  
  // Lecture des LDR
  int ldr1Value = lireLDR(LDR1_PIN);
  int ldr2Value = lireLDR(LDR2_PIN);
  int ldrDifference = ldr1Value - ldr2Value;
  
  // Calcul de l'efficacité de tracking (plus la différence est faible, meilleur est le tracking)
  float trackingAccuracy = 100.0 - (abs(ldrDifference) / 40.95); // Normalisation sur 4095 max
  if (trackingAccuracy < 0) trackingAccuracy = 0;
  
  // Affichage résumé
  Serial.print("Puissance: ");
  Serial.print(puissance, 2);
  Serial.print(" W | LDR Diff: ");
  Serial.print(ldrDifference);
  Serial.print(" | Précision tracking: ");
  Serial.print(trackingAccuracy, 1);
  Serial.println(" %");
  
  // Création du JSON complet
  StaticJsonDocument<512> doc;
  doc["device_id"] = mqtt_client_id;
  doc["timestamp"] = millis();
  
  // Données environnementales
  doc["temperature"] = round(temperature * 100) / 100.0;
  doc["humidity"] = round(humidite * 100) / 100.0;
  
  // Données électriques
  doc["current"] = round(courant * 1000) / 1000.0;
  doc["voltage"] = round(tension * 100) / 100.0;
  doc["power"] = round(puissance * 100) / 100.0;
  
  // Données LDR et tracking
  doc["ldr1"] = ldr1Value;
  doc["ldr2"] = ldr2Value;
  doc["ldr_diff"] = ldrDifference;
  doc["tracking_accuracy"] = round(trackingAccuracy * 10) / 10.0;
  
  // État du système
  doc["servo_angle"] = angleServo;
  doc["auto_tracking"] = modeAutoTracking;
  
  // Conversion et envoi
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);
  
  if (client.publish(mqtt_topic, jsonBuffer)) {
    Serial.println("✓ Données envoyées au cloud");
  } else {
    Serial.println("✗ Échec envoi MQTT");
  }
  Serial.println();
}