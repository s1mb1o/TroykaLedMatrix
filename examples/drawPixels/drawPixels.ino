// библиотека для работы I²C
#include "Wire.h"
// библиотека для работы со светодиодной матрицей
#include "TroykaLedMatrix.h"

// создаём объект matrix для работы с матрицей
TroykaLedMatrix matrix;

void setup()
{
  // начало работы с матрицей
  matrix.begin();
  // очищаем матрицу
  matrix.clear();
}

void loop()
{
  // заполняем матрицу случайным пикселем
  matrix.drawPixel(random(8),random(8));
  // очищаем на матрице случайный пиксель
  matrix.clearPixel(random(8),random(8));
  // ждём 50 мс
  delay(50);
}
