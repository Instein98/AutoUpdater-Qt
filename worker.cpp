#include "worker.h"
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QTextCodec>
#include <private/qzipreader_p.h>


// the version info is stored in xml
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
    download(&xmlUrl, false, NEW_XML_NAME, MSG_CHECK_UPDATE);
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
                  emit sigShowStatus(MSG_LAUNCH);
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
//            qDebug() << "Network Error, statusCode: " << statusCode;
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
//    if (!checkNetworkOnline()){
//        emit sigShowError("Network Error", "Please check your internet connection!");
//        return FAILURE;
//    }
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
            // check game file
            QDir dir(newVersionInfo->dirName);
            QFileInfo zipInfo(newVersionInfo->zipName);
            if (dir.exists() || zipInfo.exists()){
                connect(this, SIGNAL(sigLaunchGame()), this, SLOT(launchGame()));
                emit sigShowProgress(1, 1);
                emit sigShowStatus(MSG_LAUNCH);
                emit sigLaunchGame();
            }else{
                // Download new game files
                emit sigShowInfo(newVersionInfo->updateInfo);
                QUrl url = QUrl::fromUserInput(newVersionInfo->url);
                download(&url, true, url.fileName(), MSG_UPDATE);
            }
        }else{
            // Delete old game files first
            QDir dir(oldVersionInfo->dirName);
            if (dir.exists()){
                deleteFileOrDirectory(dir.absolutePath());
            }
            QFileInfo zipInfo(oldVersionInfo->zipName);
            if (zipInfo.exists()){
                deleteFileOrDirectory(zipInfo.absolutePath());
            }
            // Download new game files
            emit sigShowInfo(newVersionInfo->updateInfo);
            QUrl url = QUrl::fromUserInput(newVersionInfo->url);
            download(&url, true, url.fileName(), MSG_UPDATE);
        }
    }else{
        emit sigShowInfo(newVersionInfo->updateInfo);
        QUrl url = QUrl::fromUserInput(newVersionInfo->url);
        download(&url, true, url.fileName(), MSG_UPDATE);
    }
}

// parse the xml file to the structure VersionInfo
void Worker::parseXML(QString fileName, VersionInfo *versionInfo){
    // Avoid chinese messy code
    QTextCodec *codec = QTextCodec::codecForName("UTF8");
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){  // will download the new version
        emit sigShowError("File I/O Erorr", "Can not open xml file!");
        return;
    }

    while(!file.atEnd()){
         QByteArray byteArray = file.readLine();
         QString line = codec->toUnicode(byteArray);
         int left;
         int right;
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
                     versionInfo->updateInfo = line.mid(left+len, right-left-len).replace("\\n", "\n");  // to support new line
                     break;
                 }
             }
         }
    }
    file.close();
}

// unzip the game zip file and execute the game
void Worker::launchGame(){
    QDir dir(newVersionInfo->dirName);
    if (!dir.exists()){
        QFileInfo zipInfo(newVersionInfo->zipName);
        if (!zipInfo.exists()){
            emit sigShowError("File Missing", "Can not find game files.");
            return;
        }
        if (!unzip(newVersionInfo->zipName)){
            return;
        }
    }

    // check whether the exe exist
    QFileInfo exeInfo(newVersionInfo->dirName+"\\"+newVersionInfo->exeName);
    if (!exeInfo.exists()){
        emit sigShowError("File Missing", "Can not find executable file.");
        return;
    }

    // should not delete this process, it still runs after GUI terminate
    QProcess *process = new QProcess();
    process->start(newVersionInfo->dirName+"\\"+newVersionInfo->exeName, QStringList());
    process->waitForFinished(1500);  // wait for 1.5s and quit GUI
    emit sigExit();
}

// unzip zip file compressed by deflate algorithm
bool Worker::unzip(QString fileName){
    QZipReader qZip(fileName);
    foreach(QZipReader::FileInfo item, qZip.fileInfoList()){
        if (item.isDir){
            QDir d(item.filePath);
            if (!d.exists())
                d.mkpath(item.filePath);
        }else if (item.isFile){
            QFile file(item.filePath);
            file.open(QIODevice::WriteOnly | QIODevice::Truncate);
            file.write(qZip.fileData(item.filePath));
            file.close();
        }
    }
    qZip.close();
    // if unzip failed, remove the damaged zip file
    QDir dir(newVersionInfo->dirName);
    if (!dir.exists()){
        deleteFileOrDirectory(newVersionInfo->zipName);
        emit sigShowError("Unzip Error", "The zip file may be damaged.");
        return false;
    }
    return true;
}

bool Worker::deleteFileOrDirectory(QString path){
    if (path.isEmpty() || !QDir().exists(path)){
        return false;
    }
    QFileInfo fileInfo(path);
    if (fileInfo.isFile()){
        return QFile::remove(path);
    }else if(fileInfo.isDir()){
        QDir dir(path);
        dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
        QFileInfoList fileList = dir.entryInfoList();
        foreach (QFileInfo fi, fileList){
            if (fi.isFile()){
                fi.dir().remove(fi.fileName());
            }else{
                deleteFileOrDirectory(fi.absoluteFilePath());
            }
        }
        dir.rmpath(dir.absolutePath());
    }
    return true;
}

