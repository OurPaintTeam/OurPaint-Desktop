# OurPaintDCM
Открытый 2D-решатель геометрических ограничений для эскизов CAD. Поддерживает точки, линии, окружности и дуги и ограничения над ними: расстояния, параллельность, перпендикулярность и др. Реализован на C++ с использованием Eigen, рассчитан на работу в реальном времени для параметрического CAD.

## Установка 
Выполняем следующие действия
```bash
git clone --recurse-submodules https://github.com/OurPaintTeam/OurPaintDCM.git
cd OurPaintDCM
mkdir build
cd build
cmake ..
cmake --build . --config Release
```
Запуск тестов
```bash
cd build
ctest
```