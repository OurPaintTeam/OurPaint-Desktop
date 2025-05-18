#ifndef OURPAINT_HEADERS_GUI_WINDOWS_SAVEDIALOG_H_
#define OURPAINT_HEADERS_GUI_WINDOWS_SAVEDIALOG_H_

/*
*
* Custom window for saving the window
*
*
*/



class SaveDialog : public QDialog {
private:
    QString file;
    bool mousePressed;
    QPoint mousePos;
public:
    //void setFile(QString File){file=File;}
    SaveDialog(QWidget *parent ) : QDialog(parent) {
        setParent(parent, Qt::SubWindow);
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
        setAttribute(Qt::WA_TranslucentBackground);
        setWindowModality(Qt::ApplicationModal); // For the modality only within the parent window

        // setStyleSheet("background-color: gray; color: black;");

        QVBoxLayout * layout = new QVBoxLayout(this);
        layout->setContentsMargins(10, 10, 10, 10);
        QLabel * label = new QLabel("Сохранить проект?", this);
        layout->addWidget(label);

        QHBoxLayout * buttonLayout = new QHBoxLayout();

        QPushButton * yesButton = new QPushButton("Yes", this);
        QPushButton * noButton = new QPushButton("No", this);
        QPushButton * cancelButton = new QPushButton("Cancel", this);

        buttonLayout->addWidget(yesButton);
        buttonLayout->addWidget(noButton);
        buttonLayout->addWidget(cancelButton);

        layout->addLayout(buttonLayout);

        connect(yesButton, &QPushButton::clicked, this, [this]() {
            done(QMessageBox::Yes);
        });
        connect(noButton, &QPushButton::clicked, this, [this]() {
            done(QMessageBox::No);
        });
        connect(cancelButton, &QPushButton::clicked, this, [this]() {
            done(QMessageBox::Cancel);
        });
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QColor(50, 50, 50, 240));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rect(), 10, 10);
    }

    void mousePressEvent(QMouseEvent *event) override { // Press
        if (event->button() == Qt::LeftButton) {
            setCursor(Qt::SizeAllCursor);
            mousePressed = true;
            mousePos = event->pos();
            event->accept();
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override { // Moving
        if (mousePressed) {
            //move(event->globalPos() - mousePos);
            QPoint Pos = pos();
            QPoint newPos = Pos + (event->pos() - mousePos);
            move(newPos);
            event->accept();
        }
    }

    void mouseReleaseEvent(QMouseEvent *event) override { // release
        if (event->button() == Qt::LeftButton) {
            setCursor(Qt::ArrowCursor);
            mousePressed = false;
            event->accept();
        }
    }

};


#endif // ! OURPAINT_HEADERS_GUI_WINDOWS_SAVEDIALOG_H_
