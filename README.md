# Rastreador-ESP32
Projeto de protótipo de rastreador usando ESP32 

# ZJ®Placa de Desenvolvimento LoRa, Módulo ESP32, DIY, WiFi, Bluetooth, 433MHz, 868MHz, 915MHz, 0, 96 Polegada OLED, SX1276, SX1278, V2.1

![image](https://github.com/user-attachments/assets/ad371596-af30-4ddc-adb9-9bb1dd1f6a18)

![image](https://github.com/user-attachments/assets/48dd17cd-17bb-450e-ac1d-6fe039d1896a)

![image](https://github.com/user-attachments/assets/4ecda7d5-56aa-44db-9491-49b33ab8e3cb)

![image](https://github.com/user-attachments/assets/a1a7e174-abee-41c8-beaf-7cf290e14494)


# Material utilizado
Para fazer o projeto, você precisará somente de um cabo micro-USB e um Módulo WiFi ESP32 com Suporte de Bateria, GPS e LORA 915MHZ. Este módulo já contém o ESP32 e GPS integrados, sendo ideal para o desenvolvimento deste projeto (nenhum circuito adicional será preciso). Além disso, esse módulo ainda conta com suporte para baterias de Li-Ion 18650 e chip para comunicação LoRa, coisas muito úteis em vários projetos ligados à Internet das Coisas.


# Para garantir um bom funcionamento do rastreador, o software desenvolvido para esse projeto faz uso do FreeRTOS. Há, ao todo, duas tarefas executadas pelo FreeRTOS neste projeto:

task_leitura_gps: tarefa responsável por obter de forma periódica (nesse caso, de dois em dois minutos) do módulo GPS as localizações geográficas e horários (via GPS). As localizações geográficas e horários são armazenados numa fila, com posições (espaços) suficientes para suportar até 8 horas de rastreamento (o suficiente para cobrir um dia inteiro de operação).

O uso de uma fila para armazenar as posições garante que estas fiquem armazenadas de forma segura e que sejam lidas na exata forma que foram inseridas na fila. A ordem de leitura é algo muito importante se você desejar traçar a rota que o rastreador percorreu, funcionalidade bastante comum em sistemas que utilizam localização GPS.

task_wifi_mqtt: tarefa responsável por se conectar ao wi-fi, se conectar ao broker MQTT, garantir que ambas conexões estejam estabelecidas e por enviar as localizações geográficas salvas na fila (quando há conectividade wi-fi e MQTT presentes).

Dessa forma, a funcionalidades de obtenção de localização geográfica e garantia de conectividade operam em paralelo, o que maximiza a performance do rastreador.

# Formas de envio da localização
Conforme dito no tópico anterior, há diversas formas de envio de localização geográfica a um servidor remoto, podendo ser através conectividade móvel / celular (GPRS) e também conectividade local (wi-fi), dependendo do uso do rastreador.

Por exemplo, pode-se utilizar rastreamento com base em conectividade wi-fi em aluguéis de maquinário pesado, tais como: tratores, escavadeiras, betoneiras, guindastes e afins. Para o aluguel desse tipo de maquinário, uma das principais modalidades de cobrança é a por hora de uso. Dessa forma, bastaria saber o momento que a máquina saiu de seu pátio-base e quando retornou, para assim se fazer a diferença de tempo e calcular o preço total do aluguel. Nesse caso, a empresa que aluga os maquinários não precisaria comprar planos de celular para cada rastreador de cada máquina, e sim somente garantir conectividade wi-fi no pátio-base.
Dessa maneira, o rastreador ficaria armazenando continuamente o seu posicionamento (e também horário, obtido via GPS) em memória interna e, chegando ao local com wi-fi, descarregaria estes dados todos de uma só vez via wi-fi no servidor remoto. Isso pode significar uma economia bem relevante, tanto em custo fixo (hardware do rastreador não precisar de modem GPRS e circuitos correlatos) quanto em custo variável (consumo de dados pelo rastreador).

Atualmente, em alguns casos, é possível utilizar rastreador com conectividade wi-fi para rastreamento “em tempo real” (respeitando o tempo entre envio de localizações, conforme descrito no tópico anterior). Isso é possível devido a boa parte dos smartphones vendidos terem função Hotspot ou até mesmo em veículos (carros e ônibus) que possuem roteadores wi-fi.

Outra forma comum de se fazer o envio de dados dos rastreadores é via GPRS (conectividade à Internet via celular). Nesta forma, basta haver cobertura de sinal de uma (ou mais) operadoras para que o rastreador consiga fazer o envio dos dados ao servidor remoto / nuvem. Em virtude da tendência de cada vez mais se ter zonas de coberturas de conectividade à Internet via celular, esta ainda é uma forma muito utilizada para envio dos dados do rastreador. O maior problema desta modalidade de envio reside no preço, afinal planos de dados ainda não custam barato.

Outra vertente que está começando a ganhar força nos meios de rastreamento são as LPWANs (Low-Power Wide Area Network), tais como LoRaWAN e Sigfox. Dessa forma, a cobertura em zona urbana e rural poderia ser garantida e, ainda, com um gasto em energia elétrica muito menor que via GPRS ou wi-fi. Isso viabilizaria rastreadores portáteis alimentados com baterias de baixa carga.
Por enquanto poucas empresas exploram esta maneira de envio de dados de rastreamento, porém acredito ser uma forte tendência num futuro próximo.
# Bibliotecas necessárias para o rastreador veicular com ESP32 e FreeRTOS
Para o projeto de um rastreador veicular com ESP32 e FreeRTOS, você precisará instalar duas bibliotecas:

 TinyGPS++ – Biblioteca para comunicação com módulo GPS da placa.
Obtenha esta biblioteca no repositório Github oficial dela (https://github.com/mikalhart/TinyGPSPlus) e a instale na Arduino IDE via Sketch > Include Library > Add .zip Library…
 AXP20X – Biblioteca para comunicação chip de gerenciamento de energia do módulo, de modo a permitir liberar energia para ligar o módulo GPS da placa.
Obtenha esta biblioteca no repositório Github oficial dela (https://github.com/lewisxhe/AXP202X_Library) e a instale na Arduino IDE via Sketch > Include Library > Add .zip Library…
