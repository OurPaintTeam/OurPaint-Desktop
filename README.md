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

## Зависимости Linux

Для Ubuntu/Debian:

```bash
sudo apt update
sudo apt install -y git cmake ninja-build build-essential pkg-config \
  qt6-base-dev qt6-base-dev-tools qt6-svg-dev libqt6opengl6-dev \
  libgl1-mesa-dev libxkbcommon-dev libxcb-cursor0
```

Рекомендуется CMake версии `3.26` или новее.

## Клонирование

```bash
git clone https://github.com/OurPaintTeam/OurPaint-Desktop.git
cd OurPaint-Desktop
git switch dev
```

## Подготовка подмодулей

```bash
git config submodule.src/UI.url https://github.com/OurPaintTeam/OurPaint-Desktop-UI.git

git submodule update --init --recursive src/UI

rm -rf src/core/DCM .git/modules/src/core/DCM
git clone https://github.com/OurPaintTeam/OurPaintDCM.git src/core/DCM
git -C src/core/DCM checkout main
git -C src/core/DCM submodule update --init --recursive
```

## Сборка

```bash
rm -rf build

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target ourpaint --parallel
```

## Запуск
```bash
cd build/src/app
./ourpaint
```

## Если шейдеры не найдены

Скопируйте ресурсы вручную:

```bash
mkdir -p build/src/app/shaders build/src/app/fonts

cp -a src/rendering/renderer/shaders/. build/src/app/shaders/
cp -a src/Rendering/renderer/shaders/. build/src/app/shaders/
cp -a src/Rendering/renderer/fonts/. build/src/app/fonts/
```

После этого снова запустите:

```bash
cd build/src/app
./ourpaint
```

## Текущее состояние

Ветка `dev` находится в активной разработке. На Linux возможны проблемы из-за различий в регистре путей и Windows-ориентированных скриптов копирования ресурсов. Если сборка падает, в первую очередь нужно проверить пути `src/rendering` и `src/Rendering`, а также наличие шейдеров в `build/src/app/shaders`.