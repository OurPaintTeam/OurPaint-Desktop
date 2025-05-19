#include <QTextBlock>
#include "Help.h"

Help::Help(QWidget* parent) :
        QMainWindow(parent),
        ui(new Ui::Help) {
    ui->setupUi(this);

    ui->topBar->installEventFilter(this);

    // Connect buttons to slots
    connect(ui->commandsButton, &QPushButton::clicked, this, &Help::showCommands);
    connect(ui->requirementsButton, &QPushButton::clicked, this, &Help::showRequirements);
    connect(ui->aboutButton, &QPushButton::clicked, this, &Help::showAbout);
    connect(ui->guide, &QPushButton::clicked, this, &Help::GuideInf);

    showCommands();

}

Help::~Help() {
    delete ui;
}

bool Help::eventFilter(QObject* obj, QEvent* event) {
    if (obj == ui->topBar) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                dragPosition = mouseEvent->globalPosition().toPoint() - frameGeometry().topLeft();
                return true;
            }
        } else if (event->type() == QEvent::MouseMove) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
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
 <h2 style="color: #5A9BD5;">Commands</h2>
        <ul>
            <li><b>point x y</b> - Draw a point in coordinates (x,y)</li>
            <li><b>section x1 y1 x2 y2</b> - Draw a line segment between points (x1,y1) and (x2,y2)</li>
            <li><b>circle x y r</b> - Draw a circle with radius r and center at (x,y)</li>
            <li><b>getelement ID</b> - Get information about an element with an ID</li>
            <li><b>clear</b> - Clear the screen</li>
            <li><b>adrem<instruction> <objects> <param></b> - Perform an action with objects</li>
        </ul>
    )";
    ui->helpContentBrowser->setHtml(commandsText);
}

void Help::showRequirements() {
    QString requirementsText = R"(
        <h2 style="color: #5A9BD5;">Requirements</h2>
         <ul>
            <li>'1' - Takes 2 arguments: one point and one segment, sets the distance between them to <param>.</li>
            <li>'2' - Accepts a point and a line segment, sets them relative to each other (can be without <param>).</li>
            <li>'3' - Takes 2 points (they can be on the shape) and sets the distance between them to <param>.</li>
            <li>'4' - Takes 2 points (they can be on the figure) and sets them relative to each other (it can be without <param>).</li>
            <li>'5' - Accepts a circle and a segment, sets the distance between them to <param>.</li>
            <li>'6' - Accepts a circle and a segment, sets the segment on the circle (it can be without <param>).</li>
            <li>'7' - Accepts a circle and a segment, completely places the segment inside the circle (it can be without <param>).</li>
            <li>'8' - Accepts 2 segments, sets the distance and parallel alignment between them to <param>.</li>
            <li>'9' - Accepts 2 segments, sets them perpendicular to each other (can be without <param>).</li>
            <li>'10' - Takes 2 segments, sets a certain angle between them to <param>.</li>
         </ul>
    )";
    ui->helpContentBrowser->setHtml(requirementsText);
}

void Help::showAbout() {
    QString aboutText = R"(
      <h2 style="color: #5A9BD5;">About the project</h2>
       <p>OurPaint is a computer—aided design (CAD) system on a two-dimensional plane developed using the Qt framework. It allows users to create and edit geometric primitives such as lines, points, and circles. The program supports collaboration, which makes it suitable for team projects.</p>
    )";
    ui->helpContentBrowser->setHtml(aboutText);
}

