@echo off  
echo �����ǰ����Ŀ¼: 
echo %cd%

rem ���������
echo *********************************************
set /p ProgramName=�����������: 
echo ѡ��ĳ�������%ProgramName%
if "%ProgramName%"=="" (
	echo ������Ϊ��
	pause
	exit
)

rem ѡ�����汾
echo *********************************************
echo ��ѡ�����İ汾����ȷ����Ӧ�汾�Ѿ��������
echo 1:Desktop_Qt_5_9_9_MinGW_32bit-Release
echo 2:Desktop_Qt_5_9_9_MSVC2017_64bit-Release
set /p VersionType=������1����2��
echo ѡ��İ汾��%VersionType%
if not %VersionType%==1 (if not %VersionType%==2 ( 
	echo ����ֵ����
	pause
	exit
) )
echo *********************************************

rem Qt�������·��
if %VersionType%==1 (
	set WINDEPLOYQT_EXE=D:\Qt\Qt5.9.9\5.9.9\mingw53_32\bin\windeployqt.exe
	set RELEASE_DIR=.\build-%ProgramName%-Desktop_Qt_5_9_9_MinGW_32bit-Release\release
) else (
	set WINDEPLOYQT_EXE=D:\Qt\Qt5.9.9\5.9.9\msvc2017_64\bin\windeployqt.exe
	set RELEASE_DIR=.\build-%ProgramName%-Desktop_Qt_5_9_9_MSVC2017_64bit-Release\release
)
echo windeployqt·��: %WINDEPLOYQT_EXE%
rem �ж�QT��������Ƿ����
if not exist %WINDEPLOYQT_EXE% (
	echo "WINDEPLOYQT_EXE �����ڣ���Ҫ�������á�"%WINDEPLOYQT_EXE%
	pause
	exit
)
rem �ж�QT�����Ŀ��·���Ƿ����
if not exist %RELEASE_DIR% (
	echo Ŀ��·�������ڣ�����Ŀ��·��������Ҫ�������á�
	echo %RELEASE_DIR%
	pause
	exit
)

if %VersionType%==1 (
	echo *********************************************
	echo ## Desktop_Qt_5_9_9_MinGW_32bit-Release ##
	rem �������Ŀ¼
	mkdir %ProgramName%-release
	rem ��������ļ�
	copy /Y %RELEASE_DIR%\%ProgramName%.exe %ProgramName%-release\
	
	rem ʹ��QT���������QT������
	%WINDEPLOYQT_EXE% %ProgramName%-release\%ProgramName%.exe --qmldir D:\Qt\Qt5.9.9\5.9.9\mingw53_32\qml
	if %errorlevel%==0 (
		echo ����QT������ɹ�
	) else (
		echo ����QT������ʧ��
		pause
		exit
	)
)

if %VersionType%==2 (
	echo *********************************************
	echo ## Desktop_Qt_5_9_9_MSVC2017_64bit-Release ##
	rem �������Ŀ¼
	mkdir %ProgramName%-release
	rem ��������ļ�
	copy /Y %RELEASE_DIR%\%ProgramName%.exe %ProgramName%-release\

	rem ʹ��QT���������QT������
	%WINDEPLOYQT_EXE% %ProgramName%-release\%ProgramName%.exe --qmldir D:\Qt\Qt5.9.9\5.9.9\msvc2017_64\qml
	if %errorlevel%==0 (
		echo ����QT������ɹ�
	)else (
		echo ����QT������ʧ��
		pause
		exit
	)
)

rem ����Cet�Զ����
mkdir %ProgramName%-release\plugins\
copy F:\sharefolder\cetqtlearn\CetToolLicense\CetTooslPlugin\plugins\CetLicensePlugin.dll %ProgramName%-release\plugins
copy F:\sharefolder\cetqtlearn\CetToolLicense\CetTooslPlugin\plugins\CetUpdatePlugin.dll %ProgramName%-release\plugins
echo ����Cet������ɹ�

echo ################## ����ɹ� ###################
echo ## VersionType=%VersionType%(1:MinGW_32bit,2:MSVC2017_64bit)
echo ## %ProgramName%.exe
echo #################################################
pause
exit

