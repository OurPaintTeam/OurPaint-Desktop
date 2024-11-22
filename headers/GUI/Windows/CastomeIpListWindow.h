#ifndef OURPAINT_CASTOMEIPLISTWINDOW_H
#define OURPAINT_CASTOMEIPLISTWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QApplication>
#include <QDebug>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QIcon>

class CastomeIPListWindow : public QWidget {
Q_OBJECT

public:
    explicit CastomeIPListWindow(const QStringList &ipAddresses, QWidget *parent = nullptr) : QWidget(parent) {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
        setAttribute(Qt::WA_TranslucentBackground);
        setWindowModality(Qt::ApplicationModal);
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(10, 10, 10, 10);
        QHBoxLayout *topLayout = new QHBoxLayout();
        topLayout->setContentsMargins(0, 0, 0, 0);
        topLayout->setSpacing(0);

        QPushButton *closeButton = new QPushButton(this);
        closeButton->setFixedSize(25, 25);
        closeButton->setIcon(QIcon("../Static/icons/close.ico"));
        closeButton->setStyleSheet("QPushButton { background: none; border: none; color: white; }"
                                   "QPushButton:hover { background-color: rgba(255, 255, 255, 0.3); }");
        connect(closeButton, &QPushButton::clicked, this, &QWidget::close);

        QLabel *label = new QLabel("Available IP-addresses", this);
        label->setStyleSheet("color: #D8D8F6; font-weight: bold; font-size: 16px;");

        topLayout->addWidget(label);
        topLayout->addStretch();
        topLayout->addWidget(closeButton);

        layout->addLayout(topLayout);

        QScrollArea *scrollArea = new QScrollArea(this);
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setWidgetResizable(true);
        scrollArea->setStyleSheet("background: transparent;");

        QWidget *scrollWidget = new QWidget(this);
        QVBoxLayout *scrollLayout = new QVBoxLayout(scrollWidget);
        scrollLayout->setAlignment(Qt::AlignTop);

        for (const QString &ip : ipAddresses) {
            QHBoxLayout *hLayout = new QHBoxLayout();

            QLabel *ipLabel = new QLabel(ip, this);
            ipLabel->setStyleSheet("color: #D8D8F6; font-size: 14px;");

            QPushButton *connectButton = new QPushButton("Connect", this);
            connectButton->setStyleSheet(
                    "QPushButton { background-color: #4CAF50; color: white; border-radius: 5px; padding: 5px 10px; }"
                    "QPushButton:hover { background-color: #45a049; }"
            );

            connect(connectButton, &QPushButton::clicked, this, [this, ip]() {
                emit onConnectButtonClicked(ip);
            });

            hLayout->addWidget(ipLabel);
            hLayout->addStretch();
            hLayout->addWidget(connectButton);

            scrollLayout->addLayout(hLayout);
        }

        scrollWidget->setLayout(scrollLayout);
        scrollArea->setWidget(scrollWidget);

        layout->addWidget(scrollArea);

        setLayout(layout);
        resize(400, 300);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QColor(50, 50, 50, 240));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rect(), 10, 10);
    }

signals:
    void onConnectButtonClicked(const QString &ip);
};

#endif // OURPAINT_CASTOMEIPLISTWINDOW_H