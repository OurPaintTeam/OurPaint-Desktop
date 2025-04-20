#include "QTPainter.h"

QTPainter::QTPainter(QWidget *parent) : QFrame(parent), IDmove(0), id(0), circleStorage(nullptr),
                                        sectionStorage(nullptr), pointStorage(nullptr) {

    if (parent) {
        // Получение сигналов
        resize(parentWidget()->size()); // Берем размер от отца

        // Ловим сигнал изменения размера окна
        connect(parent->window(), SIGNAL(resize()), this, SLOT(onWorkWindowResized()));

        // Ловим сигналы от рисования мышкой
        connect(&drawFigM, &DrawMouse::SigPoint, this, &QTPainter::onSigPoint);
        connect(&drawFigM, &DrawMouse::SigSection, this, &QTPainter::onSigSection);
        connect(&drawFigM, &DrawMouse::SigCircle, this, &QTPainter::onSigCircle);
    }

    // Установим стили
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet("background: \"#ffffff\"");
    setAttribute(Qt::WA_AcceptTouchEvents);

    // Базовые параметры
    Scaling::updateScaling();
    Scaling::setStartMonitorSize((short int) width(), (short int) height());
}


// Обновляет экран отрисовки
void QTPainter::draw() {
    update();
}


// Очистка экрана и приводит все параметры к базовым
void QTPainter::clear() {
    id = 0;
    IDmove = 0;
    selectedClear();
    LeftMenuElem.clear();
    Scaling::setZoomZero();
    DrawAdditionalInf::setID(0);
    DrawAdditionalInf::setLeftMenuID(0);

    circleStorage = nullptr;
    pointStorage = nullptr;
    sectionStorage = nullptr;

    SelectedRectangle.clear();
}


// Удаление данных о фигурах
void QTPainter::figureDelete() {
    points.clear();
    circles.clear();
    sections.clear();
}


// Выделение обьекта в левом меню
void QTPainter::selectedElemByID(std::vector<double> &parameters, unsigned long long int IDselected) {
    if (!LeftMenuElem.empty()) {
        LeftMenuElem.clear();
    }

    if (parameters.size() > 4 || parameters.size() == 0 || IDselected == 0) {
        return;
    }

    LeftMenuElem = parameters;
    DrawAdditionalInf::setLeftMenuID(IDselected);
}


// Присвоение айди выделенной фигуры
void QTPainter::setIdFigures(ID Id) {
    if (Id.id > 0) {
        this->id = Id;
    }
}


// Получение айди фигуры перемещения
ID QTPainter::getIdFigures() {
    return IDmove;
}


// Получение вектора айди для требований
std::vector<ID> QTPainter::getVecID() {
    std::vector<ID> vec_id; // Обьеденияем все в один вектор
    vec_id.insert(vec_id.end(), selectedIdPoint.begin(), selectedIdPoint.end());
    vec_id.insert(vec_id.end(), selectedIdCircle.begin(), selectedIdCircle.end());
    vec_id.insert(vec_id.end(), selectedIdSection.begin(), selectedIdSection.end());
    return vec_id;
}


// Очистка данных выделенных обьектов
void QTPainter::selectedClear() {
    selectedIdPoint.clear();
    selectedIdSection.clear();
    selectedIdCircle.clear();
}


// Вспомогательная функция
bool QTPainter::matchesLeftMenu(const std::vector<double> &menu, const std::vector<double> &coords) {
    const double eps = 1e-5;
    if (menu.size() != coords.size()) {
        return false;
    }
    for (size_t i = 0; i < menu.size(); ++i)
        if (std::abs(menu[i] - coords[i]) > eps)
            return false;
    return true;
}


// Поиск выделенных обьектов
bool QTPainter::checkSelection(Element type, std::vector<ID> &selected, const std::vector<double> &coords) {
    if (type == ET_POINT) { emit getIdFigure(type, coords[0],coords[1]); }
    else if (type == ET_SECTION) { emit getIdFigure(type, coords[0],coords[1],coords[2],coords[3]); }
    else { emit getIdFigure(type, coords[0],coords[1],coords[2]);  }

    if (std::find(selected.begin(), selected.end(), id) != selected.end()) {
        DrawAdditionalInf::setID(id.id);
        return true;
    }
    return false;
}


