#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> // Librería para manejar JSON

// Configuración de la red Wi-Fi
const char* ssid = "Matias0421"; 
const char* password = "Esteban1223";

// URLs de la API
const char* serverActuador = "http://192.168.18.170/api_iot/get_actuador.php";  // Endpoint para obtener el estado del actuador
const char* serverSensor = "http://192.168.18.170/api_iot/update_sensor.php";  // Endpoint para enviar la temperatura

// Pines de hardware
const int ledPin = 27;  // LED en el pin D27
const int sensorPin = 34; // Sensor de temperatura en el pin A34 (ajústalo según tu hardware)

void setup() {
    Serial.begin(115200);
    
    // Configurar pines
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW); 

    // Conectar a la red WiFi
    WiFi.begin(ssid, password);
    Serial.print("Conectando a WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConectado a WiFi");
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        // 1. Obtener el estado del actuador (bomba)
        HTTPClient http;
        http.begin(serverActuador);
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0) {
            String payload = http.getString();
            Serial.println("Respuesta del servidor: " + payload);

            StaticJsonDocument<200> jsonDocument;
            DeserializationError error = deserializeJson(jsonDocument, payload);

            if (!error) {
                const char* estado = jsonDocument["estado"]; 
                Serial.print("Estado del actuador: ");
                Serial.println(estado);

                if (strcmp(estado, "OFF") == 0) {
                    digitalWrite(ledPin, HIGH);
                    Serial.println("LED encendido");
                } else if (strcmp(estado, "ON") == 0) {
                    digitalWrite(ledPin, LOW);
                    Serial.println("LED apagado");
                }
            } else {
                Serial.println("Error al parsear JSON");
            }
        } else {
            Serial.print("Error en la solicitud: ");
            Serial.println(httpResponseCode);
        }
        http.end();

        // 2. Leer sensor y enviar datos a la API
        int sensorValue = analogRead(sensorPin);
        float temperatura = sensorValue * (3.3 / 4095.0) * 100;  // Ajusta la fórmula según tu sensor

        Serial.print("Temperatura medida: ");
        Serial.println(temperatura);

        HTTPClient http2;
        http2.begin(serverSensor);
        http2.addHeader("Content-Type", "application/json"); 

        // Crear JSON con la temperatura
        StaticJsonDocument<200> jsonDoc;
        jsonDoc["sensor"] = "temperatura";
        jsonDoc["valor"] = temperatura;
        
        String jsonData;
        serializeJson(jsonDoc, jsonData);

        Serial.print("JSON enviado: ");
        Serial.println(jsonData); // 👀 Esto nos ayuda a ver qué está enviando realmente el ESP32

        int httpPostResponse = http2.POST(jsonData);

        if (httpPostResponse > 0) {
            Serial.print("Respuesta del servidor: ");
            Serial.println(http2.getString());
        } else {
            Serial.print("Error al enviar temperatura: ");
            Serial.println(httpPostResponse);
        }

        http2.end();
    } else {
        Serial.println("WiFi desconectado, intentando reconectar...");
        WiFi.begin(ssid, password);
    }

    delay(1000); // Ejecutar cada 5 segundos
}
