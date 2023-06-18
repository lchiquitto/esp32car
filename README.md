# esp32car

## Componentes

Este projeto implementa um "carrinho" de controle remoto simples através dos seguintes componentes:

- Chassi em acrílico com 3 rodas (duas ligadas a motores e uma solta)
- Microcontrolador ESP32 (DEVKIT V1, de 30 pinos)
- Motores elétricos (3-6V)
- Ponte H (L298N)
- Duas baterias de 4.2V (modelo 18650)

O veículo carrega os seguintes sensores de medição:

- Sensor ultrasônico (HC-SR04)
- Sensor de temperatura e umidade (DHT11)
- Sensor de som com microfone (KY-038)

As conexões entre microcontrolador e sensores são feitas com um mini protoboard.

## Funcionamento

### Alimentação

As baterias, ligadas em série, fornecem aproximadamente 8.4V de tensão. Seus polos estão conectados às entradas "12 V" e "GND" da ponte H. O regulador de tensão da ponte H está habilitado. A conexão "5 V" (Vlogic) da ponte H fornece alimentação para o microcontrolador ESP32 através de seu pino VIN. Os sensores, que consomem pouca corrente, são alimentados pelo pino "3V3" do microcontrolador ESP32.

### Comunicação

O microcontrolador é programado para conectar-se a uma rede WIFI pré-definida. O software executado no microcontrolador recebe comandos e envia dados das leituras dos sensores via rede (IP/UDP). Os comandos aceitos são: F (move o carro para frente), B (move o carro para trás), L (vira à esquerda), R (vira à direita), S (para o carro), X (solicita o envio dos dados dos sensores).

Os pacotes UDP que controlam a movimentação do carro possuem um único byte no seu campo de dados (as letras acima). O pacote enviado pelo carro para o controlador, contendo os dados de leituras dos sensores, possui o seguinte formato:

```
{"T": 24.00, "H": 48.00, "SA": 2067, "SD": 0, "D": 77.42}
```

Os valores são, respectivamente, temperatura (celsius), umidade (percentual), som/volume analógico (sem unidade), som/volume digital (sem unidade) e distância (centímetros).
