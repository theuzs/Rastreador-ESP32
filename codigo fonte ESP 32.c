/* Projeto: rastreador com placa ESP32 TTGO T Beam
 * Autor: Pedro Bertoleti
 * 
 * Bibliotecas utlizadas:
 * - TinyGPS++: https://github.com/mikalhart/TinyGPSPlus
 * - axp20x: https://github.com/lewisxhe/AXP202X_Library
 */
  
#include <TinyGPS++.h>
#include <axp20x.h>
#include <WiFi.h>
#include <PubSubClient.h>
 
/* Definições gerais */
/* o módulo GPS da placa está ligado na serial 1 do ESP32 */
#define SERIAL_GPS                  1 
#define BAUDRATE_SERIAL_GPS         9600
#define GPIO_RX_SERIAL_GPS          34
#define GPIO_TX_SERIAL_GPS          12
#define TEMPO_ENTRE_POSICOES_GPS    120 //s
#define TEMPO_UM_DIA_DE_TRABALHO    28800 //s (28800s = 8h)
#define TAMANHO_FILA_POSICOES_GPS   (28800/TEMPO_ENTRE_POSICOES_GPS)
#define TEMPO_LEITURA_SERIAL_GPS    1000  //ms
 
/* definições de temporização das tarefas */
#define TEMPO_PARA_VERIFICAR_WIFI_MQTT     1000  //ms
#define TICKS_ESPERA_POSICAO_GPS           ( TickType_t )1000
#define TICKS_ESPERA_ENVIO_POSICAO_GPS     ( TickType_t )10000
 
/* Baudrate da serial usada para debug (serial monitor) */
#define BAUDRATE_SERIAL_DEBUG   115200
 
/* Filas */
/* Fila para armazenar posições GPS */
QueueHandle_t xQueue_GPS;
 
/* Semáforos */
SemaphoreHandle_t xSerial_semaphore;
 
/* Estrutura de dados de posição */
typedef struct
{
   float latitude;
   float longitude;
   int horas;
   int minutos;
   int segundos;
}TPosicao_GPS;
 
/* Variaveis e constantes - wi-fi */
/* Coloque aqui o nome da rede wi-fi ao qual o ESP32 precisa se conectar */
const char* ssid_wifi = " ";
 
/* Coloque aqui a senha da rede wi-fi ao qual o ESP32 precisa se conectar */
const char* password_wifi = " ";
 
WiFiClient espClient;
 
/* Variaveis e constantes - MQTT */
/* URL do broker MQTT */
const char* broker_mqtt = "broker.hivemq.com";
const char* topico_publish_mqtt = "MakerHero_localizacao_gps";
 
/* Porta para se comunicar com broker MQTT */
int broker_port = 1883;
 
PubSubClient MQTT(espClient);
 
/* Demais objetos e variáveis globais */
TinyGPSPlus gps;
HardwareSerial GPS(SERIAL_GPS);
AXP20X_Class axp;
 
/* Protótipos das funções das tarefas */
void task_leitura_gps( void *pvParameters );
void task_wifi_mqtt( void *pvParameters );
 
/* Protótipos de funções gerais */
void init_wifi(void);
void conecta_wifi(void);
void verifica_conexao_wifi(void);
void init_MQTT(void);
void conecta_broker_MQTT(void);
void verifica_conexao_mqtt(void);
 