// Отрисовка фигур
void QTPainter::drawingFigures(QPainter &painter) {

    // Обработка точек
    if (pointStorage != nullptr && !pointStorage->empty()) {
        if (selectedIdPoint.empty() && LeftMenuElem.size() != 2) {
            DrawFigures::drawPoint(painter, *pointStorage);
        } else {
            for (const auto &pt: *pointStorage) {
                bool isSelected = checkSelection(ET_POINT, selectedIdPoint, {pt.x, pt.y});
                if (matchesLeftMenu(LeftMenuElem, {pt.x, pt.y})) {
                    DrawFigures::setPen(QColor(Qt::red));
                }
                DrawFigures::drawPoint(painter, QPointF(pt.x, pt.y), isSelected);
            }
        }
    }

    // Обработка отрезков
    if (sectionStorage != nullptr && !sectionStorage->empty()) {
        if (selectedIdSection.empty() && LeftMenuElem.size() != 4) {
            DrawFigures::drawSection(painter, *sectionStorage);
        } else {
            for (const auto &sec: *sectionStorage) {
                bool isSelected = checkSelection(ET_SECTION, selectedIdSection,
                                                 {sec.beg->x, sec.beg->y, sec.end->x, sec.end->y});
                if (matchesLeftMenu(LeftMenuElem, {sec.beg->x, sec.beg->y, sec.end->x, sec.end->y})) {
                    DrawFigures::setPen(QColor(Qt::red));
                }
                DrawFigures::drawSection(painter, QPointF(sec.beg->x, sec.beg->y), QPointF(sec.end->x, sec.end->y),
                                         isSelected);
            }
        }
    }

    // Обработка кругов
    if (circleStorage != nullptr && !circleStorage->empty()) {
        if (selectedIdCircle.empty() && LeftMenuElem.size() != 3) {
            DrawFigures::drawCircle(painter, *circleStorage);
        } else {
            for (const auto &c: *circleStorage) {
                bool isSelected = checkSelection(ET_CIRCLE, selectedIdCircle, {c.center->x, c.center->y, c.R});
                if (matchesLeftMenu(LeftMenuElem, {c.center->x, c.center->y, c.R})) {
                    DrawFigures::setPen(QColor(Qt::red));
                }
                DrawFigures::drawCircle(painter, QPointF(c.center->x, c.center->y), c.R, isSelected);
            }
        }
    }
}


// Функция проверки курсора мышки и обьекта
bool QTPainter::findClosesObject() {
    bool leftClick = ModeManager::getActiveMode(MouseMode::LeftClick);
    bool doubleClick = ModeManager::getActiveMode(MouseMode::DoubleClickLeft);

    if (!leftClick && !doubleClick) {
        return true;
    }

    auto currentTime = std::chrono::steady_clock::now();
    if (currentTime - lastClickTime < std::chrono::milliseconds(300)) {
        return true;
    }

    lastClickTime = currentTime;

    bool objectFound = false;


    // Шаблонная обработка выбора
    auto processSelection = [this, &objectFound](auto *storage, auto checkFunc, auto emitFunc, auto &selectionList) {
        if (!storage || storage->empty() || objectFound) {
            return;
        }

        for (const auto &item: *storage) {
            if (checkFunc(item)) {
                emitFunc(item);
                objectFound = true;

                bool isSelected = std::find(selectionList.begin(), selectionList.end(), id) != selectionList.end();

                if (!ModeManager::getActiveMode(KeyMode::Shift)) {
                    selectedClear();
                    if (id.id > 0) selectionList.push_back(id);
                } else {
                    if (isSelected) {
                        selectionList.erase(std::remove(selectionList.begin(), selectionList.end(), id),
                                            selectionList.end());
                    } else if (id.id > 0) {
                        selectionList.push_back(id);
                    }
                }
                break;
            }
        }
    };

    // Обработка точек
    processSelection(pointStorage,
                     [](const Point &pt) {
                         return ClosesPoint::checkFigure(pt.x, pt.y);
                     },
                     [this](const Point &pt) { emit getIdFigure(ET_POINT, pt.x, pt.y); }, selectedIdPoint);

    // Обработка отрезков
    processSelection(sectionStorage,
                     [](const Section &sec) {
                         return ClosesPoint::checkFigure(sec.beg->x, sec.beg->y, sec.end->x, sec.end->y);
                     },
                     [this](const Section &sec) {
                         emit getIdFigure(ET_SECTION, sec.beg->x, sec.beg->y, sec.end->x, sec.end->y);
                     }, selectedIdSection);

    // Обработка кругов
    processSelection(circleStorage,
                     [](const Circle &c) {
                         return ClosesPoint::checkFigure(c.center->x, c.center->y, c.R);
                     },
                     [this](const Circle &c) {
                         emit getIdFigure(ET_CIRCLE, c.center->x, c.center->y, c.R);
                     }, selectedIdCircle);

    if (!objectFound) {
        selectedClear();
        return false;
    }

    return true;
}

