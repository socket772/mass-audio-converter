#include "mainwindow.h"
#include "QFileDialog"
#include "QFlags"
#include "ui_mainwindow.h"
#include "utils.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Connetti il pulsante per aprire il file picker di sistema per la cartella di input
    connect(ui->inputFilePickerPushButton, &QPushButton::clicked, this, [this]{
        openFilePicker("input");
    });

    // Connetti il pulsante per aprire il file picker di sistema per la cartella di output
    connect(ui->outputFilePickerPushButton, &QPushButton::clicked, this, [this]{
        openFilePicker("output");
    });

    // Connetti lo slider al label accanto per indicare in numero di thread da usare
    // se 0 allora sara automatico (ovvero il massimo disponibile)
    connect(ui->threadsSlider, &QSlider::sliderMoved, this, [this]{
        updateSliderLabel(ui->threadsSlider->sliderPosition());
    });

    // Riempo la combobox dei formati di output
    ui->formatComboBox->addItems(formatsList());
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
        else{
            ui->outputFolderTextEdit->setPlainText(filePath);
        }
    }
}

void MainWindow::updateSliderLabel(qint64 threads) {
    if (threads== 0)
        ui->threadsNumberLabel->setText("auto");
    else
        ui->threadsNumberLabel->setText(QString::number(threads));
}