void Help::GuideInf() {
    QString guideText = R"(
    <html>
       <h2 style="color: #5A9BD5; text-align: center;">Welcome to OurPaint</h2>

        <img src="C:/Users/Tim/CLionProjects/OurPaint_/Static/jpg/Primitives.png"
             alt="Primitives"
             style="float: left; margin: 10px; width: 150px; height: auto;">

        <h3 style="color: #4A6E8C;">How to start working:</h3>
        <p style="font-size: 14px; text-align: justify; margin: 10px;">
            To get started, open the program and select the necessary tools from the vertical menu on the left.
            The basic steps for getting started are described below.:
        </p>

        <h4 style="color: #FF6347;" id="section1">1) Shapes:</h4>
        <p style="font-size: 13px; text-align: justify; margin-left: 20px;">
            This section displays information about the selected shapes: their coordinates, dimensions, and other parameters.
        </p>

        <h4 style="color: #FF6347;" id="section2">2) Chat:</h4>
        <p style="font-size: 13px; text-align: justify; margin-left: 20px;">
            To start working with the chat, you need to create a server or connect to an existing one. This will help you share data with other users.
        </p>

        <h4 style="color: #FF6347;" id="section3">3) Selection of primitives:</h4>
        <p style="font-size: 13px; text-align: justify; margin-left: 20px;">
            Choose one of the basic primitives: a point, circle, or line for working with drawings.
        </p>

        <h4 style="color: #FF6347;" id="section4">4) Editing modes:</h4>
        <p style="font-size: 13px; text-align: justify; margin-left: 20px;">
            Select the editing mode depending on the tasks: <b>Normal mode</b>, <b>Movement mode</b>, <b>Rotation mode</b>, <b>Resizing</b>.
        </p>

        <h4 style="color: #FF6347;" id="section5">5) Requirements:</h4>
        <p style="font-size: 13px; text-align: justify; margin-left: 20px;">
            To work with projects, there are 10 basic requirements that must be met. These requirements include standard sizes, resolutions, and specifications for various types of objects.
        </p>

        <h3 style="color: #4A6E8C;">Useful tips:</h3>
        <ul style="font-size: 12px; text-align: justify; margin: 10px;">
            <li>Use the <b>Shift</b> key to select multiple objects.</li>
            <li>To move objects, simply select them and drag them.</li>
            <li>To clear the selection, click in an empty place in the drawing area.</li>
        </ul>

        <h3 style="color: #4A6E8C;">Support:</h3>
        <p style="font-size: 13px; text-align: justify; margin: 10px;">
            If you have any questions or problems, please contact our support team via the <a href="#support" style="color: #1E90FF;">"Help"</a>.
        </p>

        <p style="font-size: 13px; text-align: justify; margin: 10px;">Thank you for using OurPaint! Successful design!</p>

        <h2 style="color: #5A9BD5;" id="support">Support contacts</h2>
<p style="font-size: 13px; text-align: justify; margin: 10px;">You can contact us through the following channels:</p>
        <ul style="font-size: 13px; text-align: justify; margin-left: 20px;">
            <li>Telegram: <a href="https://t.me/+Du5DMLT9QukxZjky " style="color: #1E90FF;">Our channel</a></li>
            <li>Email: <a href="mailto:support@ourpaint.com" style="color: #1E90FF;">support@ourpaint.com</a></li>
        </ul>

        <h3 style="color: #4A6E8C;">Keyboard shortcuts and commands:</h3>
        <p style="font-size: 13px; text-align: justify; margin: 10px;">
            <b>Keyboard shortcuts:</b> <br>
            <ul style="font-size: 12px; text-align: justify;">
                <li><b>Ctrl + arrows</b> — Zooming the application window in the monitor area.</li>
                <li><b>Ctrl + 0</b> — Resets the scale and moves to the point (0,0).</li>
                <li><b>Ctrl + Z</b> — Cancel the action.</li>
                <li><b>Ctrl + Shift + Z</b> — Repeat the action (Redo).</li>
            </ul>
        </p>

        <h3 style="color: #4A6E8C;">How to work with tools:</h3>
        <p style="font-size: 13px; text-align: justify; margin: 10px;">
            Select any primitive in the menu or enter a command in the console to render it. The program will automatically adjust the scale if the rendering goes off the screen.
        </p>

        <p style="font-size: 13px; text-align: justify; margin: 10px;">
            To set the requirements, select a shape using <b>Shift</b> and then select the required parameters. You can also use the right mouse button to reset the shape or move objects.
        </p>

        <h3 style="color: #4A6E8C;">Interface and console:</h3>
        <p style="font-size: 13px; text-align: justify; margin: 10px;">
            All commands in the console are entered in the format:
            <code>point x y</code>, <code>section x y x1 y1</code>, <code>circle x y r</code>.
            To set requirements, use the <code>addreq</code> command. After pressing <b>Tab</b>, you will see hints that will help you.
        </p>
    </html>
)";
    ui->helpContentBrowser->setHtml(guideText);

    // So that the links are processed inside the application
    ui->helpContentBrowser->setOpenExternalLinks(false);

    // Enabling the link click handler
    connect(ui->helpContentBrowser, &QTextBrowser::anchorClicked, this, &Help::handleAnchorClicked);
}

void Help::handleAnchorClicked(const QUrl& url) {
    // Check if the URL has a fragment
    if (!url.fragment().isEmpty()) {
        QString fragment = url.fragment();

        // Iterate through the blocks to find the one with the matching name
        QTextDocument* document = ui->helpContentBrowser->document();
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

