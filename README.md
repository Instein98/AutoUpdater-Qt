# AutoUpdater-Qt

### Introduction

It is a launcher/updater for any executable program in windows (havn't been tested under other OS). It uses xml file to record the version information. First it downloads the newest xml file to check if the current version is newest, if no, it downloads the program from the link provided in the newest xml file and execute it.

### Usage

1. You need to specify the xml link in the header file *worker.h*.  You may specify that in configure file in later version.

2. The structure for xml file should be like this:

   ```xml
   <release>
   	<version>0.0.1</version>	     
       <url>The URL of zip file</url>
       <zipName>The name of the zip file</zipName>
       <dirName>The name of the folder in zip file</dirName>
       <exeName>The name of the executable in the folder</exeName>
       <log>The update log(optional)</log>
   </release>
```
   
3. You should build this project on your own. You can open this project in Qt Creator. The binary will release in later version.