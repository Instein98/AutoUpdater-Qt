#include "worker.h"

#include <QFileInfo>

struct Worker::VersionInfo{
    QString versionTag;
    QString url;
    QString zipName;
    QString dirName;
    QString exeName;
    QString updateInfo;
};

Worker::Worker(){
    oldVersionInfo = new VersionInfo;
    newVersionInfo = new VersionInfo;
}

Worker::~Worker(){
    delete oldVersionInfo;
    delete newVersionInfo;
}

void Worker::doWork(){  // start the work
    currentStage = DownloadXML;
    QUrl xmlUrl = QUrl::fromUserInput(XML_URL);
    download(&xmlUrl, false, NEW_XML_NAME, "Checking for updates...");
}

void Worker::updateProgress(qint64 dowloadedBytes, qint64 totalBytes){
    int statusCode = networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (showProgress && statusCode == 200){  // show progress only when the request is OK and wantShowProgress is true
        emit sigShowProgress(dowloadedBytes, totalBytes);
    }
}

void Worker::onReadyRead(){
    int statusCode = networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(statusCode == 200){
        downloadedFile->write(networkReply->readAll());
    }
}

void Worker::onFinishDownload(){
    int statusCode = networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    switch (statusCode) {
        case 200:  // OK
            downloadedFile->close();
            delete downloadedFile;
            networkReply->deleteLater();
            networkManager->deleteLater();
            switch (currentStage){
              case DownloadXML:
                  connect(this, SIGNAL(sigCheckUpdate()), this, SLOT(checkUpdate()));
                  currentStage = DownloadGame;
                  emit sigCheckUpdate();
                  break;
              case DownloadGame:
                  connect(this, SIGNAL(sigLaunchGame()), this, SLOT(launchGame()));
                  currentStage = LaunchGame;
                  if (QFile::exists(OLD_XML_NAME)){
                      QFile::remove(OLD_XML_NAME);
                  }
                  QFile::rename(NEW_XML_NAME, OLD_XML_NAME);  // replace the old xml with the new one
                  emit sigShowStatus("Update complete; Launching the game...");
                  emit sigLaunchGame();
                  break;
              default:
                  break;
            }
            break;
        case 302:{  // Redirection, should try another time
            QUrl url = networkReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
            networkReply = networkManager->get(QNetworkRequest(url));
            connect(networkReply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
            connect(networkReply, SIGNAL(finished()), this, SLOT(onFinishDownload()));
            connect(networkReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateProgress(qint64, qint64)));
            break;
        }
        default:  // Other cases
            qDebug() << "Network Error, statusCode: " << statusCode;
            emit sigShowError("Network Error", "HTTP status codes: " + QString::number(statusCode));
            downloadedFile->close();
            delete downloadedFile;
            networkReply->deleteLater();
            networkManager->deleteLater();
            break;
    }
}

int Worker::download(QUrl *url, bool wantShowProgress, QString fileName, QString status){
    showProgress = wantShowProgress;
    if (!url->isValid()){
        emit sigShowError("Invalid URL", "The URL of target file is invalid!");
        return FAILURE;
    }
    downloadedFile = new QFile(fileName);
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

void Worker::checkUpdate(){
    QFileInfo oldXmlInfo(OLD_XML_NAME);
    QFileInfo newXmlInfo(NEW_XML_NAME);
    if (!newXmlInfo.isFile()){
        sigShowError("File Not Found", "Can not find newest XML file!");
        return;
    }
    parseXML(NEW_XML_NAME, newVersionInfo);
    if (oldXmlInfo.isFile()){
        parseXML(OLD_XML_NAME, oldVersionInfo);
        if (QString::compare(oldVersionInfo->versionTag, newVersionInfo->versionTag)==0){  // already latest
            emit sigShowStatus("Game is already up to date; Launching game...");
            emit sigLaunchGame();
        }else{
            QUrl url = QUrl::fromUserInput(newVersionInfo->url);
            download(&url, true, url.fileName(), "Updating...");
        }
    }else{
        QUrl url = QUrl::fromUserInput(newVersionInfo->url);
        download(&url, true, url.fileName(), "Updating...");
    }
}

void Worker::parseXML(QString fileName, VersionInfo *versionInfo){
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){  // will download the new version
        emit sigShowError("File I/O Erorr", "Can not open xml file!");
        return;
    }
    QString line;
    QTextStream textStream(&file);
    int left;
    int right;
    while(!textStream.atEnd()){
        line = textStream.readLine();
        for (int i = 0; i < 6; i++){
            if ((left = line.indexOf(xmlStartTag[i]))!=-1){
                right = line.indexOf(xmlEndTag[i]);
                int len = xmlStartTag[i].length();
                switch(i){
                case 0:
                    versionInfo->versionTag = line.mid(left+len, right-left-len);
                    break;
                case 1:
                    versionInfo->url = line.mid(left+len, right-left-len);
                    break;
                case 2:
                    versionInfo->zipName = line.mid(left+len, right-left-len);
                    break;
                case 3:
                    versionInfo->dirName = line.mid(left+len, right-left-len);
                    break;
                case 4:
                    versionInfo->exeName = line.mid(left+len, right-left-len);
                    break;
                case 5:
                    versionInfo->updateInfo = line.mid(left+len, right-left-len);
                    break;
                }
            }
        }
    }
    file.close();
}

void Worker::launchGame(){

}

