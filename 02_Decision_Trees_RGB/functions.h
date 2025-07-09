// Структура данных для хранения образцов цвета полученных с датчика.
struct Color {
  // Наименование цвета.
  String name;
  // Значения компонент цвета.
  int RED, GREEN, BLUE, WHITE;
};


/** Реализлвана функция для захвата цвета с матрицы датчика. 
  Color DetectedColor[] - Структура данных для хранения образца цвета полученного с датчика.**/
void colorCapture(Color DetectedColor[]){

  // Состояния пинов S2 и S3 определяют, какой набор фотодиодов мы используем: LOW/LOW — для КРАСНОГО.
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  // Получаем значение для красной компоненты цвета.
  DetectedColor[0].RED = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  delay(20);

  // Меняем состояния пинов S2 и S3 на LOW/HIGH для голубой компоненты цвета.
  digitalWrite(s3, HIGH); 
  // Получаем значение для голубой компоненты цвета.
  DetectedColor[0].BLUE = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  delay(20);

  // Меняем состояния пинов S2 и S3 на HIGH/HIGH для зелёной компоненты цвета.
  digitalWrite(s2, HIGH);
  // Получаем значение для зелёной компоненты цвета.
  DetectedColor[0].GREEN = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  delay(20);

  // Меняем состояния пинов S2 и S3 на HIGH/LOW для белой компоненты цвета.
  digitalWrite(s3, LOW);
  // Получаем значение для белой компоненты цвета.
  DetectedColor[0].WHITE = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  delay(20);
}




String color_classify(Color DetectedColor[]) {
  if (DetectedColor[0].BLUE > 14.5) {
    if (DetectedColor[0].BLUE > 17.5) {
      if (DetectedColor[0].RED > 21) {
        if (DetectedColor[0].RED > 10.5) {
          return " Black";  // leaf 0
        } else {
          return "Unknown";  // leaf 1
        }
      } else {
        if (DetectedColor[0].RED > 10.5) {
          return " Green";  // leaf 2
        } else {
          return " Red";  // leaf 3
        }
      }
    } else {
      if (DetectedColor[0].RED > 21) {
        if (DetectedColor[0].RED > 10.5) {
          return "Unknown";  // leaf 4
        } else {
          return "Unknown";  // leaf 5
        }
      } else {
        if (DetectedColor[0].RED > 10.5) {
          return "Unknown";  // leaf 6
        } else {
          return " Yellow";  // leaf 7
        }
      }
    }
  } else {
    if (DetectedColor[0].BLUE > 17.5) {
      if (DetectedColor[0].RED > 21) {
        if (DetectedColor[0].RED > 10.5) {
          return "Unknown";  // leaf 8
        } else {
          return "Unknown";  // leaf 9
        }
      } else {
        if (DetectedColor[0].RED > 10.5) {
          return "Unknown";  // leaf 10
        } else {
          return "Unknown";  // leaf 11
        }
      }
    } else {
      if (DetectedColor[0].RED > 21) {
        if (DetectedColor[0].RED > 10.5) {
          return " Blue";  // leaf 12
        } else {
          return "Unknown";  // leaf 13
        }
      } else {
        if (DetectedColor[0].RED > 10.5) {
          return " Purple";  // leaf 14
        } else {
          return " White";  // leaf 15
        }
      }
    }
  }
}


/** Реализлвана функция для классификации цвета при помощи решающего дерева. **
String color_classify(Color DetectedColor[]){
  if(DetectedColor[0].r>14.5){
    if(DetectedColor[0].b>15.5){
        if(DetectedColor[0].b>18.5){
            if(DetectedColor[0].b>16.5){
                if(DetectedColor[0].r>51){
                  if(DetectedColor[0].r>23.5){
                    //Serial.println("Black");
                    return "BLACK";
                  }else{
                    //Serial.println("NO COLOR!");
                    return "Unknown";
                  }
                }else{
                  if(DetectedColor[0].r>23.5){
                    //Serial.println("Black");
                    return "BLACK";
                  }else{
                    //Serial.println("GREEN");
                    return "GREEN";
                  }
                }
            }else{
              //Serial.println("NO COLOR!");
              return "Unknown";
            }
        }else{
          //Serial.println("NO COLOR!");
          return "Unknown";
        }
    }else{ 
      if(DetectedColor[0].b>18.5){
        //Serial.println("NO COLOR!");
        return "Unknown";
      }else{
        if(DetectedColor[0].b>16.5){
          //Serial.println("NO COLOR!");
          return "Unknown";
        }else{
          if(DetectedColor[0].r>51){
            //Serial.println("NO COLOR!");
            return "Unknown";
          }else{
            if(DetectedColor[0].r>23.5){
              //Serial.println("BLUE");
              return "BLUE";
            }else{
              //Serial.println("BLUE OR Purple");
              return "PURPLE";
            }
          }
        }
      }
    }
  }else{
    if(DetectedColor[0].b>15.5){
      if(DetectedColor[0].b>18.5){
        if(DetectedColor[0].b>16.5){
          if(DetectedColor[0].r>51){
            //Serial.println("NO COLOR!");
            return "Unknown";
          }else{
            if(DetectedColor[0].r>23.5){
              //Serial.println("NO COLOR!");
              return "Unknown";
            }else{
              //Serial.println("RED");
              return "RED";
            }
          }
        }else{
          //Serial.println("NO COLOR!");
          return "Unknown";
        }
      }else{
        if(DetectedColor[0].b>16.5){
          if(DetectedColor[0].r>51){
            //Serial.println("NO COLOR!");
            return "Unknown";
          }else{
            if(DetectedColor[0].r>23.5){
              //Serial.println("NO COLOR!");
              return "Unknown";
            }else{
              //Serial.println("Orange");
              return "ORANGE";
            }
          }
        }else{
          if(DetectedColor[0].r>51){
            //Serial.println("NO COLOR!");
            return "Unknown";
          }else{
            if(DetectedColor[0].r>23.5){
              //Serial.println("NO COLOR!");
              return "Unknown";
            }else{
              //Serial.println("YELLOW");
              return "YELLOW";
            }
          }
        }
      }
    }else{
      if(DetectedColor[0].b>18.5){
        //Serial.println("NO COLOR!");
        return "Unknown";
      }else{
        if(DetectedColor[0].b>16.5){
          //Serial.println("NO COLOR!");
          return "Unknown";
        }else{
          if(DetectedColor[0].r>51){
            //Serial.println("NO COLOR!");
            return "Unknown";
          }else{
            if(DetectedColor[0].r>23.5){
              //Serial.println("NO COLOR!");
              return "Unknown";
            }else{
              //Serial.println("WHITE");
              return "WHITE";
            }
          }
        }
      }
    }
  }
}




/**
  if(...){
    ...
  }else{
    ...
  }
**/ 