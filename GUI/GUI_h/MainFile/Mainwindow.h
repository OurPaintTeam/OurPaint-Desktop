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

#include "FileSystems.h"
#include "ui_mainwindow.h"
#include "LeftMenuBar.h"
#include "MainWindowController.h"

class FileSystems;
class LeftMenuBar;
class ui_mainwindow;
class Help;
class CustomWindowError;
class CustomWindowSuccessful;
class CustomWindowWarning;
class CustomIpListWindow;
class SaveDialog;
class InputWindow;
class QTPainter;
class Modes;
class ParameterDelegate;
class  MainWindowController;


QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow final : public QMainWindow {
Q_OBJECT

private:
    Ui::MainWindow* ui = new Ui::MainWindow;
    LeftMenuBar* leftMenuBar= new LeftMenuBar(this);             // A class for managing the left menu
    MainWindowController* windowController = new MainWindowController(this);

    FileSystems* fileSystems = new FileSystems(this);
    friend class FileSystems;

private:
    void initConnections();
    void setupLeftMenu();

public:
    explicit MainWindow(QWidget* parent = nullptr);

    LeftMenuBar* getLeftMenuBar() const;
    QTPainter* getQTPainter() const;
    QString getProjectPath() const;

    void inStartWindow() const;
    void inProjectWindow() const;

    void selectLeftMenuElem(const QModelIndex& index) const;
    void setupConsoleCommands(const QStringList& commandList) const;
    void updateExitServerStyle(bool) const;
    void setMessage(const QString& name, const QString& message) const;

    /***    Custom windows      ***/
    void showError(const QString& text) const;
    void showSuccess(const QString& text) const;
    void showWarning(const QString& text) const;

    /***     Save/import settings       ***/
    QString getUserName();

    QPushButton* getFirstBut() const;
    QPushButton* getSecondBut() const;
    QPushButton* getThirdBut() const;
    QPushButton* getFourthBut() const;
    QPushButton* getFifthBut() const;
    QPushButton* getSixthBut() const;
    QPushButton* getSeventhBut() const;
    QPushButton* getEighthBut() const;
    QPushButton* getNinthBut() const;
    QPushButton* getTenthBut() const;

    bool closeProgram();

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
    void slotSaveProject(const QString& workDir);
    void slotOpenProject(const QString& workDir);
    void slotChangeTabs(const QString& tabName);
    void slotCreateNewTab(const QString& tabName);

    void buttonScript();
    void Message();

    static void Point();
    static void Section();
    static void Circle();
    static void Arc();
    static void ToolMoving();
    static void ToolSelected();
    static void ToolShowSize();

    void onExportJPG();
    void onExportJPEG();
    void onExportPNG();
    void onExportBMP();
    void onExportTIFF();
    void onExportPDF();
    void onExportOURP();
    void onExportSVG();

    void onLeftMenuRightClick(const QPoint& pos);
    void updateGrid(const bool checked) const;
    void updateAxis(const bool checked) const;
    void commandsInConsole();

signals:
    void EnterCommand(const QString& command);
    void EnterMessage(const QString& text);
    void EmitScript(const QString& fileName);

    void SaveProject(const QString& workDir);
    void OpenProject(const QString& workDir);
    void ChangeTabs(const QString& tabName);
    void CreateNewTab(const QString& tabName);

    void SaveProjectInFormat(const QString& fileName,const QString& format);
};

#endif // MAINWINDOW_H