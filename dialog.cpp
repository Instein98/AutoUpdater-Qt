#include <QMessageBox>
#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) : QDialog(parent), ui(new Ui::Dialog) {
    ui->setupUi(this);
    ui->progressBar->setValue(0);
    worker = new Worker;
    worker->moveToThread(&workerThread);
    connect(&workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(this, SIGNAL(sigWork()), worker, SLOT(doWork()));
    connect(worker, SIGNAL(sigShowProgress(qint64, qint64)), this, SLOT(showProgress(qint64, qint64)));
    connect(worker, SIGNAL(sigShowStatus(const QString &)), this, SLOT(showStatus(const QString &)));
    connect(worker, SIGNAL(sigShowInfo(const QString &)), this, SLOT(showInfo(const QString &)));
    connect(worker, SIGNAL(sigShowError(const QString &, const QString &)), this, SLOT(showError(const QString &, const QString &)));
    connect(worker, SIGNAL(sigExit()), this, SLOT(exit()));
    workerThread.start();
    emit sigWork();
}

void Dialog::showProgress(qint64 dowloadedBytes, qint64 totalBytes){
//    qDebug() << "dowloadedBytes: " << dowloadedBytes;
//    qDebug() << "totalBytes: " << totalBytes;
    if (totalBytes == -1){
        ui->progressBar->setValue(0);
        return;
    }
    ui->progressBar->setValue(int(dowloadedBytes*100.0f/totalBytes));
}

void Dialog::showStatus(const QString & status){
    ui->statusLabel->setText(status);
}

void Dialog::showInfo(const QString & info){
    ui->versionInfo->setText(info);
}

void Dialog::showError(const QString & title, const QString & message){
    QMessageBox::StandardButton button = QMessageBox::critical(this, title, message);
    if(button == QMessageBox::Ok || QMessageBox::Close){
        exit();
    }
}

void Dialog::exit(){
    qApp->quit();
}

Dialog::~Dialog(){
    workerThread.exit();
    delete worker;
    delete ui;
}
