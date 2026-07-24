#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QFlags"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->inputFilePickerPushButton, &QPushButton::clicked, this, [this]{
        openFilePicker("input");
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openFilePicker(QString mode) {
    QString filePath = QFileDialog::getExistingDirectory(this, tr("file_picker_select_folder"), "./");
    if (!filePath.isEmpty()) {
        if (mode == "input"){
                ui->inputFolderTextEdit->setPlainText(filePath);
            }
        else {}
    }
}