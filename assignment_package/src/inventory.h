#ifndef INVENTORY_H
#define INVENTORY_H

#include <QWidget>
#include <QPushButton>
#include <ui_mainwindow.h>

namespace Ui {
class Inventory;
}

class Inventory: public QWidget
{
    Q_OBJECT

public:
    explicit Inventory(QWidget *parent = nullptr);
    ~Inventory();

    void keyPressEvent(QKeyEvent *e);
    Ui::MainWindow *ui_main;

public slots:
    void slot_setNumGrass(int);
    void slot_setNumDirt(int);
    void slot_setNumStone(int);
    void slot_setNumSand(int);
    void slot_setNumWood(int);
    void slot_setNumLeaf(int);
    void slot_setNumIce(int);
    void slot_setNumSnow(int);

    void slot_setCurrBlockToGrass();
    void slot_setCurrBlockToDirt();
    void slot_setCurrBlockToStone();
    void slot_setCurrBlockToSand();
    void slot_setCurrBlockToWood();
    void slot_setCurrBlockToLeaf();
    void slot_setCurrBlockToIce();
    void slot_setCurrBlockToSnow();

private:
    Ui::Inventory *ui;
};

#endif // INVENTORY_H
