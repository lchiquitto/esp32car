// vim: ts=4 sw=4 et

#include <Ticker.h>
#include "DHTesp.h"
#include "WiFi.h"
#include "WiFiUdp.h"
#include "ESPCar.h"

#define DEBUG 1

inline void Debug(String line)
{
#if DEBUG
    Serial.println(line);
#endif
}

/* Estrutura para armazenar as leituras dos sensores */
struct sensorReadings {
    float temperature;
    float humidity;
    int sound_analog;
    int sound_digital;
    float distance;
} srs = {0.0, 0.0, 0, 0, 0};

void tempTask(void *pvParameters);
void soundTask(void *pvParameters);
void distanceTask(void *pvParameters);

bool getTemperature();
void triggerGetTemp();

bool getSound();
void triggerGetSound();

bool getDistance();
void triggerGetDistance();

TaskHandle_t tempTaskHandle = NULL;
TaskHandle_t soundTaskHandle = NULL;
TaskHandle_t distanceTaskHandle = NULL;

Ticker tempTicker;
Ticker soundTicker;
Ticker distanceTicker;

/* Flag que indica quando a inicialização foi concluída */
bool tasksEnabled = false;
/* Flag que indica que o WIFI esta conectado */
bool isConnected = false;

/* Configurações do WIFI */
const char *ssid = "espcar";
const char *password = "esp32car";

WiFiUDP udp;
DHTesp dht;

static int carState = S_STOPPED;

void stopMoving()
{
    analogWrite(PIN_PH_ENA, 0);
    analogWrite(PIN_PH_ENB, 0);
    digitalWrite(PIN_PH_IN1, LOW);
    digitalWrite(PIN_PH_IN2, LOW);
    digitalWrite(PIN_PH_IN3, LOW);
    digitalWrite(PIN_PH_IN4, LOW);
    carState = S_STOPPED;
}

void moveForward()
{
    digitalWrite(PIN_PH_IN1, HIGH);
    digitalWrite(PIN_PH_IN2, LOW);
    digitalWrite(PIN_PH_IN3, HIGH);
    digitalWrite(PIN_PH_IN4, LOW);

    if (carState == S_STOPPED) {
        analogWrite(PIN_PH_ENA, V_STARTUP);
        analogWrite(PIN_PH_ENB, V_STARTUP);
        delay(STARTUP_MS);
    }
    analogWrite(PIN_PH_ENA, V_MOVING);
    analogWrite(PIN_PH_ENB, V_MOVING);
    carState = S_MOVING;
}

void moveBack()
{
    digitalWrite(PIN_PH_IN1, LOW);
    digitalWrite(PIN_PH_IN2, HIGH);
    digitalWrite(PIN_PH_IN3, LOW);
    digitalWrite(PIN_PH_IN4, HIGH);

    if (carState == S_STOPPED) {
        analogWrite(PIN_PH_ENA, V_STARTUP);
        analogWrite(PIN_PH_ENB, V_STARTUP);
        delay(STARTUP_MS);
    }
    analogWrite(PIN_PH_ENA, V_MOVING);
    analogWrite(PIN_PH_ENB, V_MOVING);
    carState = S_MOVING;
}

void turnLeft()
{
    digitalWrite(PIN_PH_IN1, LOW);
    digitalWrite(PIN_PH_IN2, LOW);
    digitalWrite(PIN_PH_IN3, HIGH);
    digitalWrite(PIN_PH_IN4, LOW);

    if (carState == S_STOPPED) {
        analogWrite(PIN_PH_ENA, V_STARTUP);
        analogWrite(PIN_PH_ENB, V_STARTUP);
        delay(STARTUP_MS);
    }
    analogWrite(PIN_PH_ENA, V_MOVING);
    analogWrite(PIN_PH_ENB, V_MOVING);

    carState = S_MOVING;
}

void turnRight()
{
    digitalWrite(PIN_PH_IN1, HIGH);
    digitalWrite(PIN_PH_IN2, LOW);
    digitalWrite(PIN_PH_IN3, LOW);
    digitalWrite(PIN_PH_IN4, LOW);

    if (carState == S_STOPPED) {
        analogWrite(PIN_PH_ENA, V_STARTUP);
        analogWrite(PIN_PH_ENB, V_STARTUP);
        delay(STARTUP_MS);
    }
    analogWrite(PIN_PH_ENA, V_MOVING);
    analogWrite(PIN_PH_ENB, V_MOVING);
    carState = S_MOVING;
}

bool initSensors()
{
    pinMode(PIN_KY038_A, INPUT);
    pinMode(PIN_KY038_D, INPUT);

    pinMode(PIN_HC_TRIGGER, OUTPUT);
    pinMode(PIN_HC_ECHO, INPUT);

    dht.setup(PIN_DHT11, DHTesp::DHT11);

    xTaskCreate(tempTask, "tempTask", 4096, NULL, 6, &tempTaskHandle);
    xTaskCreate(soundTask, "soundTask", 4096, NULL, 5, &soundTaskHandle);
    xTaskCreate(distanceTask, "distanceTask", 4096, NULL, 4, &distanceTaskHandle);

    if (!tempTaskHandle || !soundTaskHandle || !distanceTaskHandle)
        return false;

    /* Le sensor de temperatura a cada 20 segundos */
    tempTicker.attach(20, triggerGetTemp);

    /* Le sensor de som/volume a cada 5 segundos */
    soundTicker.attach(5, triggerGetSound);

    /* Le sensor de distancia a cada segundo */
    distanceTicker.attach(1, triggerGetDistance);

    return true;
}

