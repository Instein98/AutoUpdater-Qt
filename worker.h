#ifndef WORKER_H
#define WORKER_H

#include <QThread>
#include <QString>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>

#define XML_URL "https://github.com/Instein98/PlayableLab/releases/download/Updater/Release.xml"
#define OLD_XML_NAME "Release.xml"
#define NEW_XML_NAME "Release-new.xml"
#define SUCCESS 1
#define FAILURE 0

class Worker : public QObject {

    Q_OBJECT

signals:
    void sigShowProgress(qint64, qint64);  // show the download progress
    void sigShowStatus(const QString &);  // show the status message
    void sigShowInfo(const QString &);  // show the update log
    void sigShowError(const QString &, const QString &);  // show error message

private slots:
    void updateProgress(qint64, qint64);  // update the progress bar
    void doWork();
    void onReadyRead();
    void onFinishDownload();

public:
    Worker();
//    ~Worker();
    QNetworkAccessManager *networkManager;
    QNetworkReply *networkReply;

private:
    QString newVersion;
    QUrl newURL;
    QString newInfo;
    QString oldDirName;
    QString oldExeName;
    QString newZipName;
    QString newDirName;
    QString newExeName;
    QFile *downloadedFile;
    bool showProgress;
    int downloadXML();
    int downloadZip();
    int download(QUrl *url, bool wantShowProgress, QString status);
//    inline void setupDownload(QNetworkReply*, QNetworkAccessManager*, QUrl*, QObject*);
    bool checkUpdate();
    void extractZip();
    void executeGame();
    void replaceXML();
};

#endif // WORKER_H