// Сохранение в файл
void QTPainter::saveToImage(const QString &fileName, QString &format) {

    resizeRectangle();

    if (format.startsWith('.'))
        format = format.mid(1);

    QString originalFormat = format.toLower();
    QString chosenFormat = originalFormat;

    if (originalFormat == "jpg" || originalFormat == "jpeg") {
        chosenFormat = "JPEG";
    } else if (originalFormat == "png") {
        chosenFormat = "PNG";
    } else if (originalFormat == "bmp") {
        chosenFormat = "BMP";
    } else if (originalFormat == "tiff" || originalFormat == "tif") {
        chosenFormat = "TIFF";
    } else if (originalFormat == "pdf") {
        chosenFormat = "PDF";
    } else if (originalFormat == "svg") {
        chosenFormat = "SVG";
    } else {
        QMessageBox::warning(this, tr("Ошибка"), tr("Неподдерживаемый формат: %1").arg(format));
        return;
    }

    QString filePath = fileName;

    if (!filePath.endsWith("." + originalFormat, Qt::CaseInsensitive)) {
        filePath += "." + originalFormat;
    }

    if (chosenFormat == "SVG") {
        QSvgGenerator generator;
        generator.setFileName(filePath);
        generator.setSize(this->size());
        generator.setViewBox(QRect(0, 0, this->width(), this->height()));
        generator.setTitle("Exported SVG");
        generator.setDescription("Generated by QTPainter");

        QPainter painter(&generator);

        this->render(&painter);

        painter.end();
    } else {
        QPixmap pixmap = grab();

        if (chosenFormat == "PDF") {
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly)) {
                QPdfWriter writer(&file);
                writer.setPageSize(QPageSize(QPageSize::A4));
                writer.setResolution(300);

                QPainter pdfPainter(&writer);

                QPixmap pixmap = this->grab(this->rect());
                if (!pixmap.isNull()) {
                    QSize targetSize(writer.width(), writer.height());
                    QPixmap scaledPixmap = pixmap.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    pdfPainter.drawPixmap(0, 0, scaledPixmap);
                } else {
                    qDebug() << "Pixmap is null!";
                }

                pdfPainter.end();
                file.close();
            } else {
                QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось открыть PDF-файл для записи."));
            }
        } else {
            if (!pixmap.save(filePath, chosenFormat.toUtf8().constData())) {
                QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось сохранить изображение в формате %1").arg(chosenFormat));
            }
        }
    }
}


// Функция для изменения размеров окна
void QTPainter::resizeEvent(QResizeEvent *event) {
    QFrame::resizeEvent(event);
    Scaling::setActualMonitorSize(width(), height());
    update();
}


