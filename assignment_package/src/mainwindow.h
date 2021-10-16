#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "cameracontrolshelp.h"
#include "playerinfo.h"
#include "inventory.h"


namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionQuit_triggered();
    void on_actionCamera_Controls_triggered();
    void slot_inventoryOpenClose(bool);

private:
    Ui::MainWindow *ui;
    CameraControlsHelp cHelp;
    PlayerInfo playerInfoWindow;
    Inventory inventoryWindow;
};


#endif // MAINWINDOW_H
