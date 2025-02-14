#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// 📡 Configuración de la red Wi-Fi
const char* ssid = "SISE"; 
const char* password = "Fulvineitor23";

// 🌍 URLs de la API
const char* serverActuador = "http://192.168.0.114/api_iot/get_actuador.php";  
const char* serverSensor = "http://192.168.0.114/api_iot/update_sensor.php";
const char* serverUpdateActuador = "http://192.168.0.114/api_iot/update_actuador.php";  

// ⚡ Pines de hardware
#define SensorPin       34  // Sensor de humedad
#define DHTPin          14  // DHT11
#define RelayPin        23  // Bomba de agua
#define wifiLed         2   // LED indicador de WiFi
#define RelayButtonPin  32  // Botón para activar la bomba manualmente
#define ModeSwitchPin   33  // Botón para cambiar modo
#define BuzzerPin       26  // Buzzer
#define ModeLed         15  // LED indicador de modo

// Configuración del sensor DHT11
#define DHTTYPE DHT11
DHT dht(DHTPin, DHTTYPE);

// Variables de estado
bool modoAutomatico = true;
bool bombaEncendida = false;
bool buzzerEncendido = false;
int humedadSuelo = 0;
int temperatura = 0;
int humedadAire = 0;
const int humedadUmbral = 30; // 🌱 Umbral de humedad para activar la bomba automáticamente

void setup() {
    Serial.begin(115200);

    // 🔹 Configurar pines
    pinMode(RelayPin, OUTPUT);
    pinMode(wifiLed, OUTPUT);
    pinMode(ModeLed, OUTPUT);
    pinMode(BuzzerPin, OUTPUT);
    pinMode(RelayButtonPin, INPUT_PULLUP);
    pinMode(ModeSwitchPin, INPUT_PULLUP);

    digitalWrite(RelayPin, LOW);
    digitalWrite(wifiLed, LOW);
    digitalWrite(ModeLed, LOW);
    digitalWrite(BuzzerPin, LOW);

    // 🔹 Iniciar el sensor DHT11
    dht.begin();

    // 🔹 Conectar a la red WiFi
    WiFi.begin(ssid, password);
    Serial.print("Conectando a WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\n✅ Conectado a WiFi");
    digitalWrite(wifiLed, HIGH);
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        // 🔹 Obtener estado del actuador "modo"
        String modoEstado = obtenerEstadoActuador("modo");
        bool nuevoModoAutomatico = (modoEstado == "ON");

        // 🔹 Si el estado ha cambiado, activar el buzzer (como en Blynk)
        if (nuevoModoAutomatico != modoAutomatico) {
            modoAutomatico = nuevoModoAutomatico;
            Serial.println("🔄 Cambio de modo detectado: " + modoEstado);
            activarBuzzer();
        }

        digitalWrite(ModeLed, modoAutomatico ? HIGH : LOW);

        // 🔹 Obtener estado de la bomba
        bombaEncendida = obtenerEstadoActuador("bomba") == "ON";

        // 🔹 Obtener estado del buzzer
        buzzerEncendido = obtenerEstadoActuador("buzzer") == "ON";
        digitalWrite(BuzzerPin, buzzerEncendido ? HIGH : LOW);

        // 🔹 Leer datos del sensor DHT11
        temperatura = (int) dht.readTemperature();
        humedadAire = (int) dht.readHumidity();

        // 🔹 Leer sensor de humedad del suelo
        humedadSuelo = map(analogRead(SensorPin), 3000, 930, 0, 100);
        
        Serial.print("🌡 Temp: "); Serial.print(temperatura); Serial.print("°C  ");
        Serial.print("💧 Humedad: "); Serial.print(humedadAire); Serial.print("%  ");
        Serial.print("🌱 Suelo: "); Serial.print(humedadSuelo); Serial.println("%");

        // 🔹 Enviar datos a la API
        enviarDatosSensor("temperatura", temperatura);
        enviarDatosSensor("humedad", humedadAire);
        enviarDatosSensor("suelo", humedadSuelo);

        // 🔹 Control automático de la bomba de agua
        if (modoAutomatico) {
            if (humedadSuelo < humedadUmbral && !bombaEncendida) {
                activarActuador("bomba", "ON");
                bombaEncendida = true;
            } else if (humedadSuelo >= humedadUmbral && bombaEncendida) {
                activarActuador("bomba", "OFF");
                bombaEncendida = false;
            }
        }

        // 🔹 Verificar botones físicos
        if (digitalRead(ModeSwitchPin) == LOW) {
            cambiarModo();
            delay(500);
        }
        if (digitalRead(RelayButtonPin) == LOW && !modoAutomatico) {
            bombaEncendida = !bombaEncendida;
            activarActuador("bomba", bombaEncendida ? "ON" : "OFF");
            delay(500);
        }
    } else {
        Serial.println("⚠ WiFi desconectado, intentando reconectar...");
        WiFi.begin(ssid, password);
    }

    delay(500); // Esperar 3 segundos
}

// 🔹 Función para obtener estado de un actuador desde la API
String obtenerEstadoActuador(const char* nombre) {
    HTTPClient http;
    http.begin(String(serverActuador) + "?nombre=" + nombre);
    int httpResponseCode = http.GET();
    String estado = "OFF";

    if (httpResponseCode > 0) {
        String payload = http.getString();
        Serial.println("📥 Respuesta: " + payload);

        StaticJsonDocument<200> jsonDocument;
        DeserializationError error = deserializeJson(jsonDocument, payload);
        if (!error) {
            estado = jsonDocument["estado"].as<String>();
        }
    }
    http.end();
    return estado;
}

// 🔹 Función para enviar datos de sensores a la API
void enviarDatosSensor(const char* sensor, int valor) {
    HTTPClient http;
    http.begin(serverSensor);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> jsonDoc;
    jsonDoc["sensor"] = sensor;
    jsonDoc["valor"] = valor;

    String jsonData;
    serializeJson(jsonDoc, jsonData);
    int httpPostResponse = http.POST(jsonData);
    
    Serial.print("📤 Enviado: "); Serial.println(jsonData);
    if (httpPostResponse > 0) {
        Serial.println("📥 Respuesta API: " + http.getString());
    }
    http.end();
}

// 🔹 Función para activar/desactivar actuador desde la API
void activarActuador(const char* nombre, const char* estado) {
    HTTPClient http;
    http.begin(serverUpdateActuador);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "nombre=" + String(nombre) + "&estado=" + String(estado);
    int httpPostResponse = http.POST(postData);
    
    Serial.print("📤 Cambiando estado de "); Serial.print(nombre); Serial.print(" a "); Serial.println(estado);
    if (httpPostResponse > 0) {
        Serial.println("📥 Respuesta API: " + http.getString());
    }
    http.end();
}

// 🔹 Función para cambiar el modo y activar el buzzer
void cambiarModo() {
    modoAutomatico = !modoAutomatico;
    activarActuador("modo", modoAutomatico ? "ON" : "OFF");
    activarBuzzer();
}

// 🔹 Función para activar el buzzer (como en Blynk)
void activarBuzzer() {
    digitalWrite(BuzzerPin, HIGH);
    delay(500);
    digitalWrite(BuzzerPin, LOW);
}
