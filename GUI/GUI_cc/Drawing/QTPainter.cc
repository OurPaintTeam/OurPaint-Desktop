#include "QTPainter.h"

QTPainter::QTPainter(QWidget *parent) : QFrame(parent) {
    //setAttribute(Qt::WA_OpaquePaintEvent); // ускоряет отрисовку
    if (parent) {
        // Получение сигналов
        resize(parentWidget()->size()); // Берем размер от отца

        // Ловим сигнал изменения размера окна
        connect(parent->window(), SIGNAL(resized()), this, SLOT(onWorkWindowResized()));

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
    IDmove = 0;
    id = 0;
    circleStorage = nullptr;
    pointStorage = nullptr;
    sectionStorage = nullptr;
}

void QTPainter::draw() // Обновляет экран отрисовки
{
    update();
}

// Очистка экрана и приводит все параметры к базовым
void QTPainter::clear() {
    Scaling::setZoomZero();
    selectedClear();
    LeftMenuElem.clear();
    DrawAdditionalInf::setLeftMenuID(0);
    DrawAdditionalInf::setID(0);
    id = 0;
}

// Удаление данных о фигурах
void QTPainter::figureDelete() {
    points.clear();
    circles.clear();
    sections.clear();
    //pointStorage= nullptr;
    //sectionStorage= nullptr;
    //circleStorage= nullptr;
    //  id=0;
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
std::vector <ID> QTPainter::getVecID() {
    std::vector <ID> vec_id; // Обьеденияем все в один вектор
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


void QTPainter::drawingFigures(QPainter &painter) {

    // Обработка точек
    if (pointStorage != NULL) {
        if (pointStorage->size() > 0) {

            for (auto &pt: *pointStorage) {
                bool isSelected = false;

                if (selectedIdPoint.empty() && LeftMenuElem.size() != 2) {
                    DrawFigures::drawPoint(painter, *pointStorage);
                    break;
                }

                // Если вектор не пуст мы ищем нужный айди для выделения, чтобы получить координаты и сопоставить нужный ему ID
                if (!selectedIdPoint.empty()) {
                    emit getIdFigure(ET_POINT, pt.x, pt.y);
                    isSelected = std::find(selectedIdPoint.begin(), selectedIdPoint.end(), id) !=
                                 selectedIdPoint.end();


                    if (isSelected) {
                        DrawAdditionalInf::setID(id.id); // Отрисовка айди
                    }


                    if (LeftMenuElem.size() == 2) {
                        if (std::abs(pt.x - LeftMenuElem[0]) < 1e-5 &&
                            std::abs(pt.y - LeftMenuElem[1]) < 1e-5) {

                            DrawFigures::setPen(QColor(Qt::red));
                        }
                    }

                    DrawFigures::drawPoint(painter, QPointF(pt.x, pt.y), isSelected);

                }
            }
        }
    }


    // Аналогично отрезки
    if (sectionStorage != nullptr && sectionStorage->size() > 0)
        for (auto &sec: *sectionStorage) {

            if (selectedIdSection.empty() && LeftMenuElem.size() != 4) {
                DrawFigures::drawSection(painter, *sectionStorage);

                break;
            }

            bool isSelected = false;

            if (!selectedIdSection.empty()) {
                emit getIdFigure(ET_SECTION, sec.beg->x, sec.beg->y, sec.end->x, sec.end->y);
                isSelected = std::find(selectedIdSection.begin(), selectedIdSection.end(), id) !=
                             selectedIdSection.end();


                if (isSelected) {
                    DrawAdditionalInf::setID(id.id); // Отрисовка айди
                }
            }

            if (LeftMenuElem.size() == 4) {
                if (std::abs(sec.beg->x - LeftMenuElem[0]) < 1e-3 &&
                    std::abs(sec.beg->y - LeftMenuElem[1]) < 1e-3 &&
                    std::abs(sec.end->x - LeftMenuElem[2]) < 1e-3 &&
                    std::abs(sec.end->y - LeftMenuElem[3]) < 1e-3) {
                    DrawFigures::setPen(QColor(Qt::red));
                }
            }

            DrawFigures::drawSection(painter, QPointF(sec.beg->x, sec.beg->y), QPointF(sec.end->x, sec.end->y),
                                     isSelected);

        }

    // Аналогично круги
    if (sectionStorage != nullptr && circleStorage->size() > 0)
        for (auto &c: *circleStorage) {

            bool isSelected = false;

            if (selectedIdCircle.empty() && LeftMenuElem.size() != 3) {
                DrawFigures::drawCircle(painter, *circleStorage);
                break;
            }

            if (!selectedIdCircle.empty()) {
                emit getIdFigure(ET_CIRCLE, c.center->x, c.center->y, c.R);
                isSelected = std::find(selectedIdCircle.begin(), selectedIdCircle.end(), id) !=
                             selectedIdCircle.end();
                if (isSelected) {
                    DrawAdditionalInf::setID(id.id); // Отрисовка айди
                }
            }

            if (LeftMenuElem.size() == 3) {
                if (std::abs(c.center->x - LeftMenuElem[0]) < 1e-5 &&
                    std::abs(c.center->y - LeftMenuElem[1]) < 1e-5
                    && std::abs(c.R - LeftMenuElem[2]) < 1e-5) {
                    DrawFigures::setPen(QColor(Qt::red));
                }
            }

            DrawFigures::drawCircle(painter, QPointF(c.center->x, c.center->y), c.R, isSelected);

        }
}

// Функция проверки курсора мышки и обьекта
bool QTPainter::findClosesObject() {

    // Используем только если было нажатие
    bool leftClick = ModeManager::getActiveMode(MouseMode::LeftClick);
    bool DoubleClick = ModeManager::getActiveMode(MouseMode::DoubleClickLeft);
    bool flag = true; // Если нашли нужный обьект к следующим не идем

    if (leftClick || DoubleClick) {

        // Так как работа идет real time замедляем процесс, чтобы обработать одно нажатие один раз
        auto currentTime = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastClickTime).count() >= 300) {

            bool Delete = true; // Если не находим нужный обьект(курсор был нажат в пустой области) - очищаем выделение

            // Обработка точек
            if (pointStorage != nullptr && pointStorage->size() > 0)
                for (auto &pt: *pointStorage) {

                    bool near = ClosesPoint::checkFigure(pt.x,
                                                         pt.y); // Функция сравнения координат курсора и обьекта

                    if (near) {
                        Delete = false;
                        flag = false;

                        lastClickTime = currentTime;
                        emit getIdFigure(ET_POINT, pt.x, pt.y); // Получем айди

                        bool isSelected =
                                std::find(selectedIdPoint.begin(), selectedIdPoint.end(), id) !=
                                selectedIdPoint.end();

                        // Если айди уже существует снимаем выделение иначе добавляем

                        if (!ModeManager::getActiveMode(KeyMode::Shift)) {
                            // Если не нажат шифт убираем остальные выделения
                            selectedClear();
                            if (id.id > 0) {
                                selectedIdPoint.push_back(id);
                            }
                        } else {
                            if (isSelected) {
                                // Снимаем выделение
                                selectedIdPoint.erase(
                                        std::remove(selectedIdPoint.begin(), selectedIdPoint.end(), id.id),
                                        selectedIdPoint.end());
                            } else {
                                // Добавляем в выделенные
                                if (id.id > 0) {
                                    selectedIdPoint.push_back(id);
                                }
                            }
                        }
                        break; // Закончили поиск
                    }


                }

            if (flag) {
                // Если не нашли аналогично обработаем отрезки
                if (sectionStorage != nullptr && sectionStorage->size() > 0)
                    for (auto &sec: *sectionStorage) {

                        bool near = ClosesPoint::checkFigure(sec.beg->x, sec.beg->y, sec.end->x, sec.end->y);

                        if (near) {
                            Delete = false;
                            flag = false;
                            lastClickTime = currentTime;
                            emit getIdFigure(ET_SECTION, sec.beg->x, sec.beg->y, sec.end->x, sec.end->y);

                            bool isSelected =
                                    std::find(selectedIdSection.begin(), selectedIdSection.end(), id) !=
                                    selectedIdSection.end();

                            if (!ModeManager::getActiveMode(KeyMode::Shift)) {
                                selectedClear();
                                if (id.id > 0) {
                                    selectedIdSection.push_back(id);
                                }
                            } else {
                                if (isSelected) {
                                    // Снимаем выделение
                                    selectedIdSection.erase(
                                            std::remove(selectedIdSection.begin(), selectedIdSection.end(), id),
                                            selectedIdSection.end());
                                } else {
                                    // Добавляем в выделенные
                                    if (id.id > 0) {
                                        selectedIdSection.push_back(id);
                                    }
                                }
                            }
                            break;
                        }

                    }
            }

            if (flag) {
                // Аналогично
                if (circleStorage != nullptr && circleStorage->size() > 0)
                    for (auto &c: *circleStorage) {
                        bool near = ClosesPoint::checkFigure(c.center->x, c.center->y, c.R);

                        if (near) {
                            Delete = false;
                            lastClickTime = currentTime;
                            emit getIdFigure(ET_CIRCLE, c.center->x, c.center->y, c.R);

                            bool isSelected =
                                    std::find(selectedIdCircle.begin(), selectedIdCircle.end(), id) !=
                                    selectedIdCircle.end();

                            if (!ModeManager::getActiveMode(KeyMode::Shift)) {
                                selectedClear();
                                if (id.id > 0)
                                    selectedIdCircle.push_back(id);
                            } else {
                                if (isSelected) {
                                    // Снимаем выделение
                                    selectedIdCircle.erase(
                                            std::remove(selectedIdSection.begin(), selectedIdCircle.end(), id),
                                            selectedIdCircle.end());
                                } else {
                                    // Добавляем в выделенные
                                    if (id.id > 0)
                                        selectedIdCircle.push_back(id);
                                }
                            }
                            break;
                        }
                    }
            }


            if (Delete) { // Очистка если нажали в пустую область
                selectedClear();
                return false;
            }

        }
    }
    return true;
}


void QTPainter::saveToImage(const QString &format) {
    QPixmap pixmap = grab();

    // Запросить путь для сохранения изображения
    QString filePath = QFileDialog::getSaveFileName(this, tr("Сохранить изображение"), "",
                                                    tr("Image Files (*.jpg *.jpeg *.png *.bmp *.pdf)"));

    if (!filePath.isEmpty()) {
        // Проверка формата в зависимости от заданного аргумента
        QStringList supportedFormats = QStringList() << "jpg" << "jpeg" << "png" << "bmp" << "pdf";

        QString chosenFormat = format.isEmpty() ? QFileInfo(filePath).suffix().toLower() : format.toLower();

        if (supportedFormats.contains(chosenFormat)) {
            if (chosenFormat == "pdf") {
                // Сохранение в формате PDF
                QFile file(filePath);
                if (file.open(QIODevice::WriteOnly)) {
                    QPdfWriter writer(&file);
                    writer.setPageSize(QPageSize(QPageSize::A4));
                    QPainter pdfPainter(&writer);
                    pdfPainter.drawPixmap(0, 0, pixmap); // Рисуем изображение на PDF
                    pdfPainter.end();
                    file.close();
                }
            } else {
                // Сохранить в файл с нужным форматом
                pixmap.save(filePath, chosenFormat.toUtf8().constData());
            }
        } else {
            // Если формат не поддерживается, выводим сообщение об ошибке
            QMessageBox::warning(this, tr("Ошибка"), tr("Неподдерживаемый формат файла."));
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

    if (!painter.isActive()) {
        return;
    }

    //  Включение сглаживания для графических примитивов
    painter.setRenderHint(QPainter::Antialiasing, true);


    painter.translate((int) (width() / 2 + Scaling::getDeltaX()), (int) (height() / 2 + Scaling::getDeltaY()));
    drawBackground::drawFon(painter);

    if (ModeManager::getCursor())
        DrawAdditionalInf::drawCursor(painter);


    if (!LeftMenuElem.empty() &&
        ModeManager::getActiveMode(MouseMode::LeftClick) || ModeManager::getActiveMode(MouseMode::RightClick)) {
        LeftMenuElem.clear();
    } else if (LeftMenuElem.size() > 4) {
        LeftMenuElem.clear();
        DrawAdditionalInf::setLeftMenuID(0);
    }

    // Отрисовка фигур мышкой

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

    // Функции выделения и перемещения

    // Выделение
    if (ModeManager::getActiveMode(WorkModes::Editor)) {
        if (ModeManager::getActiveMode(MouseMode::LeftClick)) {
            if (circleStorage != nullptr && circleStorage->size() > 0 ||
                sectionStorage != nullptr && sectionStorage->size() > 0 ||
                pointStorage != nullptr && pointStorage->size() > 0)
                findClosesObject();
        } else if (ModeManager::getActiveMode(MouseMode::DoubleClickLeft)) {
            id = 0;
            if (circleStorage != nullptr && circleStorage->size() > 0 ||
                sectionStorage != nullptr && sectionStorage->size() > 0 ||
                pointStorage != nullptr && pointStorage->size() > 0)
                findClosesObject();

            if (id != 0) {
                emit DoubleClickOnObject(id);
            }

        }
    } else if (ModeManager::getActiveMode(WorkModes::Move)) {
        if (!ModeManager::getActiveMode(MouseMode::ReleasingLeft) &&
            ModeManager::getActiveMode(MouseMode::LeftClick)) {
            // Перемещение только если зажата левая клавиша
            if (IDmove.id != 0) {

                emit MovingFigures(); // Даем сигнал перемещения


            } else {
                if (circleStorage != nullptr && circleStorage->size() > 0 ||
                    sectionStorage != nullptr && sectionStorage->size() > 0 ||
                    pointStorage != nullptr && pointStorage->size() > 0)
                    if (findClosesObject()) {
                        IDmove = id;
                        selectedClear();
                    } else {
                        IDmove.id = 0;
                    }
            }
        } else {
            IDmove.id = 0;
        }
    } else {
        id = 0;
        selectedClear();
    }

    QElapsedTimer timer;
    timer.start();  // Начинаем отсчет времени



    // Вызов функции рисования
    drawingFigures(painter);

    // Получаем прошедшее время в миллисекундах
    qint64 elapsedTime = timer.elapsed();

    // qDebug() << "Time for drawingFigures: " << elapsedTime << "ms";

    figureDelete();

    QFrame::paintEvent(event);
}


void QTPainter::onWorkWindowResized() {
    // При изменении размера окна родителя меняем размер
    Scaling::setActualMonitorSize(parentWidget()->width(),parentWidget()->height());
    resize(parentWidget()->size());
}