void initControls()
{
    pinMode(ONBOARD_LED, OUTPUT);

    pinMode(PIN_PH_ENA, OUTPUT);
    pinMode(PIN_PH_IN1, OUTPUT);
    pinMode(PIN_PH_IN2, OUTPUT);
    pinMode(PIN_PH_IN3, OUTPUT);
    pinMode(PIN_PH_IN4, OUTPUT);
    pinMode(PIN_PH_ENB, OUTPUT);
}

void connectToWiFi()
{
    Debug("Conectando a rede WIFI: " + String(ssid));

    WiFi.disconnect(true);
    WiFi.onEvent(WiFiEvent);
    WiFi.begin(ssid, password);

    Debug("Esperando pela conexao WIFI...");
}

int udpPort = 3333;

void WiFiEvent(WiFiEvent_t event)
{
    switch (event) {
      case ARDUINO_EVENT_WIFI_STA_GOT_IP:
          Debug("WiFi conectado! Endereco IP: " + WiFi.localIP());
          udp.begin(WiFi.localIP(), udpPort);
          isConnected = true;
          break;
      case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
          Debug("Conexao WIFI perdida!");
          isConnected = false;
          break;
      default:
          break;
    }
}

void triggerGetTemp()
{
    if (tempTaskHandle != NULL) {
        xTaskResumeFromISR(tempTaskHandle);
    }
}

void triggerGetSound()
{
    if (soundTaskHandle != NULL) {
        xTaskResumeFromISR(soundTaskHandle);
    }
}

void triggerGetDistance()
{
    if (distanceTaskHandle != NULL) {
        xTaskResumeFromISR(distanceTaskHandle);
    }
}

void tempTask(void *pvParameters)
{
    Debug("Iniciado loop para sensor de temperatura");
    while (1) {
        if (tasksEnabled) {
            getTemperature();
        }
        vTaskSuspend(NULL);
    }
}

void soundTask(void *pvParameters)
{
    Debug("Iniciado loop para sensor de volume/som");

    while (1) {
        if (tasksEnabled) {
            getSound();
        }
        vTaskSuspend(NULL);
    }
}

void distanceTask(void *pvParameters)
{
    Debug("Iniciado loop para sensor de distancia");

    while (1) {
        if (tasksEnabled) {
            getDistance();
        }
        vTaskSuspend(NULL);
    }
}

bool getTemperature()
{
    /* A leitura da temperatura e umidade pode levar até 250ms */
    TempAndHumidity v = dht.getTempAndHumidity();
    if (dht.getStatus() != 0) {
        Debug("DHT11 error status: " + String(dht.getStatusString()));
        return false;
    }
    Debug("T:" + String(v.temperature) + " H:" + String(v.humidity));

    srs.temperature = v.temperature;
    srs.humidity = v.humidity;

    return true;
}

bool getSound()
{
    int v_digital = digitalRead(PIN_KY038_D);
    int v_analog = analogRead(PIN_KY038_A);

    Debug("SA: " + String(v_analog) + " SD: " + String(v_digital));

    srs.sound_analog = v_analog;
    srs.sound_digital = v_digital;

    return true;
}

bool getDistance()
{
    float distance_cm;
    long duration;

    digitalWrite(PIN_HC_TRIGGER, LOW);
    delayMicroseconds(2);

    digitalWrite(PIN_HC_TRIGGER, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_HC_TRIGGER, LOW);

    duration = pulseIn(PIN_HC_ECHO, HIGH);
    distance_cm = duration * SOUND_SPEED/2;

    Debug("D: " + String(distance_cm));

    srs.distance = distance_cm;

    return true;
}

void sendReadings()
{
    char mask[] = "{\"T\": %.2f, \"H\": %.2f, \"SA\": %d, \"SD\": %d, \"D\": %.2f}";

    udp.beginPacket(udp.remoteIP(), udpPort);
    udp.printf(mask, srs.temperature,
                     srs.humidity,
                     srs.sound_analog,
                     srs.sound_digital,
                     srs.distance);
    udp.endPacket();
}

void processCommand(char cmd)
{
    static bool led = true;

    digitalWrite(ONBOARD_LED, led);
    led = !led;

    switch (cmd) {
        case CMD_FORWARD:
            moveForward();
            break;
        case CMD_BACK:
            moveBack();
            break;
        case CMD_LEFT:
            turnLeft();
            break;
        case CMD_RIGHT:
            turnRight();
            break;
        case CMD_STOP:
            stopMoving();
            break;
        case CMD_READ:
            sendReadings();
            break;
        default:
            return;
    }
}

void setup()
{
#if DEBUG
    Serial.begin(115200);
#endif
    Debug("ESP32 carro com sensores e controle via Wifi");

    if (!initSensors()) {
        Debug("Erro inicializando tarefas dos sensores");
        return;
    }
    initControls();
    connectToWiFi();

    tasksEnabled = true;
}

char packetBuffer[255];

void loop()
{
    if (!tasksEnabled) {
        delay(2000);
        tasksEnabled = true;
        if (tempTaskHandle != NULL) {
            vTaskResume(tempTaskHandle);
        }
        if (soundTaskHandle != NULL) {
            vTaskResume(soundTaskHandle);
        }
        if (distanceTaskHandle != NULL) {
            vTaskResume(distanceTaskHandle);
        }
    }

    /* Loop para receber comandos via UDP */
    int packetSize = udp.parsePacket();

    if (packetSize) {
        int len = udp.read(packetBuffer, 255);
        if (len == 1) {
            Debug("Comando recebido: " + String(packetBuffer[0]));
            processCommand(packetBuffer[0]);
        } else {
            Debug("Pacote com tamanho invalido descartado");
        }
    }

    /* Necessario para que as outras tarefas possam rodar */
    yield();
}
