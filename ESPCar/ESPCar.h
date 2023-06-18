// vim: ts=4 sw=4 et

/* Pinos utilizados (ESP32 30) */

#define ONBOARD_LED 2

/* Pinos de dados conectados aos sensores DHT11, KY-038 e HC-SR04 */
#define PIN_DHT11       4
#define PIN_KY038_A     35
#define PIN_KY038_D     34
#define PIN_HC_ECHO     16
#define PIN_HC_TRIGGER  17

/* Pinos ligados a ponte-H para controle dos motores */
#define PIN_PH_ENA      25
#define PIN_PH_IN1      26
#define PIN_PH_IN2      27
#define PIN_PH_IN3      14
#define PIN_PH_IN4      12
#define PIN_PH_ENB      13

/* Tensões PWM para controle de velocidade dos motores */
#define V_MOVING    100
#define V_STARTUP   150

/* Estado do carro (em movimento ou parado) */
#define S_STOPPED   0
#define S_MOVING    1

/* Para cálculo de distância do HC-SR04 */
#define SOUND_SPEED 0.034

/* Código dos comandos para controlar o carro */
#define CMD_FORWARD 'F'
#define CMD_BACK    'B'
#define CMD_LEFT    'L'
#define CMD_RIGHT   'R'
#define CMD_STOP    'S'
#define CMD_READ    'X'
