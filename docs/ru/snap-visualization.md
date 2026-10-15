# Визуализация Snap

В App реализован отдельный канал временных подсказок: `app::SnapFeedback` → `OverlayModel` → `RenderSceneBuilder` → обычные примитивы `RenderScene`. Он показывает выбранную позицию, символ, исходную геометрию и направляющие. Вычисление кандидатов, выбор привязки и интеграция в инструменты остаются задачей producer — кода, который публикует подсказку.

Реализация находится в [SnapFeedback.h](../../src/app/viewport/SnapFeedback.h), [OverlayModel.h](../../src/app/viewport/OverlayModel.h), [RenderSceneBuilder.cc](../../src/app/viewport/render/RenderSceneBuilder.cc) и [ViewportStyle.h](../../src/app/viewport/ViewportStyle.h). Она не зависит от типов `editor/snap` или `SnapKind` из другой ветки.

## Контракт данных

```cpp
namespace app {

enum class SnapGlyph { Point, Endpoint, Midpoint, Center, Intersection, Direction };
enum class SnapGuideExtent { Segment, InfiniteLine };

struct SnapGuide {
    core::sketch::Line2 line;
    SnapGuideExtent extent = SnapGuideExtent::Segment;
};

struct SnapFeedback {
    core::sketch::Vec2 position;
    SnapGlyph glyph = SnapGlyph::Point;
    std::vector<core::sketch::GeometryRef> sources;
    std::vector<SnapGuide> guides;
};

}  // namespace app
```

`position` и концы `line` заданы в координатах эскиза, в тех же единицах, что геометрия текущего `Sketch`. Для `InfiniteLine` две различные точки определяют опорную прямую; builder обрезает её границами viewport. Для `Segment` рисуется конечный отрезок. Producer передаёт конечные числовые координаты и ненулевое направление.

`sources` содержит публичные ссылки на сущности текущего эскиза: `Whole` подсвечивает исходную геометрию, `Start`/`End`/`Center` — соответствующий точечный элемент. Для середины отрезка источником служит `Whole` линии, а вычисленная середина хранится в `position`. Создавать фиктивный `EntityId` или точечную сущность для такого site не требуется.

`SnapFeedback` владеет значениями: контейнеры запроса или временные результаты геометрического поиска могут быть уничтожены после публикации. `GeometryRef` не владеет сущностью и не делает её координаты неизменяемыми. Подсветка источника читается из текущего `Sketch`, поэтому producer отвечает за согласованность позиции и ссылок.

Символы различаются формой, поэтому смысл подсказки виден и без различения цветов:

| `SnapGlyph` | Форма |
| --- | --- |
| `Point` | Плюс |
| `Endpoint` | Квадрат |
| `Midpoint` | Треугольник |
| `Center` | Кольцо |
| `Intersection` | Диагональный крест |
| `Direction` | Ромб; направление показывает guide |

## Публикация

Producer создаёт полное новое значение и передаёт его через `setSnapFeedback`. Повторный вызов заменяет предыдущую подсказку целиком, включая `sources` и `guides`.

```cpp
#include <numeric>
#include <utility>

#include "OverlayModel.h"

namespace sketch = core::sketch;

void showMidpoint(OverlayModel& overlay, sketch::EntityId lineId, const sketch::Line2& line) {
    app::SnapFeedback feedback;
    feedback.position = {std::midpoint(line.start.x, line.end.x), std::midpoint(line.start.y, line.end.y)};
    feedback.glyph = app::SnapGlyph::Midpoint;
    feedback.sources = {{lineId, sketch::SubElement::Whole}};
    overlay.setSnapFeedback(std::move(feedback));
}
```

Здесь `lineId` и `line` должны относиться к одной актуальной сущности текущего эскиза. Публикация показывает середину без `Sketch::addPoint`, изменения selection или записи в массивы preview. Выбор этой середины и передача её инструменту выполняются отдельно.

Для уже найденного пересечения передаются обе исходные кривые:

```cpp
void showIntersection(OverlayModel& overlay, sketch::Vec2 intersection,
                      sketch::EntityId firstCurve, sketch::EntityId secondCurve) {
    app::SnapFeedback feedback;
    feedback.position = intersection;
    feedback.glyph = app::SnapGlyph::Intersection;
    feedback.sources = {{firstCurve, sketch::SubElement::Whole},
                        {secondCurve, sketch::SubElement::Whole}};
    overlay.setSnapFeedback(std::move(feedback));
}
```

Направляющая через опорную точку может быть бесконечной. В примере producer уже вычислил допустимую горизонтальную позицию `resolved` с той же координатой Y, что у `anchor`:

```cpp
void showHorizontalGuide(OverlayModel& overlay, sketch::Vec2 anchor, sketch::Vec2 resolved) {
    app::SnapFeedback feedback;
    feedback.position = resolved;
    feedback.glyph = app::SnapGlyph::Direction;
    feedback.guides.push_back({sketch::Line2{anchor, {anchor.x + 1.0, anchor.y}},
                              app::SnapGuideExtent::InfiniteLine});
    overlay.setSnapFeedback(std::move(feedback));
}
```