// Стартовая точка для отрисовки всего холста
void QTPainter::paintEvent(QPaintEvent *event) {

    // Инициализация QPainter для текущего виджета
    QPainter painter(this);

    // Если не активен то не рисуем
    if (!painter.isActive()) {
        return;
    }

    //  Включение сглаживания для графических примитивов
    painter.setRenderHint(QPainter::Antialiasing, true);


    // Переносим на центр экрана
    painter.translate((int) (width() / 2 + Scaling::getDeltaX()), (int) (height() / 2 + Scaling::getDeltaY()));

    // Отрисовка фона
    DrawBackground::drawFon(painter);

    // Если курсор в области то рисуем его значения
    if (ModeManager::getCursor()) {
        DrawAdditionalInf::drawCursor(painter);
    }


    // Выделение обьекта левого меню
    if (!LeftMenuElem.empty() &&
        ModeManager::getActiveMode(MouseMode::LeftClick) || ModeManager::getActiveMode(MouseMode::RightClick)) {
        LeftMenuElem.clear();
    } else if (LeftMenuElem.size() > 4) {
        LeftMenuElem.clear();
        DrawAdditionalInf::setLeftMenuID(0);
    }



    /*************** Отрисовка фигур мышкой ********************/

    // Чтобы не рисовать много точек перемещая мышь и удерживая кнопку
    // Рисуем одну точку одним нажатием

    if (ModeManager::getCursor()) {
        if (!(ModeManager::getActiveMode(MouseMode::MouseMove)) &&
            ModeManager::getActiveMode(WorkModes::Point)) {

            drawFigM.DrawFiguresMouse(painter);

        } else if (ModeManager::getActiveMode(WorkModes::Circle)) {
            drawFigM.DrawFiguresMouse(painter);
        } else if (ModeManager::getActiveMode(WorkModes::Section)) {

            drawFigM.DrawFiguresMouse(painter);

            if (pointStorage != nullptr && pointStorage->size() > 0) {
                QPointF closes = ClosesPoint::findClosestPoint(*pointStorage); // Ищем ближайшие точки
                drawFigM.drawHints(painter, closes);
            }
        }

    }



    /****************** Функции выделения и перемещения  ********************/


    // Выделение
    if (ModeManager::getActiveMode(WorkModes::Editor)) {
        if (ModeManager::getActiveMode(MouseMode::LeftClick)) {
            if (circleStorage != nullptr && circleStorage->size() > 0 ||
                sectionStorage != nullptr && sectionStorage->size() > 0 ||
                pointStorage != nullptr && pointStorage->size() > 0)
                findClosesObject();
                 IDmove=0;
        } else if (ModeManager::getActiveMode(MouseMode::DoubleClickLeft)) {
            id = 0;
            IDmove=0;
            if (circleStorage != nullptr && circleStorage->size() > 0 ||
                sectionStorage != nullptr && sectionStorage->size() > 0 ||
                pointStorage != nullptr && pointStorage->size() > 0)
                findClosesObject();

            if (id != 0) {
                emit DoubleClickOnObject(id);
            }

        }
    }else if(ModeManager::getActiveMode(WorkModes::Selected)){
        QRectF rect=SelectedRectangle.selected(painter);

    }  else if (ModeManager::getActiveMode(WorkModes::Move)) {

        //  Кнопка мыши зажата
        bool isDragging = ModeManager::getActiveMode(MouseMode::LeftClick) &&
                          !ModeManager::getActiveMode(MouseMode::ReleasingLeft);

        if (isDragging) {
            //  Уже что-то выбрано для перемещения
            if (IDmove != 0) {
                    emit MovingFigures();
            } else {
                // Поиск
                if (findClosesObject()) {
                    IDmove = id;

                }
            }
        } else {
            IDmove = 0; // сброс при отпускании
        }
    } else {
        // сброс
        id = 0;
        IDmove = 0;
    }




    // Вызов функции рисования
    drawingFigures(painter);



   // figureDelete();

    QFrame::paintEvent(event);
}


// Изменение размера окна
void QTPainter::onWorkWindowResized() {
    // При изменении размера окна родителя меняем размер
    Scaling::setActualMonitorSize(parentWidget()->width(), parentWidget()->height());
    resize(parentWidget()->size());
}
void QTPainter::resizeRectangle() {

}



