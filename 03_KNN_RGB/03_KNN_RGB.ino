// Библиотека для работы с wifi подключением (standard library).
#include <WiFi.h>  
// Библиотека для создания и запуска веб-сервера, обработки HTTP-запросов, формирования и отправки HTTP-ответов клиенту.
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
// Библиотека позволяет устанавливать постоянное соединение между сервером и клиентом, что делает возможным двустороннюю передачу данных в реальном времени.
#include <WebSocketsServer.h> 
// Библиотека используется для работы с JSON (JavaScript Object Notation) данными.
#include <ArduinoJson.h>
// Документ хранящий код HTML странички.
#include "WebPage.h"

// Данные WiFi сети.
const char* ssid = "WiFi_name";
const char* password = "WiFi_password";

// Экземпляр вебсервера ссылается на 80 порт.
AsyncWebServer server(80);
// Экземпляр WebSocket-сервера ссылается на 81 порт.
WebSocketsServer webSocket = WebSocketsServer(81);

// Переменная отображает цвет выбранный в элементе select (списке цветов для детекции) на вебстраничке.
String selected_color = "Red";
// Переменная отображает состояние кнопки 'BTN_Detect' отвечающей за захват цвета с датчика и его классификацию.
bool detect = false;


// S0 - пин для управления масштабирование выходной частоты.
const int s0 = 19;
// S1 - пин для управления масштабирование выходной частоты.
const int s1 = 18;
// S0 | S1 | выходная частота
//  L |  L | питание отключено
//  L |  H | 2%
//  H |  L | 20%
//  H |  H | 100%

// S2 - пин для выбора измеряемой цветовой комоненты (красной, зелёной, голубой, белой).
const int s2 = 2;
// S3 - пин для выбора измеряемой цветовой комоненты (красной, зелёной, голубой, белой).
const int s3 = 4;
// S2 | S3 | цветовая комонента
//  L |  L | красная
//  L |  H | голубая
//  H |  L | белая
//  H |  H | зелёная

// out - пин с которого считывается значение цветовой компоненты (диапозон от 0 до 255).
const int out = 15;

// Кол-во ближайших рассматриваемых образцов цвета к полученному с датчика.
const int K = 15;

/** 
В документе реализована функция для захвата цвета с матрицы датчика, а так же классификацию цвета.
**/
#include "functions.h"
// Структура данных хранящая список ближайших образцов цвета к полученному с датчика.
dotDistance minDistances [K];
/** 
В документе реализлвана функция webSocketEvent для обработки данных полученых через соеденение сокетов. 
**/
#include "socketConnection.h"

void setup() {
  // Последовательный порт (serial port).
  Serial.begin(115200);

  // Устанавливаем пины в режим выходов.
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);  
  pinMode(s2, OUTPUT);  
  pinMode(s3, OUTPUT);  
  // Устанавливаем пин в режим входа.
  pinMode(out, INPUT);  
  // Выставляем выходную частоту в 100% для датчика цвета.
  digitalWrite(s0, HIGH);  
  digitalWrite(s1, HIGH);  


  // Подключение к WiFi.
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  // Вывод IP адреса в последовательный порт.
  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP());

  
  /** Инициализация вебсервера (загружаем HTML страничку на WebServer, делаем её корневой "/"):
        + "/" - корневая папка, 
        + HTTP_GET - HTTP-метод GET для запроса данных с сервера
        + [](AsyncWebServerRequest *request) {} - лямбда-функция
        + AsyncWebServerRequest *request - указатель на объект, 
          который содержит всю информацию о запросе, поступившем на сервер.**/
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    // отправить (200 - http код ответа, метод отправки по html, HTML страничка)
    request -> send(200, "text\html", getHTML());
  });

  // Запуск вебсокета.
  webSocket.begin();
  // При приёме данных от клиента контролером через соеденение вебсокетов передать данные функцие webSocketEvent для дальнейшей обработки.
  // Функция webSocketEvent реализована в документе "receiveData.h".
  webSocket.onEvent(webSocketEvent);

  // Запуск вебсервера.
  server.begin();

}

void loop() {

  /** Метод webSocket.loop() обеспечивает:
        - Поддержание активного соединения с клиентами.
        - Обработку входящих данных от клиентов.
        - Обработку новых подключений и отключений клиентов.
        - Отправку данных клиентам, если это необходимо.**/
  webSocket.loop();

  // Если на вебстраничке была нажата кнопка 'BTN_Detect' отвечающая за захват цвета с датчика и его классификацию, то выполняется следующий блок кода.
  if(detect){
    // Переменная отображает состояние кнопки 'BTN_Detect' отвечающей за захват цвета с датчика и его классификацию.
    detect = false;
    // Структура данных для хранения образца цвета полученного с датчика.
    Color DetectedColor[] = {{"DetectedColor", 0, 0, 0, 0}};
    // Вызвов функции для захвата цвета с матрицы датчика.
    colorCapture(DetectedColor);
    // Структура данных для хранения отмасштабированного образца цвета полученного с датчика.
    Color ScaledDetectedColor[] = {{"ScaledDetectedColor", 0, 0, 0, 0}};
    // Функция для масштабирования данных полученных с датчика. 
    scaleData(MeanAndScale, DetectedColor, ScaledDetectedColor);
    // Получим массив ближайших цветов для образца цвета полученного с датчика.
    getListOfNearestColors(K, minDistances, ScaledDetectedColor[0].r, ScaledDetectedColor[0].g, ScaledDetectedColor[0].b, ScaledDetectedColor[0].w);
    // Получим наименование цвета захваченого датчиком.
    String prediction = colorClasify(K, minDistances, colors, numColors);

    // Добавить строку с новыми данными в таблицу.
    String table = prepareKNNTable(prediction, DetectedColor, ScaledDetectedColor, minDistances, K);

    // Передаём  String строку с таблицей на вебстраничку.
    sendJson(jsonString, doc_tx, "table_string", table);

    // Выведим список ближайших соседей в последовательный порт.
    for (int i = 0; i < K; i++) {
      Serial.println(String(i+1) + ") " + String(minDistances[i].name) + "| Dist: " + String(minDistances[i].dist) + "|red: " + String(minDistances[i].r) + "|green: " + String(minDistances[i].g) + "|blue: " + String(minDistances[i].b) + "|white: "  + String(minDistances[i].w) );
    }
    // Выведим предсказаный цвет в последовательный порт.
    Serial.println("Prediction: " + prediction);
  }
}
