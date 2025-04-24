/*
 *     Окошко для кнопки сервера
 * */

#ifndef OURPAINT_HEADERS_GUI_WINDOWS_WINDOWSERVER_H_
#define OURPAINT_HEADERS_GUI_WINDOWS_WINDOWSERVER_H_

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QHBoxLayout>

class InputWindow: public QDialog {
Q_OBJECT
private:
    QLineEdit *lineEdit;
    QPushButton *okButton;
    QPushButton *closeButton;
signals:
    void textEnter(const QString &text);

private slots:

    void OkClicked() {
        QString text = lineEdit->text();
        if (!text.isEmpty()) {
            emit textEnter(text);
        }
        accept();
    }

    void CloseClicked() {
        reject();
    }

public:
    InputWindow(const QString &message, QWidget *parent = nullptr) : QDialog(parent) {

        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
        setAttribute(Qt::WA_TranslucentBackground);
        setModal(true);

        setWindowModality(Qt::ApplicationModal);  // Для модальности только в пределах родительского окна

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(10, 10, 10, 10);

        QHBoxLayout *topLayout = new QHBoxLayout();
        topLayout->setContentsMargins(0, 0, 0, 0);
        topLayout->setSpacing(0);

        // Создание иконки закрытия
        closeButton = new QPushButton(this);
        closeButton->setFixedSize(25, 25);
        closeButton->setIcon(QIcon("../Static/icons/close.ico")); // Установка иконки
        closeButton->setStyleSheet("QPushButton { background: none; border: none; color: white; border-radius: 5px; }"
                                   "QPushButton:hover { background-color: rgba(255, 255, 255, 0.3); }"); // Подсветка при наведении
        connect(closeButton, &QPushButton::clicked, this, &InputWindow::CloseClicked);

        QLabel *label = new QLabel(message, this);
        label->setStyleSheet("color: #D8D8F6; font-weight: bold; font-size: 16px;");

        // Добавляем метку сначала, а затем кнопку закрытия
        topLayout->addWidget(label);
        topLayout->addWidget(closeButton, 0, Qt::AlignRight);

        layout->addLayout(topLayout);

        lineEdit = new QLineEdit(this);
        layout->addWidget(lineEdit);

         okButton = new QPushButton("OK", this);
        connect(okButton, &QPushButton::clicked, this, &InputWindow::OkClicked);
        layout->addWidget(okButton);

        lineEdit->installEventFilter(this);

        setLayout(layout);
        resize(250, 100);
    }

    QString getText() const {
        return lineEdit->text();
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QColor(50, 50, 50, 240));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rect(), 10, 10);
    }

    bool eventFilter(QObject *enter, QEvent *event) override {
        if (enter == lineEdit && event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
                OkClicked();
                return true;
            }
        }
        return QWidget::eventFilter(enter, event);
    }
};

#endif // ! OURPAINT_HEADERS_GUI_WINDOWS_WINDOWSERVER_H_