void setup() 
{
    /* Inicializa serial para debug */
    Serial.begin(BAUDRATE_SERIAL_DEBUG);
 
    /* Inicializa comunicação I²C com chip gerenciador de energia (AXP192) */
    Wire.begin(21, 22);
    if (!axp.begin(Wire, AXP192_SLAVE_ADDRESS)) 
        Serial.println("Sucesso ao inicializar comunicação com chip de energia (AXP192)");        
    else
    {
        Serial.println("Falha ao inicializar comunicação com chip de energia (AXP192). O ESP32 será reiniciado...");
        delay(2000);
        ESP.restart();
    }
 
    /* Energiza módulo GPS */
    axp.setPowerOutPut(AXP192_LDO2, AXP202_ON);
    axp.setPowerOutPut(AXP192_LDO3, AXP202_ON);
    axp.setPowerOutPut(AXP192_DCDC1, AXP202_ON);
    axp.setPowerOutPut(AXP192_DCDC2, AXP202_ON);
    axp.setPowerOutPut(AXP192_DCDC3, AXP202_ON);
    axp.setPowerOutPut(AXP192_EXTEN, AXP202_ON);
 
    /* Inicializa serial para comunicar com GPS */
    GPS.begin(BAUDRATE_SERIAL_GPS, 
              SERIAL_8N1, 
              GPIO_RX_SERIAL_GPS, 
              GPIO_TX_SERIAL_GPS);
 
    /* Criação das filas */         
    xQueue_GPS = xQueueCreate(TAMANHO_FILA_POSICOES_GPS, sizeof(TPosicao_GPS));
 
    if (xQueue_GPS == NULL)
    {
        Serial.println("Falha ao inicializar filas. O programa nao pode prosseguir. O ESP32 sera reiniciado...");
        delay(2000);
        ESP.restart();
    }
     
    /* Criação dos semáforos */        
    xSerial_semaphore = xSemaphoreCreateMutex();
 
    if (xSerial_semaphore == NULL)
    {
        Serial.println("Falha ao inicializar semaforos. O programa nao pode prosseguir. O ESP32 sera reiniciado...");
        delay(2000);
        ESP.restart();
    }
     
    /* Configuração das tarefas */
    xTaskCreate(
    task_leitura_gps                    /* Funcao a qual esta implementado o que a tarefa deve fazer */
    , "leitura_gps"                     /* Nome (para fins de debug, se necessário) */
    ,  4096                             /* Tamanho da stack (em words) reservada para essa tarefa */
    ,  NULL                             /* Parametros passados (nesse caso, não há) */
    ,  6                                /* Prioridade */
    ,  NULL );                          /* Handle da tarefa, opcional (nesse caso, não há) */
 
    xTaskCreate(
    task_wifi_mqtt             
    , "wifi_mqtt"        
    ,  4096    
    ,  NULL                      
    ,  5
    ,  NULL );   
 
    /* O FreeRTOS está inicializado */
}
 
void loop() 
{
    /* todas as funcionalidades são feitas pelas tarefas 
       task_leitura_gps e task_wifi_mqtt */
}
 
/* 
 *  Funções gerais
 */
  
/* Função: inicializa wi-fi
 * Parametros: nenhum
 * Retorno: nenhum 
 */
void init_wifi(void) 
{
    Serial.println("------WI-FI -----");
    Serial.print("Conectando-se a rede: ");
    Serial.println(ssid_wifi);
    Serial.println("Aguarde...");    
    conecta_wifi();
}
 
/* Função: conecta-se a rede wi-fi
 * Parametros: nenhum
 * Retorno: nenhum 
 */
void conecta_wifi(void) 
{
    /* Se ja estiver conectado, nada é feito. */
    if (WiFi.status() == WL_CONNECTED)
        return;
 
    /* refaz a conexão */
    WiFi.begin(ssid_wifi, password_wifi);
     
    while (WiFi.status() != WL_CONNECTED) 
    {        
        vTaskDelay( 100 / portTICK_PERIOD_MS );
        Serial.print(".");
    }
   
    Serial.println();
    Serial.print("Conectado com sucesso a rede wi-fi ");
    Serial.println(ssid_wifi);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}
 
/* Função: verifica se a conexao wi-fi está ativa 
 *         (e, em caso negativo, refaz a conexao)
 * Parametros: nenhum
 * Retorno: nenhum 
 */
void verifica_conexao_wifi(void)
{
    conecta_wifi(); 
}
 
/* Função: inicializa MQTT
 * Parametros: nenhum
 * Retorno: nenhum 
 */
void init_MQTT(void)
{
    MQTT.setServer(broker_mqtt, broker_port);  
}
 
/* Função: conecta-se ao broker MQTT
 * Parametros: nenhum
 * Retorno: nenhum 
 */
void conecta_broker_MQTT(void) 
{
    char mqtt_id_randomico[5] = {0};
    int i;
     
    while (!MQTT.connected()) 
    {
        /* refaz a conexão */
        Serial.print("* Tentando se conectar ao broker MQTT: ");
        Serial.println(broker_mqtt);
 
        /* gera id mqtt randomico */
        randomSeed(random(9999));
        sprintf(mqtt_id_randomico, "%ld", random(9999));
         
        if (MQTT.connect(mqtt_id_randomico)) 
            Serial.println("Conexao ao broker MQTT feita com sucesso!");
        else
            Serial.println("Falha ao se conectar ao broker MQTT");
    }
}
 
/* Função: verifica se a conexao ao broker MQTT está ativa 
 *         (e, em caso negativo, refaz a conexao)
 * Parametros: nenhum
 * Retorno: nenhum 
 */
void verifica_conexao_mqtt(void)
{
    conecta_broker_MQTT(); 
}
 
/* 
 *  Tarefas
 */
 
