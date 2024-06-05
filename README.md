# AZL Autopatcher

An autopatcher made for my project FormidableML.  
You will need to put each .so inside its correspondent folder inside a libs folder in the same folder as AZLAutopatcher.  
Example:  

Folder/  
├─ AzurLane.apk  
├─ AZLAutopatcher.exe  
├─ libs/  
│  ├─  arm64-v8a/  
│  ├─  ...  


```
Usage: ./azlautopatcher -a <APK_PATH> [-d]
Options:
  -a <APK_PATH>    Specify the path to the APK file.
  -d               Enable Android debug mode.
  -r               Reuse the extracted apk.
```
If you want to build it once built you will need aditional tools like apktool or zipalign to make it work correctly (also you will need a keystore to sign the apk)