Вместо очень длинного отрезка передаются две точки опорной прямой. Для подсказки продления от конца исходного отрезка до пересечения используется `SnapGuideExtent::Segment`.

Доступ к состоянию — `const std::optional<app::SnapFeedback>& OverlayModel::snapFeedback() const noexcept`. Отсутствие предложения обозначается `overlay.clearSnap()`, а не значением `SnapFeedback{}`: у опубликованного значения всегда есть целевая позиция.

## Жизненный цикл и перерисовка

В этом этапе инструменты и контроллер не публикуют подсказки автоматически. После `setSnapFeedback` или `clearSnap` текущий владелец interaction должен запросить перерисовку своим существующим путём. `RenderSceneBuilder::rebuild()` читает состояние и заново формирует слои; setter не вычисляет snap, не меняет геометрию и сам не запрашивает redraw.

Producer должен:

1. Публиковать результат свежего запроса для текущего инструмента и текущего эскиза. Принадлежность `EntityId` конкретному эскизу не проверяется каналом визуализации: в разных эскизах одинаковые ID могут обозначать разные сущности.
2. Пересчитывать или очищать подсказку при изменении модели, допустимых источников, шага инструмента, модификаторов и камеры. В частности, redraw после zoom обновляет экранную геометрию, но не проверяет допустимость старого snap.
3. Очищать подсказку при отсутствии кандидата, отмене/завершении ввода, смене инструмента или документа, подавлении snap и уходе указателя, если interaction явно не удерживает подсказку.
4. Не использовать показанную подсказку как доказательство постоянного ограничения. Совпадение координат и символ направления не создают `Coincident`, `Horizontal`, `Parallel` или другие constraints.

Builder пропускает недоступные ссылки и некорректные примитивы при формировании изображения. Это защитная обработка presentation; она не заменяет проверку свежести запроса producer и не связывает сохранённую позицию с обновлённой геометрией.

| Вызов | Preview | Пользовательская selection | Snap feedback |
| --- | --- | --- | --- |
| `clearPreview()` | Очищает | Сохраняет | Сохраняет |
| `clearSelection()` | Сохраняет | Очищает | Сохраняет |
| `clearSnap()` | Сохраняет | Сохраняет | Очищает |
| `clear()` | Очищает | Очищает | Очищает |

## Стили и единицы экрана

`ViewportStyle::snapFeedback` содержит `marker` (`StrokeStyle`), `referencePoint` (`MarkerStyle`), `referenceCurve` и `guide` (`StrokeStyle`), а также `glyphSizePx` (`double`, по умолчанию `10.0`). Размер символа, радиус точечных маркеров и толщины линий задаются в logical pixels. Они не масштабируются вместе с геометрией при zoom.

Позиции и направляющие producer оставляет в double-координатах эскиза. Builder выполняет проекцию и перевод в framebuffer pixels с учётом DPR; producer не умножает координаты или размеры на DPR. Символ располагается поверх preview, подсветка источников имеет собственный стиль. Эти данные не передают `SnapKind` или `GeometryRef` в renderer: builder преобразует их в обычные графические примитивы.

## Адаптер для ветки со SnapSystem

Адаптер находится у consumer той ветки и вручную переводит её результат в этот presentation API. Сам `SnapFeedback.h` не включает заголовки `SnapSystem` или его перечисления.

| Семейство результата той ветки | `app::SnapGlyph` |
| --- | --- |
| `StandalonePoint`, `LineBody`, `CircleBody`, `ArcBody` | `Point` |
| `LineEndpoint`, `ArcEndpoint` | `Endpoint` |
| `LineMidpoint` | `Midpoint` |
| `CircleCentre`, `ArcCentre` | `Center` |
| `Intersection`, `ExtendedIntersection`, `VirtualIntersection` | `Intersection` |
| `Horizontal`, `Vertical`, `Parallel`, `Perpendicular` | `Direction` |

После выбора glyph адаптер копирует `result.point.position` в `feedback.position`, `result.objects` в `feedback.sources` и преобразует каждую конечную `result.guideLines` в `SnapGuide{line, SnapGuideExtent::Segment}`. Бесконечность нельзя восстановить только по двум точкам: `InfiniteLine` выбирается явно, когда producer знает смысл направляющей. Если `result.snapped == false`, адаптер вызывает `clearSnap()`.

Наличие `Tangent`, `Angle` или другого значения в enum сторонней ветки само по себе не означает готовый алгоритм или готовую визуальную семантику. Такие результаты требуют отдельного осознанного отображения после реализации сценария. Подключение адаптера к инструментам, обработка input и принятие позиции не входят в текущую реализацию визуализации.
