@echo off  
echo 输出当前工作目录: 
echo %cd%

rem 输入程序名
echo *********************************************
set /p ProgramName=请输入程序名: 
echo 选择的程序名：%ProgramName%
if "%ProgramName%"=="" (
	echo 程序名为空
	pause
	exit
)

rem 选择打包版本
echo *********************************************
echo 请选择打包的版本，并确保响应版本已经生成完成
echo 1:Desktop_Qt_5_9_9_MinGW_32bit-Release
echo 2:Desktop_Qt_5_9_9_MSVC2017_64bit-Release
set /p VersionType=请输入1或者2：
echo 选择的版本：%VersionType%
if not %VersionType%==1 (if not %VersionType%==2 ( 
	echo 输入值错误
	pause
	exit
) )
echo *********************************************

rem Qt部署程序路径
if %VersionType%==1 (
	set WINDEPLOYQT_EXE=D:\Qt\Qt5.9.9\5.9.9\mingw53_32\bin\windeployqt.exe
	set RELEASE_DIR=.\build-%ProgramName%-Desktop_Qt_5_9_9_MinGW_32bit-Release\release
) else (
	set WINDEPLOYQT_EXE=D:\Qt\Qt5.9.9\5.9.9\msvc2017_64\bin\windeployqt.exe
	set RELEASE_DIR=.\build-%ProgramName%-Desktop_Qt_5_9_9_MSVC2017_64bit-Release\release
)
echo windeployqt路径: %WINDEPLOYQT_EXE%
rem 判断QT部署程序是否存在
if not exist %WINDEPLOYQT_EXE% (
	echo "WINDEPLOYQT_EXE 不存在，需要重新设置。"%WINDEPLOYQT_EXE%
	pause
	exit
)
rem 判断QT程序的目标路径是否存在
if not exist %RELEASE_DIR% (
	echo 目标路径不存在，生成目标路径后，再需要重新设置。
	echo %RELEASE_DIR%
	pause
	exit
)

if %VersionType%==1 (
	echo *********************************************
	echo ## Desktop_Qt_5_9_9_MinGW_32bit-Release ##
	rem 创建软件目录
	mkdir %ProgramName%-release
	rem 复制软件文件
	copy /Y %RELEASE_DIR%\%ProgramName%.exe %ProgramName%-release\
	
	rem 使用QT部署程序打包QT依赖项
	%WINDEPLOYQT_EXE% %ProgramName%-release\%ProgramName%.exe --qmldir D:\Qt\Qt5.9.9\5.9.9\mingw53_32\qml
	if %errorlevel%==0 (
		echo 复制QT依赖库成功
	) else (
		echo 复制QT依赖库失败
		pause
		exit
	)
)

if %VersionType%==2 (
	echo *********************************************
	echo ## Desktop_Qt_5_9_9_MSVC2017_64bit-Release ##
	rem 创建软件目录
	mkdir %ProgramName%-release
	rem 复制软件文件
	copy /Y %RELEASE_DIR%\%ProgramName%.exe %ProgramName%-release\

	rem 使用QT部署程序打包QT依赖项
	%WINDEPLOYQT_EXE% %ProgramName%-release\%ProgramName%.exe --qmldir D:\Qt\Qt5.9.9\5.9.9\msvc2017_64\qml
	if %errorlevel%==0 (
		echo 复制QT依赖库成功
	)else (
		echo 复制QT依赖库失败
		pause
		exit
	)
)

rem 复制Cet自定义库
mkdir %ProgramName%-release\plugins\
copy F:\sharefolder\cetqtlearn\CetToolLicense\CetTooslPlugin\plugins\CetLicensePlugin.dll %ProgramName%-release\plugins
copy F:\sharefolder\cetqtlearn\CetToolLicense\CetTooslPlugin\plugins\CetUpdatePlugin.dll %ProgramName%-release\plugins
echo 复制Cet依赖库成功

echo ################## 打包成功 ###################
echo ## VersionType=%VersionType%(1:MinGW_32bit,2:MSVC2017_64bit)
echo ## %ProgramName%.exe
echo #################################################
pause
exit

