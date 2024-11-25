#include "MAX30100_PulseOximeter.h"
#include <WiFi.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <UrlEncode.h>

#define REPORTING_PERIOD_MS     1000
#define SCL_PIN 22
#define SDA_PIN 21

const char* ssid = "SATC IOT";
const char* password = "IOT2024@#";
String phoneNumber = "+554896618376";
String apiKey = "3019484";

PulseOximeter pox;

uint32_t tsLastReport = 0;

void onBeatDetected()
{
    Serial.println("Batida detectada!");
}

void sendMessage(String message) {
    Serial.println("Iniciando envio de mensagem...");

    String encodedMessage = urlEncode(message);
    String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + encodedMessage;

    HTTPClient http;
    http.setTimeout(5000);  // Timeout de 5 segundos

    http.begin(url);

    int httpResponseCode = http.POST(url);
    if (httpResponseCode == 200) {
        Serial.println("Mensagem enviada com sucesso");
    } else {
        Serial.print("Erro no envio da mensagem. Código HTTP: ");
        Serial.println(httpResponseCode);
    }

    http.end();
    delay(1000);  // Delay para evitar chamadas rápidas demais
}

void setup() {
    Serial.begin(115200);
    Serial.println("Inicializando o oxímetro de pulso...");

    if (!pox.begin()) {
        Serial.println("Falha na inicialização");
        for(;;);
    } else {
        Serial.println("Inicialização bem-sucedida");
    }

    pox.setOnBeatDetectedCallback(onBeatDetected);
    Wire.begin(SDA_PIN, SCL_PIN);

    WiFi.begin(ssid, password);
    Serial.println("Conectando ao Wi-Fi...");
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) {
        delay(500);
        Serial.print(".");
        retries++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConectado ao Wi-Fi!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFalha na conexão Wi-Fi!");
        return;  // Se não conectar, não faz o envio
    }
}

void loop() {
    pox.update();

    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        float heartRate = pox.getHeartRate();
        float spo2 = pox.getSpO2();

        if (heartRate > 0 && spo2 > 0) {
            String message = "Frequência cardíaca: " + String(heartRate) + " bpm\nSpO2: " + String(spo2) + "%";
            sendMessage(message);
        } else {
            Serial.println("Leitura inválida!");
        }

        tsLastReport = millis();
    }
}