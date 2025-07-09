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

// TensorFlowLite_ESP32-----------------------------------------------------
// Модель машиного обучения.
#include "TensorFlowLiteModel.h"
// Параметры изображения передаваемого модели, а так же кол-во категорий для классификации.
#include "TensorFlowLiteModelConfig.h"
// -------------------------------------------------------------------------


// Данные WiFi сети.
const char* ssid = "WiFi_name";
const char* password = "WiFi_password";

// Экземпляр вебсервера ссылается на 80 порт.
AsyncWebServer server(80);
// Экземпляр WebSocket-сервера ссылается на 81 порт.
WebSocketsServer webSocket = WebSocketsServer(81);

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
В документе реализованы функции для захвата цвета с матрицы датчика, обработки полученых параметров и дальнейшей классификации.
**/
#include "functions.h"

/** 
В документе реализлвана функция webSocketEvent для обработки данных полученых через соеденение сокетов. 
**/
#include "socketConnection.h"

void setup() {
  // TensorFlowLite_ESP32---------------------------------------------------------------------------------------------------------

  // Для ведения журнала ошибок создадим переменную "error_reporter" на базе предоставляемых библиотекой Tensor Flow Lite структур данных.
  //static tflite::MicroErrorReporter micro_error_reporter;
  static tflite::MicroErrorReporter micro_error_reporter;
  // Переменную "error_reporter" необходимо передать в интерпретатор, который будет в свою очередь передавать в неё список ошибок.
  error_reporter = &micro_error_reporter;


  // Создадим экземпляр модели используя массив данных из документа "TensorFlowLiteModel.h" 
  model = tflite::GetModel(model_TFLite);
  // Проверка соответствия версии модели и версии библиотеки.
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }
  // Выделить обьём памяти для входного, выходного и промежуточных массивов модели,
  if (tensor_arena == NULL) {
    // Выделить более медляную память, но большую по обьёму.
    //tensor_arena = (uint8_t*) ps_calloc(kTensorArenaSize, 1);
    // Выделить более быструю память, но меньшую по обьёму.
    tensor_arena = (uint8_t *) heap_caps_malloc(kTensorArenaSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  }
  // Если не удалось выделить обьём памяти для входного, выходного и промежуточных массивов модели, то вывести сообщение об ошибке.
  if (tensor_arena == NULL) {
    printf("Couldn't allocate memory of %d bytes\n", kTensorArenaSize);
    return;
  }


  // Загрузить все методы, что содержит библиотека Tensor Flow Lite, для обработки данных моделью. (Занимает большой обьём памяти)
  // tflite::AllOpsResolver resolver;

  // Загрузить необходимые методы для обработки данных моделью из библиотеки Tensor Flow Lite.
  static tflite::MicroMutableOpResolver<9> micro_op_resolver;
  // AveragePool2D — операция, применяемая в свёрточных нейронных сетях (CNN), для уменьшения ширины и высоты входного тензора.
  //micro_op_resolver.AddAveragePool2D();
  // MaxPool2D — операция в свёрточных нейронных сетях (CNN), которая выполняет подвыборку данных, уменьшая ширину и высоту входного тензора.
  //micro_op_resolver.AddMaxPool2D();
  // Reshape — операция, используемая в машинном обучении и обработке данных, которая изменяет форму (размерность) тензора без изменения его данных
  //micro_op_resolver.AddReshape();
  // FullyConnected (полносвязанный слой) — используется для выполнения нелинейных преобразований данных и играет важную роль в моделях глубокого обучения.
  micro_op_resolver.AddFullyConnected();
  // Conv2D (свёрточный слой) — выполняет операцию свёртки над входными данными, чтобы извлекать локальные признаки, использует их для построения более сложных представлений на следующих слоях.
  //micro_op_resolver.AddConv2D();
  // DepthwiseConv2D — разновидность свёрточного слоя, которая применяется для увеличения вычислительной эффективности и уменьшения количества параметров модели.
  //micro_op_resolver.AddDepthwiseConv2D();
  // Softmax — функция активации, которая используется в выходных слоях нейронных сетей для задач классификации.
  micro_op_resolver.AddSoftmax();
  // Quantize (квантование) — процесс преобразования данных или моделей глубокого обучения, чтобы снизить их размер и вычислительную сложность, сохраняя при этом приемлемую точность.
  micro_op_resolver.AddQuantize();
  // Dequantize (деквантование) — процесс обратного преобразования данных из квантованного формата обратно в формат с плавающей точкой или в более высокую точность. 
  micro_op_resolver.AddDequantize();


  // Создадим экземпляр интерпретатора передавав необходимые данные для запуска модели.
  static tflite::MicroInterpreter static_interpreter(
    model, micro_op_resolver, tensor_arena, kTensorArenaSize);
      //model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);

  interpreter = &static_interpreter;


  // Выделим память для внутрених тензоров модели из выделеной ранее памяти tensor_arena.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  // При неудачном выделении памяти сообщить об ошибке.
  if (allocate_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
    return;
  }

  // Получить указатель на входной тензор модели.
  input = interpreter->input(0);
  // TensorFlowLite_ESP32---------------------------------------------------------------------------------------------------------

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


  // TensorFlowLite_ESP32---------------------------------------------------------------------------------------------------------
    // Входной тензор модели (вход модели).
    int8_t * input_data = input->data.int8;

    // Передаём на вход модели данные.
    // White - Красная компонента текущего пикселя.
    input_data[3] = (int8_t)(ScaledDetectedColor[0].w * 127.5);
    // Blue - Голубая компонента текущего пикселя.
    input_data[2] = (int8_t)(ScaledDetectedColor[0].b * 127.5);
    // Green - Зелёная компонента текущего пикселя.
    input_data[1] = (int8_t)(ScaledDetectedColor[0].g * 127.5);
    // Red - Красная компонента текущего пикселя.
    input_data[0] = (int8_t)(ScaledDetectedColor[0].r * 127.5);


    // Вызвать модель (произвести преобразование входного изображения в вероятность принадлежности 
    // данного изображения к каждому из возможных классов).
    if (kTfLiteOk != interpreter->Invoke()) {
      TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.");
    }

    // Получить выход модели.
    TfLiteTensor* output = interpreter->output(0);

    // Массив для хранения вероятностей для всех классов.
    int8_t probabilities [kCategoryCount];
    // Пройти по каждому элементу выхода модели.
    for(int i = 0; i < kCategoryCount; i++){
      // Получить вероятность для i-го класса.
      probabilities [i] = output->data.uint8[i];
    }

    // Получим таблицу с распределением вероятностей для каждого предсказываемого класса (цвета).
    String tableProbabilities = getProbabilitiesTable(kCategoryCount, probabilities, kCategoryLabels);
    // Передадим строку с данными для таблицы на вебстраничку, а также выведем её в последовательный порт.
    Serial.println(tableProbabilities);
    sendJson(jsonString, doc_tx, "table_string_probability", tableProbabilities);

    // Получим наименование предсказаного цвета.
    String prediction = getPredictedColor(kCategoryCount, probabilities, kCategoryLabels);

    // Получим таблицу с параметрами захвачеными с датчика, а также предсказанием цвета на базе этих параметров.
    String tableSensor = getSensorTable(DetectedColor, ScaledDetectedColor, prediction);
    // Передадим строку с данными для таблицы на вебстраничку, а также выведем её в последовательный порт.
    Serial.println(tableSensor);
    sendJson(jsonString, doc_tx, "table_string_sensor", tableSensor);
  }
}
