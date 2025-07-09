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
// Переменная отображает состояние кнопки 'BTN_Detect' отвечающей за захват цвета с датчика.
bool detect = false;
// Переменная отображает состояние кнопки 'BTN_Download' отвечающей за скачивание датасета.
bool download = false;
// Переменная отображает состояние кнопки 'BTN_Delete' отвечающей за удаление строки.
bool delete_row = false;
// Переменная содержит индекс строки в таблице которую надо удалить.
int idx_delete;




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

// Путь к текстовому файлу содержащему датасет.
String path_dataset = "/RGB_dataset.txt";


/** 
В документе реализована функция для захвата цвета с матрицы датчика, а так же файловая система SPIFFS, функции ответственные за создание файла с датасетом и обработку строк.
**/
#include "functions.h"

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

  // Инициализация файловой системы SPIFFS.
  SPIFFS_init();

  // Функция ответственная за создание файла с датасетом.
  createDataset_TXT(path_dataset);

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

  // Если на вебстраничке была нажата кнопка 'BTN_Detect' отвечающая за захват цвета с датчика, то выполняется следующий блок кода.
  if(detect){
    // Переменная отображает состояние кнопки 'BTN_Detect' отвечающей за захват цвета с датчика.
    detect = false;
    // Структура данных для хранения образца цвета полученного с датчика.
    Color DetectedColor[] = {{selected_color, 0, 0, 0, 0}};
    // Вызвов функции для захвата цвета с матрицы датчика.
    colorCapture(DetectedColor);
    // Вывести значения полученых параметров в последовательный порт.
    Serial.println("NAME: "+String(DetectedColor[0].name)+"RED: "+String(DetectedColor[0].r)+"  GREEN: "+String(DetectedColor[0].g)+"  BLUE: "+String(DetectedColor[0].b)+"  WHITE: "+String(DetectedColor[0].w));
    // Добавить строку с новыми данными в созданый файл с датасетом  и вернуть String строку с таблицей.**/
    String table = addRow(path_dataset, DetectedColor);
    // Передаём  String строку с таблицей на вебстраничку.
    sendJson(jsonString, doc_tx, "table_string", table);
  }


  // Если на вебстраничке была нажата кнопка 'BTN_Download' отвечающая за скачивание датасета, то выполняется следующий блок кода.
  if(download){
    // Переменная отображает состояние кнопки 'BTN_Download' отвечающей за скачивание датасета.
    download = false;

    // Открыть TXT файл с датасетом для чтения.
    File file_dataset_read = SPIFFS.open(path_dataset);
    // Получаем размер файла
    unsigned int dataset_size  = file_dataset_read.size();

    // Выделяем память для буфера
    uint8_t *buf = (uint8_t *)malloc(dataset_size);

    // Чтение данных из файла в буфер
    file_dataset_read.read(buf, dataset_size);

    // Отправляем TXT файл с датасетом из памяти SPIFFS пользователю.
    webSocket.broadcastBIN(buf, dataset_size);

    // Закрыть TXT файл с датасетом для чтения.
    file_dataset_read.close();
  }

  // Если на вебстраничке была нажата кнопка 'BTN_Delete' отвечающая за удаление строки, то выполняется следующий блок кода.
  if(delete_row){
    // Переменная отображает состояние кнопки 'BTN_Delete' отвечающая за удаление строки.
    delete_row = false;

    /** Функция достаёт файл с датасетом(таблицей) из памяти SPIFFS, конвертирует в String строку, удаляет из таблицы
    строку с заданым индексом, обновляет таблицу в памяти SPIFFS и возвращает String строку с таблицей.**/
    String table = deleteRowByIdx(path_dataset, idx_delete);

    // Передаём  String строку с таблицей на вебстраничку.
    sendJson(jsonString, doc_tx, "table_string", table);

    // Убрать комментарии если необходимо удалить файл из памяти SPIFFS.
    //SPIFFS.remove("/RGB_dataset.txt");
  }
}
