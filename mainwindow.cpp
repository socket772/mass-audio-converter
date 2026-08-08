#include "mainwindow.h"
#include <QDebug>
#include "QFileDialog"
#include "QFlags"
#include "ui_mainwindow.h"
#include "utils.h"
#include <qpushbutton.h>

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
    connect(ui->threadsSlider, &QSlider::valueChanged, this, [this] {
        updateSliderLabel(ui->threadsSlider->sliderPosition());
    });

    // Riempo la combobox dei formati di output
    ui->formatComboBox->addItems(formatsList());

    // Connetto il pulsate di avvio
    connect(ui->startPushButton, &QPushButton::clicked, this, &MainWindow::startProcessButton);

    // configuro la lista di elementi
    widgetList = {ui->formatComboBox,
                  ui->inputFilePickerPushButton,
                  ui->outputFilePickerPushButton,
                  ui->startPushButton};

    // Configuro il form data
    formData = {ui->inputFolderTextEdit->toPlainText().toStdString(),
                ui->outputFolderTextEdit->toPlainText().toStdString(),
                ui->formatComboBox->currentText().toStdString(),
                ui->threadsSlider->sliderPosition(),
                ui->overwriteCheckBox->isChecked()};
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*
 * Apri il file picker, seleziona se di input o di output
 */
void MainWindow::openFilePicker(QString mode) {
    QString filePath = QFileDialog::getExistingDirectory(this, tr("file_picker_select_folder"), "./");
    if (!filePath.isEmpty()) {
        if (mode == "input"){
            ui->inputFolderTextEdit->setPlainText(filePath);
            formData.inputFolder = filePath.toStdString();
        }
        else{
            ui->outputFolderTextEdit->setPlainText(filePath);
            formData.outputFolder = filePath.toStdString();
        }
    }
}

/* 
 * Aggiorna il label accanto allo slider per indicare in numero di thread
 */
void MainWindow::updateSliderLabel(qint64 threads) {
    if (threads == 0)
        ui->threadsNumberLabel->setText("auto");
    else
        ui->threadsNumberLabel->setText(QString::number(threads));
    formData.threadsNumber = threads;
}

void MainWindow::startProcessButton()
{
    /*
     * Disabilita tutti i pulsanti
     */
    for (int i = 0; i < widgetList.size(); ++i) {
        widgetList.at(i)->setEnabled(false);
    }

    filesList = filteredFilesFolder(formData.inputFolder);
}