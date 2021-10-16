#include <QKeyEvent>
#include <QPixmap>
#include "inventory.h"
#include "ui_inventory.h"

Inventory::Inventory(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Inventory)
{
    ui->setupUi(this);

    connect(ui->radioButtonGrass, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToGrass()));
    connect(ui->radioButtonDirt, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToDirt()));
    connect(ui->radioButtonStone, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToStone()));
    connect(ui->radioButtonSand, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToSand()));
    connect(ui->radioButtonWood, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToWood()));
    connect(ui->radioButtonLeaf, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToLeaf()));
    connect(ui->radioButtonSnow, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToSnow()));
    connect(ui->radioButtonIce, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToIce()));
}

Inventory::~Inventory() {
    delete ui;
}

// close the inventory
void Inventory::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_I) {
        this->close();
    }
}

// update quantities
void Inventory::slot_setNumGrass(int n) {
    ui->numGrass->display(n);
}

void Inventory::slot_setNumDirt(int n) {
    ui->numDirt->display(n);
}

void Inventory::slot_setNumStone(int n) {
    ui->numStone->display(n);
}

void Inventory::slot_setNumSand(int n) {
    ui->numSand->display(n);
}

void Inventory::slot_setNumWood(int n) {
    ui->numWood->display(n);
}

void Inventory::slot_setNumLeaf(int n) {
    ui->numLeaf->display(n);
}

void Inventory::slot_setNumSnow(int n) {
    ui->numSnow->display(n);
}

void Inventory::slot_setNumIce(int n) {
    ui->numIce->display(n);
}

// set current block
void Inventory::slot_setCurrBlockToGrass() {
    ui_main->mygl->currBlockType = GRASS;
}

void Inventory::slot_setCurrBlockToDirt() {
    ui_main->mygl->currBlockType = DIRT;
}

void Inventory::slot_setCurrBlockToStone() {
    ui_main->mygl->currBlockType = STONE;
}

void Inventory::slot_setCurrBlockToSand() {
    ui_main->mygl->currBlockType = SAND;
}

void Inventory::slot_setCurrBlockToWood() {
    ui_main->mygl->currBlockType = WOOD;
}

void Inventory::slot_setCurrBlockToLeaf() {
    ui_main->mygl->currBlockType = LEAF;
}

void Inventory::slot_setCurrBlockToSnow() {
    ui_main->mygl->currBlockType = SNOW;
}

void Inventory::slot_setCurrBlockToIce() {
    ui_main->mygl->currBlockType = ICE;
}
