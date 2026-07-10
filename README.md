# OurPaint Desktop

OurPaint Desktop — настольное приложение на C++ и Qt 6 для работы с двумерной геометрией. Приложение позволяет создавать и редактировать геометрические примитивы, отображать сцену через OpenGL и работать с ограничениями между объектами.

Актуальная рабочая ветка: `dev`.

## Возможности

- создание точек, линий, окружностей, дуг и кривых Безье;
- построение примитивов мышью;
- отображение сцены через OpenGL;
- выбор объектов на сцене;
- работа с геометрическими ограничениями;
- сохранение и загрузка данных проекта;
- использование шейдеров и графических ресурсов;
- модульные тесты для части логики проекта;
- сборка через CMake.

## Технологии

- C++20
- CMake
- Ninja
- Qt 6
- OpenGL
- Eigen
- nlohmann/json
- GoogleTest

## Зависимости

Для Ubuntu/Debian:

```bash
sudo apt update
sudo apt install -y git cmake ninja-build build-essential pkg-config \
  qt6-base-dev qt6-base-dev-tools qt6-svg-dev libqt6opengl6-dev \
  libgl1-mesa-dev libxkbcommon-dev libxcb-cursor0
```

Для Windows необходимо установить CMake, Qt 6 с компонентами для выбранного компилятора, а также Visual Studio с набором инструментов C++ ЛИБО MinGW-w64.

## Клонирование

```bash
git clone https://github.com/OurPaintTeam/OurPaint-Desktop.git
cd OurPaint-Desktop
git switch dev
```

## Подготовка подмодулей

```bash
git submodule update --init --recursive
```
## Сборка

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target ourpaint --parallel
```

## Если шейдеры не найдены!

Скопируйте ресурсы вручную:

```bash
mkdir -p build/src/app/shaders build/src/app/fonts

cp -a src/rendering/renderer/shaders/. build/src/app/shaders/
cp -a src/Rendering/renderer/shaders/. build/src/app/shaders/
cp -a src/Rendering/renderer/fonts/. build/src/app/fonts/
```

После этого снова запустите
