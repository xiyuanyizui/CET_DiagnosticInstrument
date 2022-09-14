; �йش��� Inno Setup �ű��ļ�����ϸ��������İ����ĵ���

#define MyAppName "CET Diagnostic Instrument"
#define MyAppVersion "2.1.3.193"
#define MyAppPublisher "cetxiyuan"
#define MyAppURL "http://www.cetxiyuan.com/"
#define MyAppExeName "CET_DiagnosticInstrument.exe"
#define MyAppIcon  "F:\sharefolder\cetqtlearn\CET_DiagnosticInstrument\CET_DiagnosticInstrument-setup\favorite.ico"
;ע�� Release-��ʽ��  Beta-���԰�
#define MyVersionTip "Beta"

[Setup]
; ע: AppId��ֵΪ������ʶ��Ӧ�ó���
; ��ҪΪ������װ����ʹ����ͬ��AppIdֵ��
; (��Ҫ�����µ� GUID�����ڲ˵��е�� "����|���� GUID"��)
AppId={{34BAF748-E848-4262-B7D6-54C0BD5A9F85}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DisableProgramGroupPage=yes
; ������ȡ��ע�ͣ����ڷǹ���װģʽ�����У���Ϊ��ǰ�û���װ����
;PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
OutputBaseFilename=CET_DiagnosticInstrument-Setup-{#MyVersionTip}-V{#MyAppVersion}
SetupIconFile={#MyAppIcon}
Compression=lzma
SolidCompression=yes
WizardStyle=modern
UninstallDisplayIcon={#MyAppIcon}

[Languages]
Name: "chinesesimp"; MessagesFile: "compiler:Default.isl"
Name: "english"; MessagesFile: "compiler:Languages\English.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "F:\sharefolder\cetqtlearn\CET_DiagnosticInstrument\CET_DiagnosticInstrument-release\CET_DiagnosticInstrument.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "F:\sharefolder\cetqtlearn\CET_DiagnosticInstrument\CET_DiagnosticInstrument-release\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; ע��: ��Ҫ���κι���ϵͳ�ļ���ʹ�á�Flags: ignoreversion��

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon; IconFilename: "{#MyAppIcon}"

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

