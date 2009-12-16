
[Setup]
AppName=Stonesense
AppVerName=Stonesense a4.1
Compression=lzma
SolidCompression=yes
DefaultDirName={pf}\stonesense

[Files]
Source: "stonesense.exe"; DestDir: "{app}"
Source: "*.dll"; DestDir: "{app}"; Excludes: "dfhack-debug.dll"
Source: "*.png"; DestDir: "{app}"; Flags: recursesubdirs
Source: "changelog.txt"; DestDir: "{app}"
Source: "init.txt"; DestDir: "{app}"
Source: "README.txt"; DestDir: "{app}"; Flags: isreadme
Source: "*.xml"; DestDir: "{app}"; Flags: recursesubdirs
Source: "index.txt"; DestDir: "{app}"; Flags: recursesubdirs






