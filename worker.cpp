#include "worker.h"

Worker::Worker(){

}

void Worker::doWork(){
    bool needUpdate = 1;
//    bool needUpdate = checkUpdate();
    if (needUpdate){
        emit sigShowInfo(newInfo);
        newURL = QUrl::fromUserInput("https://github.com/Instein98/PlayableLab/releases/download/V0.0.1/GameBuild.zip");
        download(&newURL, true, "Testing...");
    }
}

//int Worker::downloadZip(){

//    QUrl zipURL = QUrl::fromUserInput(newURL);
//    if (!zipURL.isValid()){
//        emit sigShowError("Invalid URL", "The URL of target file is invalid!");
//        return FAILURE;
//    }
//    downloadedFile = new QFile(zipURL.fileName());
//    if (!downloadedFile->open(QIODevice::WriteOnly)){
//        emit sigShowError("Error", "Can not open the temporary file!");
//        return FAILURE;
//    }

//    networkManager = new QNetworkAccessManager;
//    networkReply = networkManager->get(QNetworkRequest(zipURL));

//    connect(networkReply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
//    connect(networkReply, SIGNAL(finished()), this, SLOT(onFinishDownload()));
//    connect(networkReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateProgress(qint64, qint64)));
//    emit sigShowStatus("Updating...");
//    return SUCCESS;
//}

void Worker::updateProgress(qint64 dowloadedBytes, qint64 totalBytes){
    // show progress only when the request is OK and wantShowProgress is true
    if (!showProgress || networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()!=200){
        return;
    }
    emit sigShowProgress(dowloadedBytes, totalBytes);
}

void Worker::onReadyRead(){
    downloadedFile->write(networkReply->readAll());
}

void Worker::onFinishDownload(){
    int statusCode = networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "statusCode: " << statusCode;
    qDebug() << "RedirectionTarget: " << networkReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    switch (statusCode) {
        case 200:  // OK
            emit sigShowStatus("Update complete; Launching the game...");
            downloadedFile->close();
            delete downloadedFile;
            networkReply->deleteLater();
            networkManager->deleteLater();
        break;

        case 302:  // Redirection
            newURL = networkReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
            networkReply = networkManager->get(QNetworkRequest(newURL));
            connect(networkReply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
            connect(networkReply, SIGNAL(finished()), this, SLOT(onFinishDownload()));
            connect(networkReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateProgress(qint64, qint64)));
        break;
    }
}

int Worker::download(QUrl *url, bool wantShowProgress, QString status){
    showProgress = wantShowProgress;
    if (!url->isValid()){
        emit sigShowError("Invalid URL", "The URL of target file is invalid!");
        return FAILURE;
    }
    downloadedFile = new QFile(url->fileName());
    if (!downloadedFile->open(QIODevice::WriteOnly)){
        emit sigShowError("File I/O Error", "Can not open the temporary file!");
        return FAILURE;
    }

    networkManager = new QNetworkAccessManager;
    networkReply = networkManager->get(QNetworkRequest(*url));
    connect(networkReply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(networkReply, SIGNAL(finished()), this, SLOT(onFinishDownload()));
    connect(networkReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateProgress(qint64, qint64)));
    emit sigShowStatus(status);
    return SUCCESS;
}

//inline void Worker::setupDownload(QNetworkReply *networkReply, QNetworkAccessManager *networkManager, QUrl *url, QObject *receiver){
//    networkReply = networkManager->get(QNetworkRequest(*url));
//    connect(networkReply, SIGNAL(readyRead()), receiver, SLOT(onReadyRead()));
//    connect(networkReply, SIGNAL(finished()), receiver, SLOT(onFinishDownload()));
//    connect(networkReply, SIGNAL(downloadProgress(qint64,qint64)), receiver, SLOT(updateProgress(qint64, qint64)));
//}
