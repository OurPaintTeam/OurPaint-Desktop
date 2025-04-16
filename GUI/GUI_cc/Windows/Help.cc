#include <QTextBlock>
#include "Help.h"

Help::Help(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::Help) {
    ui->setupUi(this);
  //  ui->scrollArea->setStyleSheet("background: transparent;");
    // Install event filter on the topBar to enable dragging
    ui->topBar->installEventFilter(this);

    // Connect buttons to slots
    connect(ui->commandsButton, &QPushButton::clicked, this, &Help::showCommands);
    connect(ui->requirementsButton, &QPushButton::clicked, this, &Help::showRequirements);
    connect(ui->aboutButton, &QPushButton::clicked, this, &Help::showAbout);
    connect(ui->guide, &QPushButton::clicked, this, &Help::GuideInf);

    // Initially show commands content
    showCommands();
}

Help::~Help() {
    delete ui;
}

bool Help::eventFilter(QObject *obj, QEvent *event) {
    if (obj == ui->topBar) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                dragPosition = mouseEvent->globalPosition().toPoint() - frameGeometry().topLeft();
                return true;
            }
        } else if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->buttons() & Qt::LeftButton) {
                move(mouseEvent->globalPosition().toPoint() - dragPosition);
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void Help::showCommands() {
    QString commandsText = R"(
        <h2 style="color: #5A9BD5;">Команды</h2>
        <ul>
            <li><b>point x y</b> - Нарисовать точку в координатах (x,y)</li>
            <li><b>section x1 y1 x2 y2</b> - Нарисовать отрезок между точками (x1,y1) и (x2,y2)</li>
            <li><b>circle x y r</b> - Нарисовать окружность с радиусом r и центром в (x,y)</li>
            <li><b>getelement ID</b> - Получить информацию о элементе с ID</li>
            <li><b>clear</b> - Очистить экран</li>
            <li><b>addreq &lt;instruction&gt; &lt;objects&gt; &lt;param&gt;</b> - Выполнить действие с объектами</li>
        </ul>
    )";
    ui->helpContentBrowser->setHtml(commandsText);
}

void Help::showRequirements() {
    QString requirementsText = R"(
        <h2 style="color: #5A9BD5;">Требования</h2>
        <ul>
            <li>'1' - Принимает 2 аргумента: одну точку и один отрезок, устанавливает расстояние между ними на <param>.</li>
            <li>'2' - Принимает точку и отрезок, устанавливает их относительно друг друга (может быть без <param>).</li>
            <li>'3' - Принимает 2 точки (они могут быть на фигуре) и устанавливает расстояние между ними на <param>.</li>
            <li>'4' - Принимает 2 точки (они могут быть на фигуре) и устанавливает их относительно друг друга (может быть без <param>).</li>
            <li>'5' - Принимает окружность и отрезок, устанавливает расстояние между ними на <param>.</li>
            <li>'6' - Принимает окружность и отрезок, устанавливает отрезок на окружности (может быть без <param>).</li>
            <li>'7' - Принимает окружность и отрезок, полностью помещает отрезок внутри окружности (может быть без <param>).</li>
            <li>'8' - Принимает 2 отрезка, устанавливает расстояние и параллельное выравнивание между ними на <param>.</li>
            <li>'9' - Принимает 2 отрезка, устанавливает их перпендикулярность друг другу (может быть без <param>).</li>
            <li>'10' - Принимает 2 отрезка, устанавливает определенный угол между ними на <param>.</li>
        </ul>
    )";
    ui->helpContentBrowser->setHtml(requirementsText);
}

void Help::showAbout() {
    QString aboutText = R"(
        <h2 style="color: #5A9BD5;">О проекте</h2>
        <p>OurPaint — это система автоматизированного проектирования (CAD) на двухмерной плоскости, разработанная с использованием фреймворка Qt. Она позволяет пользователям создавать и редактировать геометрические примитивы, такие как линии, точки и окружности. Программа поддерживает совместную работу, что делает её подходящей для командных проектов.</p>
    )";
    ui->helpContentBrowser->setHtml(aboutText);
}

