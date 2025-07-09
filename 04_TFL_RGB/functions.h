#include <cmath>
// Структура данных для хранения образцов цвета полученных с датчика.
struct Color {
  // Наименование цвета.
  const char* name;
  // Значения компонент цвета.
  float r, g, b, w;
};


// Массив содержащий стандартное отклонение и среднее значение для каждой компоненты цвета.
Color MeanAndScale[] = {
  {"  MEAN ",  17.7875, 21.775,  19.1375,  5.7875},
  {"  SCALE ",  14.87001492, 14.88705394,  11.70335823,  4.21216616}
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


/** Реализлвана функция для масштабирования данных полученных с датчика. 
  Color MeanAndScale[] - Массив содержащий стандартное отклонение и среднее значение для каждой компоненты цвета.
  Color DetectedColor[] - Структура содержащая  образец цвета полученный с датчика.
  Color ScaledDetectedColor[] - Структура данных для хранения отмасштабированого образца цвета полученного с датчика.**/
void scaleData(Color MeanAndScale[], Color DetectedColor[], Color ScaledDetectedColor[]){
  // Масштабируем красную компоненту цвета.
  ScaledDetectedColor[0].r = (DetectedColor[0].r - MeanAndScale[0].r) / MeanAndScale[1].r;
  // Масштабируем голубую компоненту цвета.
  ScaledDetectedColor[0].g = (DetectedColor[0].g - MeanAndScale[0].g) / MeanAndScale[1].g;
  // Масштабируем зелёную компоненту цвета.
  ScaledDetectedColor[0].b = (DetectedColor[0].b - MeanAndScale[0].b) / MeanAndScale[1].b;
  // Масштабируем белую компоненту цвета.
  ScaledDetectedColor[0].w = (DetectedColor[0].w - MeanAndScale[0].w) / MeanAndScale[1].w;
}



/** Функция возвращает наименование цвета захваченого датчиком.
  int kCategoryCount - Кол-во классов предсказываемых моделью.
  int8_t probabilities [] - Массив для хранения вероятностей для всех классов.
  char* kCategoryLabels[] - Массив наименований категорий, которые модель может классифицировать.   **/
String getPredictedColor(int kCategoryCount, int8_t probabilities [], const char* kCategoryLabels[]){
  // Индекс для цвета с наибольшей вероятностью.
  int idx = 0;
  // Максимальная вероятность.
  int8_t maxProbability = probabilities[idx];

  // Пройдём через все сущности "цвета" для которых модель возвращает вероятность.
  for (int i = 1; i < kCategoryCount; i++) {
    // Если вероятность для текущей сущности "цвета" больше максимальная вероятности назначеной ранее,
    if(probabilities[i] > maxProbability){
      // Обновим максимальную вероятность.
      maxProbability = probabilities[i];
      // Обновим индекс для цвета с наибольшей вероятностью.
      idx = i;
    }
  }

  // Вернём наименование цвета с наибольшей вероятностью.
  return String(kCategoryLabels[idx]);
}




/** Функция возвращает строку на базе которой будет построена таблица с распределением вероятностей для каждого предсказываемого класса (цвета).
  int kCategoryCount - Кол-во классов предсказываемых моделью.
  int8_t probabilities [] - Массив для хранения вероятностей для всех классов.
  char* kCategoryLabels[] - Массив наименований категорий, которые модель может классифицировать.   **/
String getProbabilitiesTable(int kCategoryCount, int8_t probabilities [], const char* kCategoryLabels[]){
  // Создадим строку на базе которой будет построена таблица с распределением вероятностей для каждого предсказываемого класса (цвета).
  String row_1 = "";
  String row_2 = "";
  for(int i = 0; i < kCategoryCount; i++){
    if(i != kCategoryCount-1){
      row_1 += String(kCategoryLabels[i]) +  ", ";
      row_2 += String(probabilities [i]) +  ", ";
    } else {
      row_1 += String(kCategoryLabels[i]) +  "\n";
      row_2 += String(probabilities [i]);
    }
  }

  String table = row_1 + row_2;

  // Вернём строку на базе которой будет построена таблица.
  return table;
}




/** Функция возвращает строку на базе которой будет построена таблица с параметрами захвачеными с датчика, а также предсказанием цвета на базе этих параметров.
  Color DetectedColor[] - Структура содержащая  образец цвета полученный с датчика.
  Color ScaledDetectedColor[] - Структура данных для хранения отмасштабированого образца цвета полученного с датчика.
  String prediction - Наименование предсказаного цвета.**/
String getSensorTable(Color DetectedColor[], Color ScaledDetectedColor[], String prediction){
// Создадим строку на базе которой будет построена таблица с параметрами захвачеными с датчика, а также предсказанием цвета на базе этих параметров.
String table = String("DET_COLOR, RED, GREEN, BLUE, WHITE \n") +
                 "Sensor: " + prediction + ", " +
                 String(DetectedColor[0].r) + ", " +
                 String(DetectedColor[0].g) + ", " +
                 String(DetectedColor[0].b) + ", " +
                 String(DetectedColor[0].w) + "\n" +
                 "FLOAT: " + prediction + ", " +
                 String(ScaledDetectedColor[0].r) + ", " +
                 String(ScaledDetectedColor[0].g) + ", " +
                 String(ScaledDetectedColor[0].b) + ", " +
                 String(ScaledDetectedColor[0].w) + "\n" +
                 "INT8: " + prediction + ", " +
                 String((int8_t)(ScaledDetectedColor[0].r * 127.5)) + ", " +
                 String((int8_t)(ScaledDetectedColor[0].g * 127.5)) + ", " +
                 String((int8_t)(ScaledDetectedColor[0].b * 127.5)) + ", " +
                 String((int8_t)(ScaledDetectedColor[0].w * 127.5));

  // Вернём строку на базе которой будет построена таблица.
  return table;
}