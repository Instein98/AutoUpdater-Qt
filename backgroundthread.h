#ifndef BACKGROUNDTHREAD_H
#define BACKGROUNDTHREAD_H

#include <QThread>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#define XML_URL "https://github.com/Instein98/PlayableLab/releases/download/Updater/Release.xml"
#define OLD_XML_NAME "Release.xml"
#define NEW_XML_NAME "Release-new.xml"
#define SUCCESS 1
#define FAILURE 0

class BackgroundThread : public QThread {

    Q_OBJECT

signals:
    void sigShowProgress(qint64, qint64);  // show the download progress
    void sigShowStatus(const QString &);  // show the status message
    void sigShowInfo(const QString &);  // show the update log
    void sigShowError(const QString &, const QString &);  // show error message

private slots:
    void updateProgress(qint64, qint64);  // update the progress bar

public:
    BackgroundThread(QObject* parent);
//    ~BackgroundThread();
    QNetworkAccessManager *networkManager;
    QNetworkReply *networkReply;

protected:
    virtual void run();

private:
    QString newVersion;
    QString newURL;
    QString newInfo;
    QString oldDirName;
    QString oldExeName;
    QString newZipName;
    QString newDirName;
    QString newExeName;
    int downloadXML();
    int downloadZip();
    bool checkUpdate();
    void extractZip();
    void executeGame();
    void replaceXML();
};

#endif // BACKGROUNDTHREAD_H
