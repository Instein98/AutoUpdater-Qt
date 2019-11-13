#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QThread>
#include "worker.h"

namespace Ui {
class Dialog;
}

class Dialog : public QDialog {
    Q_OBJECT

signals:
    void sigWork();

private slots:
    void showProgress(qint64 bytesRead, qint64 totalBytes);
    void showStatus(const QString &);
    void showInfo(const QString &);
    void showError(const QString &, const QString &);

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

private:
    Ui::Dialog *ui;
    Worker *worker;
    QThread workerThread;
};

#endif // DIALOG_H
