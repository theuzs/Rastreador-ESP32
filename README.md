# Rastreador-ESP32
Projeto de protótipo de rastreador usando ESP32 

# ZJ®Placa de Desenvolvimento LoRa, Módulo ESP32, DIY, WiFi, Bluetooth, 433MHz, 868MHz, 915MHz, 0, 96 Polegada OLED, SX1276, SX1278, V2.1

![image](https://github.com/user-attachments/assets/ad371596-af30-4ddc-adb9-9bb1dd1f6a18)

![image](https://github.com/user-attachments/assets/db3f9bbb-882b-46df-9fe7-14f98eefc57f)

![image](https://github.com/user-attachments/assets/4ecda7d5-56aa-44db-9491-49b33ab8e3cb)


# Material utilizado
Para fazer o projeto, você precisará somente de um cabo micro-USB e um Módulo WiFi ESP32 com Suporte de Bateria, GPS e LORA 915MHZ. Este módulo já contém o ESP32 e GPS integrados, sendo ideal para o desenvolvimento deste projeto (nenhum circuito adicional será preciso). Além disso, esse módulo ainda conta com suporte para baterias de Li-Ion 18650 e chip para comunicação LoRa, coisas muito úteis em vários projetos ligados à Internet das Coisas.


# Para garantir um bom funcionamento do rastreador, o software desenvolvido para esse projeto faz uso do FreeRTOS. Há, ao todo, duas tarefas executadas pelo FreeRTOS neste projeto:

task_leitura_gps: tarefa responsável por obter de forma periódica (nesse caso, de dois em dois minutos) do módulo GPS as localizações geográficas e horários (via GPS). As localizações geográficas e horários são armazenados numa fila, com posições (espaços) suficientes para suportar até 8 horas de rastreamento (o suficiente para cobrir um dia inteiro de operação).

O uso de uma fila para armazenar as posições garante que estas fiquem armazenadas de forma segura e que sejam lidas na exata forma que foram inseridas na fila. A ordem de leitura é algo muito importante se você desejar traçar a rota que o rastreador percorreu, funcionalidade bastante comum em sistemas que utilizam localização GPS.

task_wifi_mqtt: tarefa responsável por se conectar ao wi-fi, se conectar ao broker MQTT, garantir que ambas conexões estejam estabelecidas e por enviar as localizações geográficas salvas na fila (quando há conectividade wi-fi e MQTT presentes).

Dessa forma, a funcionalidades de obtenção de localização geográfica e garantia de conectividade operam em paralelo, o que maximiza a performance do rastreador.


# Bibliotecas necessárias para o rastreador veicular com ESP32 e FreeRTOS
Para o projeto de um rastreador veicular com ESP32 e FreeRTOS, você precisará instalar duas bibliotecas:

TinyGPS++ – Biblioteca para comunicação com módulo GPS da placa.
Obtenha esta biblioteca no repositório Github oficial dela (https://github.com/mikalhart/TinyGPSPlus) e a instale na Arduino IDE via Sketch > Include Library > Add .zip Library…
AXP20X – Biblioteca para comunicação chip de gerenciamento de energia do módulo, de modo a permitir liberar energia para ligar o módulo GPS da placa.
Obtenha esta biblioteca no repositório Github oficial dela (https://github.com/lewisxhe/AXP202X_Library) e a instale na Arduino IDE via Sketch > Include Library > Add .zip Library…
