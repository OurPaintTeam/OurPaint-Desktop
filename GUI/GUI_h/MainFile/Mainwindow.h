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
#include "MouseEventWorkWindow.h"
#include "KeyWorkWindow.h"
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
    Ui::MainWindow* ui;

    MouseWorkWindow* mouseWW;            // For processing mouse events
    KeyWorkWindow* keyWW;                // For handling key events
    LeftMenuBar* leftMenuBar;             // A class for managing the left menu
    Settings* settings;                   // Saving Settings
    Help* helpWindow;                     // Help Window

    CustomWindowError* error;
    CustomWindowWarning* warning;
    CustomWindowSuccessful* success;

    QString documentsPath;
    QString projectsPath;
    QString settingsPath;

    std::vector<QString> commands;  // Command buffer for the console
    int Index;                      // Index for navigating commands

    enum ResizeRegion {
        None,
        Top, Bottom, Left, Right,
        TopLeft, TopRight, BottomLeft, BottomRight
    };

    const qint16 edgeMargin=8;

    bool resizing;
    bool moving;
    QPoint dragStartPos;
    QRect originalGeometry;
    ResizeRegion currentRegion = None;
public:

    MainWindow(QWidget* parent = nullptr);

    ~MainWindow();

    QTPainter* getQTPainter() const;
    LeftMenuBar* getLeftMenuBar() const;
    void initConnections();
    void setupConsoleCommands(const QStringList& commandList );
    void setupLeftMenu();
    void selectLeftMenuElem(QModelIndex& index);
    void updateStyle();
    void updateExitServerStyle(bool);
    void updateShapeCursor(const QPoint& pos);
    void setMessage(const std::string& name, const std::string& message);

    // Mouse Tracking
    void setAllMouseTracking(QWidget* widget);

    /***    Custom windows      ***/
    void showHelp();
    void showError(const QString& text);
    void showSuccess(const QString& text);
    void showWarning(const QString& text);

    /***     Save/import settings       ***/
    QString getUserName();
    void saveSettings();
    void loadSettings();

protected:

    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    bool event(QEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;


public slots:

    void redo();
    void undo();
    void deleteButton();
    void cut();
    void paste();
    void copy();

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

    void FigMoving();
    void ToolMoving();
    void ToolSelected();
    void onWorkWindowResized();

    void firstReq();
    void secondReq();
    void thirdReq();
    void fourthReq();
    void fifthReq();
    void sixthReq();
    void seventhReq();
    void eighthReq();
    void ninthReq();
    void tenthReq();
    void onExportJPG();
    void onExportJPEG();
    void onExportPNG();
    void onExportBMP();
    void onExportTIFF();
    void onExportPDF();
    void onExportOURP();
    void onExportSVG();

signals:

    void EnterPressed(const QString& command);
    void EnterMessage(const QString& text);
    void NameUsers(const QString& text);
    void SigOpenServer(const QString& text);
    void SigJoinServer(const QString& text);
    void SigExitSession();
    void projectSaved(const QString& fileName, QString format);
    void LoadFile(const QString& fileName);
    void EmitScript(const QString& fileName);

    void oneRequirements();
    void twoRequirements();
    void threeRequirements();
    void fourRequirements();
    void fiveRequirements();
    void sixRequirements();
    void sevenRequirements();
    void eightRequirements();
    void nineRequirements();
    void tenRequirements();

    void REDO();
    void UNDO();
    void DELETE();
    void CUT();
    void PASTE();
    void COPY();
    void resize();

};

#endif // MAINWINDOW_H