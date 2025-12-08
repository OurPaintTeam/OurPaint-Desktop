#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QPainter>
#include <QFrame>
#include <QTreeWidgetItem>
#include <vector>
#include <string>
#include <QWidget>
#include <QFileDialog>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QGuiApplication>
#include <QScreen>
#include <QGestureEvent>
#include <QDirIterator>
#include <QStandardPaths>

#include "Help.h"
#include "CustomWindowError.h"
#include "CustomWindowSuccessful.h"
#include "CustomWindowWarning.h"
#include "CustomIpListWindow.h"
#include "LocalScanner.h"
#include "SaveDialog.h"
#include "InputWindow.h"
#include "ui_mainwindow.h"
#include "QTPainter.h"
#include "Modes.h"
#include "LeftMenuBar.h"
#include "Settings.h"
#include "ParameterDelegate.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

private:
    Ui::MainWindow* ui = new Ui::MainWindow;

    const QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString projectsPath = QDir::cleanPath(documentsPath + "/OurPaint/projects");
    const QString settingsPath = QDir::cleanPath(documentsPath + "/OurPaint/settings/settings.set");
    const QString filePath = QDir::cleanPath(documentsPath + "/OurPaint/settings/projects.set");


    LeftMenuBar* leftMenuBar= new LeftMenuBar(this);             // A class for managing the left menu
    Settings* settings = new Settings(settingsPath);                   // Saving Settings

    enum ResizeRegion {
        None,
        Top, Bottom, Left, Right,
        TopLeft, TopRight, BottomLeft, BottomRight
    };

    const qint8 edgeMargin = 8;

    bool resizing = false;
    bool moving = false;
    QPoint dragStartPos{0,0};
    QRect originalGeometry;
    ResizeRegion currentRegion = None;

private:
    void openProject(const QString& name);
    void initListProjectStartWindow();

    void initConnections();
    void setupLeftMenu();
    void updateShapeCursor(const QPoint& pos);

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    LeftMenuBar* getLeftMenuBar() const;
    QTPainter* getQTPainter() const;

    [[maybe_unused]] void selectLeftMenuElem(QModelIndex& index);
    void setupConsoleCommands(const QStringList& commandList);
    void updateExitServerStyle(bool);
    void setMessage(const QString& name, const QString& message);

    /***    Custom windows      ***/
    void showError(const QString& text);
    void showSuccess(const QString& text);
    void showWarning(const QString& text);

    /***     Save/import settings       ***/
    QString getUserName();
    void saveSettings();
    void loadSettings();

    QPushButton* getFirstBut();
    QPushButton* getSecondBut();
    QPushButton* getThirdBut();
    QPushButton* getFourthBut();
    QPushButton* getFifthBut();
    QPushButton* getSixthBut();
    QPushButton* getSeventhBut();
    QPushButton* getEighthBut();
    QPushButton* getNinthBut();
    QPushButton* getTenthBut();

protected:
    void closeEvent(QCloseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    bool event(QEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

public slots:
    void loadProjectFile();
    void saveProjectToFile(const QString& format);
    void buttonScript();
    void openServer();
    void joinServer();
    void joinLocalServer();
    void exitSession();
    void Message();

    void Point();
    void Section();
    void Circle();
    void Arc();
    void ToolMoving();
    void ToolSelected();
    void ToolShowSize();

    void onExportJPG();
    void onExportJPEG();
    void onExportPNG();
    void onExportBMP();
    void onExportTIFF();
    void onExportPDF();
    void onExportOURP();
    void onExportSVG();
    void onCreateProject();

    void onLeftMenuRightClick(const QPoint& pos);
    void setNameUsers();
    void updateGrid(const bool checked);
    void updateAxis(const bool checked);
    void commandsInConsole();

signals:
    void EnterPressed(const QString& command);
    void EnterMessage(const QString& text);
    void NameUsers(const QString& text);
    void SigOpenServer(const QString& text);
    void SigJoinServer(const QString& text);
    void SigExitSession();
    void projectSaved(const QString& fileName, QString format);
    void LoadFile(const QString& fileName, const QTPainter* painter);
    void createFile(const QTPainter* painter);
    void changeTabs(const QTPainter* painter);
    void EmitScript(const QString& fileName);
};

#endif // MAINWINDOW_H