void Help::GuideInf() {
    QString guideText = R"(
    <html>
        <h2 style="color: #5A9BD5; text-align: center;">Добро пожаловать в OurPaint</h2>

        <img src="C:/Users/Tim/CLionProjects/OurPaint_/Static/jpg/Primitives.png"
             alt="Primitives"
             style="float: left; margin: 10px; width: 150px; height: auto;">

        <h3 style="color: #4A6E8C;">Как начать работать:</h3>
        <p style="font-size: 14px; text-align: justify; margin: 10px;">
            Для начала работы откройте программу и выберите нужные инструменты через вертикальное меню слева.
            Ниже описаны основные шаги для начала работы:
        </p>

        <h4 style="color: #FF6347;" id="section1">1) Фигуры:</h4>
        <p style="font-size: 13px; text-align: justify; margin-left: 20px;">
            Этот раздел отображает информацию о выбранных фигурах: их координаты, размеры и другие параметры.
        </p>

        <h4 style="color: #FF6347;" id="section2">2) Чат:</h4>
        <p style="font-size: 13px; text-align: justify; margin-left: 20px;">
            Для начала работы с чатом необходимо создать сервер или подключиться к существующему. Это поможет вам обмениваться данными с другими пользователями.
        </p>

        <h4 style="color: #FF6347;" id="section3">3) Выбор примитивов:</h4>
        <p style="font-size: 13px; text-align: justify; margin-left: 20px;">
            Выберите один из базовых примитивов: точка, окружность или линия для работы с чертежами.
        </p>

        <h4 style="color: #FF6347;" id="section4">4) Режимы редактирования:</h4>
        <p style="font-size: 13px; text-align: justify; margin-left: 20px;">
            Выберите режим редактирования в зависимости от задач: <b>Обычный режим</b>, <b>Режим перемещения</b>, <b>Режим вращения</b>, <b>Изменение размеров</b>.
        </p>

        <h4 style="color: #FF6347;" id="section5">5) Требования:</h4>
        <p style="font-size: 13px; text-align: justify; margin-left: 20px;">
            Для работы с проектами существует 10 основных требований, которые необходимо соблюдать. Эти требования включают стандартные размеры, разрешения и спецификации для различных типов объектов.
        </p>

        <h3 style="color: #4A6E8C;">Полезные советы:</h3>
        <ul style="font-size: 12px; text-align: justify; margin: 10px;">
            <li>Используйте клавишу <b>Shift</b> для множественного выбора объектов.</li>
            <li>Для перемещения объектов просто выделите их и перетащите.</li>
            <li>Чтобы очистить выбор, щелкните в пустом месте области рисования.</li>
        </ul>

        <h3 style="color: #4A6E8C;">Поддержка:</h3>
        <p style="font-size: 13px; text-align: justify; margin: 10px;">
            Если у вас возникли вопросы или проблемы, пожалуйста, обратитесь в нашу службу поддержки через меню <a href="#support" style="color: #1E90FF;">'Помощь'</a>.
        </p>

        <p style="font-size: 13px; text-align: justify; margin: 10px;">Спасибо за использование OurPaint! Удачного проектирования!</p>

        <h2 style="color: #5A9BD5;" id="support">Контакты для поддержки</h2>
        <p style="font-size: 13px; text-align: justify; margin: 10px;">Вы можете связаться с нами по следующим каналам:</p>
        <ul style="font-size: 13px; text-align: justify; margin-left: 20px;">
            <li>Телеграм: <a href="https://t.me/+Du5DMLT9QukxZjky" style="color: #1E90FF;">Наш канал</a></li>
            <li>Email: <a href="mailto:support@ourpaint.com" style="color: #1E90FF;">support@ourpaint.com</a></li>
        </ul>

        <h3 style="color: #4A6E8C;">Горячие клавиши и команды:</h3>
        <p style="font-size: 13px; text-align: justify; margin: 10px;">
            <b>Горячие клавиши:</b> <br>
            <ul style="font-size: 12px; text-align: justify;">
                <li><b>Ctrl + стрелочки</b> — Масштабирование окна приложения в области монитора.</li>
                <li><b>Ctrl + 0</b> — Сброс масштаба и перемещение в точку (0,0).</li>
                <li><b>Ctrl + Z</b> — Отмена действия.</li>
                <li><b>Ctrl + Shift + Z</b> — Повтор действия (Redo).</li>
            </ul>
        </p>

        <h3 style="color: #4A6E8C;">Как работать с инструментами:</h3>
        <p style="font-size: 13px; text-align: justify; margin: 10px;">
            Выберите любой примитив в меню или введите команду в консоли для его отрисовки. Программа автоматически подстроит масштаб, если отрисовка выходит за пределы экрана.
        </p>

        <p style="font-size: 13px; text-align: justify; margin: 10px;">
            Для задания требований, выберите фигуру с помощью <b>Shift</b> и затем выберите требуемые параметры. Вы можете также использовать правую кнопку мыши для сброса фигуры или перемещения объектов.
        </p>

        <h3 style="color: #4A6E8C;">Интерфейс и консоль:</h3>
        <p style="font-size: 13px; text-align: justify; margin: 10px;">
            Все команды в консоли вводятся в формате:
            <code>point x y</code>, <code>section x y x1 y1</code>, <code>circle x y r</code>.
            Для задания требований используйте команду <code>addreq</code>. После нажатия <b>Tab</b> вы увидите подсказки, которые помогут вам.
        </p>
    </html>
)";
    ui->helpContentBrowser->setHtml(guideText);
    ui->helpContentBrowser->setOpenExternalLinks(false); // Чтобы ссылки обрабатывались внутри приложения

    // Подключаем обработчик кликов по ссылкам
    connect(ui->helpContentBrowser, &QTextBrowser::anchorClicked, this, &Help::handleAnchorClicked);
}

void Help::handleAnchorClicked(const QUrl &url) {
    // Check if the URL has a fragment
    if (!url.fragment().isEmpty()) {
        QString fragment = url.fragment();

        // Iterate through the blocks to find the one with the matching name
        QTextDocument *document = ui->helpContentBrowser->document();
        QTextBlock block = document->firstBlock();

        while (block.isValid()) {
            if (block.text() == fragment) { // Assuming the block text matches the fragment
                // Get the position of the block
                int position = block.position();

                // Scroll to the position
                QTextCursor textCursor = ui->helpContentBrowser->textCursor();
                textCursor.setPosition(position);
                ui->helpContentBrowser->setTextCursor(textCursor);
                return; // Exit after finding the block
            }
            block = block.next(); // Move to the next block
        }
    }
}

