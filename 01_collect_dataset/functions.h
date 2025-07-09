// Структура данных для хранения образцов цвета полученных с датчика.
struct Color {
  // Наименование цвета.
  String name;
  // Значения компонент цвета.
  int r, g, b, w;
};


/** Реализлвана функция для захвата цвета с матрицы датчика. 
  Color DetectedColor[] - Структура данных для хранения образца цвета полученного с датчика.**/
void colorCapture(Color DetectedColor[]){

  // Состояния пинов S2 и S3 определяют, какой набор фотодиодов мы используем: LOW/LOW — для КРАСНОГО.
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  // Получаем значение для красной компоненты цвета.
  DetectedColor[0].r = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  delay(20);

  // Меняем состояния пинов S2 и S3 на LOW/HIGH для голубой компоненты цвета.
  digitalWrite(s3, HIGH); 
  // Получаем значение для голубой компоненты цвета.
  DetectedColor[0].b = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  delay(20);

  // Меняем состояния пинов S2 и S3 на HIGH/HIGH для зелёной компоненты цвета.
  digitalWrite(s2, HIGH);
  // Получаем значение для зелёной компоненты цвета.
  DetectedColor[0].g = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  delay(20);

  // Меняем состояния пинов S2 и S3 на HIGH/LOW для белой компоненты цвета.
  digitalWrite(s3, LOW);
  // Получаем значение для белой компоненты цвета.
  DetectedColor[0].w = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  delay(20);
}




/** SPIFFS — это файловая система, предназначенная для флэш-устройств SPI NOR на встроенных объектах. 
Она поддерживает выравнивание износа, проверку целостности файловой системы и многое другое. **/
#include <SPIFFS.h>


/** Функция инициализации для файловой системы SPIFFS. Инициализация должна быть проведена в void setup() {}.**/
void SPIFFS_init() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    ESP.restart();
  }
  else {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
  }
}


/** Функция должна быть проведена в void setup() {}.
    создаёт файл и первую строку **/
void firstLine(String path) {
  // Выводит в сериал-монитор сообщение о начале создания файла.
  Serial.println("Create file: " + path);
  // Открывает или создаёт файл для записи по указанному пути.
  File file_dataset = SPIFFS.open("/RGB_dataset.txt", FILE_WRITE);

  // Пытается записать в файл первую строку заголовка CSV.
  if(file_dataset.print("RED, GREEN, BLUE, WHITE, COLOR, 0 \n")){
    // Если запись успешна, выводит сообщение об успешном создании.
    Serial.println("File " + path + " is created.");
  } else {
    // Если запись не удалась, выводит сообщение об ошибке.
    Serial.println("File " + path + " is NOT created.");
  }
  // Закрывает файл, чтобы сохранить изменения.
  file_dataset.close();
}

/** Функция должна быть проведена в void setup() {}.
    Функция ответственная за создание файла с датасетом. **/
void createDataset_TXT(String path) {
  // Открывает корневую директорию файловой системы SPIFFS.
  File root = SPIFFS.open("/");
  // Получает первый файл из директории.
  File file = root.openNextFile();
  // Если файлов в корневой директории нет — сразу создаёт новый файл с заголовком.
  if(!file){
    firstLine(path);
  }

  // Цикл перебора всех файлов в корневой директории.
  while(file){
    // Печатает имя текущего файла в сериал-монитор.
    Serial.print("FILE: ");
    Serial.println(file.name());

    // Сравнивает путь текущего файла с заданным.
    if(String(String('/')+ String(file.name())) == path){
      // Если файл с нужным именем найден — сообщает об этом и выходит из цикла.
      Serial.println("This file " + path + " was found.");
      break;
    } else {
      // Если имя не совпадает — сообщает об этом.
      Serial.println("This file " + path + " was NOT found.");
    }
    // Переходит к следующему файлу.
    file = root.openNextFile();
    // Если файлов больше нет — создаёт файл с заголовком.
    if(!file){
      firstLine(path);
    }
  }
}



/** Функция достаёт файл с датасетом(таблицей) из  памяти SPIFFS, конвертирует в String строку и возвращает её.**/
String getTableString(String path) {
  // Открыть TXT файл с датасетом для чтения.
  File file_dataset_read = SPIFFS.open(path);
  // String в который будет записан датасет из файла.
  String table = "";

  // Запишем датасет из файла в String table.
  while(file_dataset_read.available()) {
    table = file_dataset_read.readString();
  }
  // Закрыть TXT файл с датасетом для чтения.
  file_dataset_read.close();
  // Возвращаем таблицу в виде String строки.
  return table;
}


