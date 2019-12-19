#ifndef WORKER_H
#define WORKER_H

#include <QString>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>

#define XML_URL "https://github.com/Instein98/PlayableLab/releases/download/Updater/Release.xml"
#define MSG_CHECK_UPDATE "Checking for updates..."
#define MSG_UPDATE "Updating..."
#define MSG_LAUNCH "Launching the game..."
#define OLD_XML_NAME "Release.xml"
#define NEW_XML_NAME "Release-new.xml"
#define SUCCESS 1
#define FAILURE 0

static QString xmlStartTag[6] = {"<version>", "<url>", "<zipName>", "<dirName>", "<exeName>", "<log>"};
static QString xmlEndTag[6] = {"</version>", "</url>", "</zipName>", "</dirName>", "</exeName>", "</log>"};

enum programStage{
    DownloadXML, DownloadGame, LaunchGame
};


// the Worker do the background work, communicate with GUI by signal
class Worker : public QObject {

    Q_OBJECT

signals:
    void sigShowProgress(qint64, qint64);  // show the download progress
    void sigShowStatus(const QString &);  // show the status message
    void sigShowInfo(const QString &);  // show the update log
    void sigShowError(const QString &, const QString &);  // show error message
    void sigCheckUpdate();
    void sigLaunchGame();
    void sigExit();

private slots:
    void updateProgress(qint64, qint64);  // update the progress bar
    void doWork();
    void onReadyRead();
    void onFinishDownload();
    void checkUpdate();
    void launchGame();

public:
    Worker();
    ~Worker();
    QNetworkAccessManager *networkManager;
    QNetworkReply *networkReply;

private:
    struct VersionInfo;
    struct VersionInfo *oldVersionInfo;
    struct VersionInfo *newVersionInfo;
    programStage currentStage;
    QFile *downloadedFile;
    bool showProgress;
    int download(QUrl*, bool, QString, QString);
    void extractZip();
    void parseXML(QString, VersionInfo *);
    bool unzip(QString);
    bool deleteFileOrDirectory(QString);
    bool checkNetworkOnline();

};

#endif // WORKER_H
