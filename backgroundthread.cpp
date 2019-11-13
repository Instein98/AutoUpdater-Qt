#include <QUrl>
#include "backgroundthread.h"

BackgroundThread::BackgroundThread(QObject *parent) : QThread (parent){
    networkManager = new QNetworkAccessManager(parent);
}

void BackgroundThread::run(){
    bool needUpdate = 1;
//    bool needUpdate = checkUpdate();
    if (needUpdate){
        emit sigShowStatus("Updating...");
        emit sigShowInfo(newInfo);
        downloadZip();
    }
}

int BackgroundThread::downloadZip(){
    newURL = "https://github.com/Instein98/PlayableLab/releases/download/V0.0.1/GameBuild.zip";
    QUrl zipURL = QUrl::fromUserInput(newURL);
    if (!zipURL.isValid()){
        emit sigShowError("Invalid URL", "The URL of target file is invalid!");
        return FAILURE;
    }
    networkManager = new QNetworkAccessManager;  // delete!!
    networkReply = networkManager->get(QNetworkRequest(zipURL));
//    connect(networkReply, SIGNAL(finished()), this, SLOT(onFinishDownload()));
    connect(networkReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateProgress(qint64, qint64)));
    emit sigShowStatus("downloadZip");
    return 1;
}

void BackgroundThread::updateProgress(qint64 dowloadedBytes, qint64 totalBytes){
    emit sigShowProgress(dowloadedBytes, totalBytes);
    emit sigShowStatus("updateProgress");
}