/** Функция достаёт файл с датасетом(таблицей) из памяти SPIFFS, конвертирует в String строку, добавляет к таблице 
строку с новыми данными, обновляет таблицу в памяти SPIFFS и возвращает String строку.**/
String addRow(String path, Color DetectedColor[]) {
  // Получить String строку с таблицей из памяти SPIFFS.
  String table = getTableString(path);
  // Получить индекс последней запятой во всём файле.
  int comma = table.lastIndexOf(',');
  // Зная индекс последней запятой получим индекс последнего экземпляра данных из таблицы.
  String idxString = table.substring(comma+2);
  int idx = idxString.toInt();

  // Открыть TXT файл с датасетом для записи.
  File file_dataset_write = SPIFFS.open(path, FILE_WRITE);
  // Добавим к таблице строку с новыми данными.
  table += String(DetectedColor[0].r) + ", " + String(DetectedColor[0].g) + ", " + String(DetectedColor[0].b) + ", " + String(DetectedColor[0].w) + ", " + DetectedColor[0].name + ", " + String(idx+1) +" \n";
  // Записать обновлённую таблицу в файл.
  if(file_dataset_write.print(table)){
    Serial.println("Row to " + path + " is added.");
  } else {
    Serial.println("Row to " + path + " is NOT added.");
  }
  // Закрыть TXT файл с датасетом для записи.
  file_dataset_write.close();

  // Возвращаем таблицу в виде String строки.
  return table;
}



/** Функция достаёт файл с датасетом(таблицей) из памяти SPIFFS, конвертирует в String строку, удаляет из таблицы
строку с заданым индексом, обновляет таблицу в памяти SPIFFS и возвращает String строку.**/
String deleteRowByIdx(String path, int idx) {
  // Получить String строку с таблицей из памяти SPIFFS.
  String table = getTableString(path);

  // String в который будет записан датасет с удалённой строкой.
  String shortMemory = "";

  // В цикле while пройдём по всем строкам таблицы.
  // Индекс первого элемента текущей строки в String table.
  int start = 0;
  while (start < table.length()) {
    // Индекс последнего элемента текущей строки в String table.
    int end = table.indexOf('\n', start);
    // Если текущая строка последняя в String table.
    if (end == -1) end = table.length();

    // Текущая строка в String table.
    String line = table.substring(start, end);
    //line.trim(); // убрать лишние пробелы

    // Индекс последней запятой в текущей строке.
    int lastComma = line.lastIndexOf(',');
    // Проверка существует ли запятая.
    if (lastComma != -1) {
      // Получить индекс текущей строки в формате String.
      String idxStr = line.substring(lastComma + 2);
      //indexStr.trim();
      // Если индекс текущей строки не равен индексу строки, которую надо удалить,
      if (idxStr.toInt() != idx) {
        // Оставим текущую строку в таблице.
        shortMemory += line + "\n";
      }
    }
    // Обновим индекс первого элемента текущей строки в String table.
    start = end + 1;
  }

  // Открыть TXT файл с датасетом для записи.
  File file_dataset_write = SPIFFS.open(path, FILE_WRITE);

  // Записать обновлённую таблицу в файл.
  if(file_dataset_write.print(shortMemory)){
    Serial.println("Row with index " + String(idx) + " was deleted.");
  } else {
    Serial.println("Row with index " + String(idx) + " was NOT deleted.");
  }
  // Закрыть TXT файл с датасетом для записи.
  file_dataset_write.close();

  // Возвращаем таблицу в виде String строки.
  return shortMemory;
}





// Не используется:

int getExampleIndex(String row) {
  // Получим индекс запятой в строке.
  int comma = row.indexOf(',');
  // Получим из всей строки число.
  String idxString = row.substring(0, comma);
  // Полученное число переведём в формат int.
  int idx = idxString.toInt();
  return idx;
}


String getRowByIdx(String table, int idx) {

  int start = 0;
  while (start < table.length()) {
    int end = table.indexOf('\n', start);
    if (end == -1) end = table.length(); // последняя строка

    String line = table.substring(start, end);
    line.trim(); // убрать лишние пробелы

    int lastComma = line.lastIndexOf(',');
    if (lastComma != -1) {
      String indexStr = line.substring(lastComma + 1);
      indexStr.trim();
      if (indexStr.toInt() == idx) {
        return line;
      }
    }

    start = end + 1;
  }

  return ""; // если не найдено
}