/* Esta task é responsável por: 
 * - Obter a localização (latitude e longitude) do módulo GPS 
 * - Enviar a localização obtida para outras tasks (usando uma fila) 
 */
void task_leitura_gps( void *pvParameters )
{
    TPosicao_GPS posicao_gps;
    unsigned long timestamp_start = 0;
    char str_horario[20] = {0};
      
    while(1)
    {
        /* Espera pelo tempo (definido em TEMPO_ENTRE_POSICOES_GPS) entre posições GPS */
        vTaskDelay( (TEMPO_ENTRE_POSICOES_GPS*1000) / portTICK_PERIOD_MS );
 
        /* Faz a leitura de todos os dados do GPS (por alguns milissegundos) */
        timestamp_start = millis();
        do
        {
            while (GPS.available())
            gps.encode(GPS.read());
        } while ( (millis() - timestamp_start) < TEMPO_LEITURA_SERIAL_GPS);
 
        /* Obtem e envia posição / localização para outras tasks usando uma fila*/
        posicao_gps.latitude = gps.location.lat();
        posicao_gps.longitude = gps.location.lng();
        posicao_gps.horas = gps.time.hour();
        posicao_gps.minutos = gps.time.minute();
        posicao_gps.segundos = gps.time.second();
        xQueueSend(xQueue_GPS, ( void * ) &posicao_gps, TICKS_ESPERA_ENVIO_POSICAO_GPS);
         
        xSemaphoreTake(xSerial_semaphore, portMAX_DELAY );
        Serial.println("Localizacao GPS obtida:");
        Serial.print("* Latitude: ");
        Serial.println(posicao_gps.latitude);
        Serial.print("* Longitude: ");
        Serial.println(posicao_gps.longitude);
        sprintf(str_horario,"%02d:%02d:%02d", posicao_gps.horas,
                                              posicao_gps.minutos,
                                              posicao_gps.segundos); 
        Serial.print("Horario (GMT 0): ");
        Serial.println(str_horario);
         
        xSemaphoreGive(xSerial_semaphore);
    }
}
  
/* Esta task é responsável por: 
 * - Conectar e reconectar (em caso de perda de conexao) no wi-fi
 * - Conectar e reconectar (em caso de perda de conexao) no broker MQTT
 * - Enviar, quando houver, as posições GPS da fila para a nuvem
 */
void task_wifi_mqtt( void *pvParameters )
{
    TPosicao_GPS posicao_gps_recebida;
    char msg_mqtt[200] = {0};
     
    /* Inicializa wi-fi e faz a conexao */
    xSemaphoreTake(xSerial_semaphore, portMAX_DELAY );
    init_wifi();
    xSemaphoreGive(xSerial_semaphore);
    
    /* Inciializa mqtt e faz conexao ao broker */ 
    xSemaphoreTake(xSerial_semaphore, portMAX_DELAY );
    init_MQTT();
    xSemaphoreGive(xSerial_semaphore);
 
    while(1)
    {
        /* Verifica tudo novamente após tempo definido em TEMPO_PARA_VERIFICAR_WIFI_MQTT */
        vTaskDelay( TEMPO_PARA_VERIFICAR_WIFI_MQTT / portTICK_PERIOD_MS );
       
        /* verifica se a conexao wi-fi e ao broker MQTT estão ativas 
           (e, em caso negativo, refaz a conexao)
        */
        xSemaphoreTake(xSerial_semaphore, portMAX_DELAY );
        verifica_conexao_wifi();
        verifica_conexao_mqtt();
        xSemaphoreGive(xSerial_semaphore);
 
        /* Se há posições GPS na fila para serem enviadas por MQTT, faz o envio aqui */         
        if( xQueueReceive( xQueue_GPS, &( posicao_gps_recebida ), TICKS_ESPERA_POSICAO_GPS) )
        {     
            sprintf(msg_mqtt,"Latitude: %f  Longitude: %f Horario: %02d:%02d:%02d", posicao_gps_recebida.latitude, 
                                                                                    posicao_gps_recebida.longitude,
                                                                                    posicao_gps_recebida.horas,
                                                                                    posicao_gps_recebida.minutos,
                                                                                    posicao_gps_recebida.segundos);
            MQTT.publish(topico_publish_mqtt, msg_mqtt);
             
            xSemaphoreTake(xSerial_semaphore, portMAX_DELAY );
            Serial.println("Localizacao GPS enviada para nuvem");
            xSemaphoreGive(xSerial_semaphore);
        }
 
        /* keep-alive da comunicação com broker MQTT */
        MQTT.loop();
    }
}