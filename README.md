## Carro Microcontrolado com Wifi

### Componentes

Este projeto implementa um "carrinho" de controle remoto simples através dos seguintes componentes:

- Chassi em acrílico com 3 rodas (duas ligadas a motores e uma solta)
- Microcontrolador ESP32 (DEVKIT V1, de 30 pinos)
- Motores elétricos (3-6V)
- Ponte H (L298N)
- Duas baterias de 4.2V (modelo 18650)

<img src="https://raw.githubusercontent.com/lchiquitto/esp32car/main/doc/chassi.jpg" width="200"> <img src="https://raw.githubusercontent.com/lchiquitto/esp32car/main/doc/bateria.jpg" width="200"> <img src="https://raw.githubusercontent.com/lchiquitto/esp32car/main/doc/esp32.jpg" width="200"> <img src="https://raw.githubusercontent.com/lchiquitto/esp32car/main/doc/ponteh.jpg" width="200">

O veículo carrega os seguintes sensores de medição, conectados ao microcontrolador através de um mini protoboard:

- Sensor ultrasônico (HC-SR04)
- Sensor de temperatura e umidade (DHT11)
- Sensor de som com microfone (KY-038)

<img src="https://raw.githubusercontent.com/lchiquitto/esp32car/main/doc/hcsr04.jpg" width="200"> <img src="https://raw.githubusercontent.com/lchiquitto/esp32car/main/doc/dht11.jpg" width="200"> <img src="https://raw.githubusercontent.com/lchiquitto/esp32car/main/doc/ky038.jpg" width="200"> <img src="https://raw.githubusercontent.com/lchiquitto/esp32car/main/doc/protoboard.jpg" width="200">

### Alimentação

As baterias, ligadas em série, fornecem aproximadamente 8.4V de tensão. Seus polos estão conectados às entradas "12 V" e "GND" da ponte H. O regulador de tensão da ponte H está habilitado. A conexão "5 V" (Vlogic) da ponte H fornece alimentação para o microcontrolador ESP32 através de seu pino VIN. Os sensores, que consomem pouca corrente, são alimentados pelo pino "3V3" do microcontrolador ESP32.

### Pinagem dos sensores

| ESP32  | Ponte H | DHT11 | KY038 | HC-SR04 |
| ------ | ------- | ----- | ----- | ------- |
| D25    | ENA     |       |       |         |
| D26    | IN1     |       |       |         |
| D27    | IN2     |       |       |         |
| D14    | IN3     |       |       |         |
| D12    | IN4     |       |       |         |
| D13    | ENB     |       |       |         |
| D04    |         | DATA  |       |         |
| D35    |         |       | A0    |         |
| D34    |         |       | D0    |         |
| D16    |         |       |       | ECHO    |
| D17    |         |       |       | TRIGGER |

### Comunicação

O microcontrolador é programado para conectar-se a uma rede WIFI pré-definida. O software executado no microcontrolador recebe comandos e envia dados das leituras dos sensores via rede (IP/UDP). Os comandos aceitos são: **F** (move o carro para frente), **B** (move o carro para trás), **L** (vira à esquerda), **R** (vira à direita), **S** (para o carro), **X** (solicita o envio dos dados dos sensores).

Os pacotes UDP que controlam a movimentação do carro possuem um único byte no seu campo de dados (as letras acima). O pacote enviado pelo carro para o controlador, contendo os dados de leituras dos sensores, possui o seguinte formato:

```
{"T": 24.00, "H": 48.00, "SA": 2067, "SD": 0, "D": 77.42}
```

Os valores são, respectivamente, temperatura (celsius), umidade (percentual), som/volume analógico (sem unidade), som/volume digital (sem unidade) e distância (centímetros).

### Código

Algumas observações sobre a organização e funcionamento do código:

- As bibliotecas `Ticker`, `DHTesp`, `WiFi` e `WiFiUdp` são usadas.
- São criadas três *threads* (Tasks), cada uma delas responsável por ler dados de um dos sensores.
- Cada tarefa executa uma única leitura e é suspensa. Um alarme (*Ticker*) é responsável por acordar as tarefas após *n* segundos (*n* é diferente para cada uma das tarefas).
- O SSID e a senha da rede WIFI é fixo no código.
- Em `setup()` ocorre a criação das tarefas, a inicialização dos sensores e pinos e a conexão com a rede WIFI.
- Em `loop()` o programa aguarda a chegada de comandos via pacotes de rede.
- As funções que leem dados dos sensores atualizam uma estrutura `sensorReadings` definida globalmente.
- A função `processCommand()` interpreta o comando recebido e chama a função adequada para tratá-lo.
- As funções `stopMoving()`, `moveForward()`, `moveBack()`, `turnLeft()` e `turnRight()` controlam os motores escrevendo no pinos conectados à ponte H.
- Para tirar o carro da inércia, uma tensão de aproximadamente 5.6V é aplicada (PWM 170) por 100ms. Após, a tensão é reduzida para aproximadamente 4.6V (PWM 140).

### Referências

As seguintes páginas e projetos foram consultados e usados como referência para esta implementação:

- Exemplos de comunicação UDP utilizando WiFiUdp.h [https://www.alejandrowurts.com/projects/esp32-wifi-udp/](https://www.alejandrowurts.com/projects/esp32-wifi-udp/)
- Exemplos básicos de comunicação UDP com Python [https://wiki.python.org/moin/UdpCommunication](https://wiki.python.org/moin/UdpCommunication)
- Tutorial de utilização do sensor de som KY-038 [https://diyi0t.com/sound-sensor-arduino-esp8266-esp32/](https://diyi0t.com/sound-sensor-arduino-esp8266-esp32/)
- Tutorial de utilização do sensor DHT11 [https://www.electronicshub.org/esp32-dht11-tutorial/](https://www.electronicshub.org/esp32-dht11-tutorial/)
- Tutorial de utilização da ponte H (L298N) [https://blog.eletrogate.com/guia-definitivo-de-uso-da-ponte-h-l298n/](https://blog.eletrogate.com/guia-definitivo-de-uso-da-ponte-h-l298n/)
- Exemplo de programação multitarefa para ESP32 [https://github.com/beegee-tokyo/DHTespOrg/tree/master/examples/DHT_ESP32](https://github.com/beegee-tokyo/DHTespOrg/tree/master/examples/DHT_ESP32)
