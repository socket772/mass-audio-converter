#include "mainwindow.h"
#include <QDebug>
#include "QFileDialog"
#include "QFlags"
#include "converterworker.h"
#include "ui_mainwindow.h"
#include "utils.h"
#include <qlogging.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <string>

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

    // Imposto il massimo dei thread per lo slider
    ui->threadsSlider->setMaximum(QThread::idealThreadCount());

    // Connetto il pulsate di avvio
    connect(ui->startPushButton, &QPushButton::clicked, this, &MainWindow::startProcessButton);

    // configuro la lista di elementi
    widgetList = {ui->formatComboBox,
                  ui->inputFilePickerPushButton,
                  ui->outputFilePickerPushButton,
                  ui->startPushButton,
                  ui->threadsSlider};

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
void MainWindow::lockUi()
{
    /*
     * Disabilita tutti i pulsanti
     */
    for (int i = 0; i < widgetList.size(); ++i) {
        widgetList.at(i)->setEnabled(false);
    }
}

void MainWindow::unlockUi()
{
    /*
     * Abilita tutti i pulsanti solo se tutti i thread hanno finito
     */
    for (int i = 0; i < widgetList.size(); ++i) {
        widgetList.at(i)->setEnabled(true);
    }
}

void MainWindow::startProcessButton()
{
    lockUi();
    finishedThreads = 0;
    filesList = filteredFilesFolder(formData.inputFolder);
    if (filesList.size() == 0) {
        ui->logText->append(tr("no_files_present_input"));
        unlockUi();
        return;
    }
    // Imposto la barra del progresso
    ui->progressBar->setMaximum(filesList.size());

    if (formData.threadsNumber == 0) {
        formData.threadsNumber = QThread::idealThreadCount();
    }

    for (int i = 0; i < formData.threadsNumber; ++i) {
        ConverterWorker *worker = new ConverterWorker(std::to_string(i));

        connect(worker,
                &ConverterWorker::progress,
                this,
                [this](std::string threadId, std::string filePath) {
                    ui->logText->append(QString::fromStdString(threadId + "->" + filePath));
                    ui->progressBar->setValue(ui->progressBar->value() + 1);
                });

        connect(worker, &ConverterWorker::finished, this, [this, worker]() {
            finishedThreads++;
            if (finishedThreads == formData.threadsNumber) {
                unlockUi();
            }
        });

        worker->start();
    }
}
