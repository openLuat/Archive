::********************************************************#
:: Copyright (C), AirM2M Tech. Co., Ltd.
:: Author: lifei
:: Description: AMOPENAT ����ƽ̨
:: Others:
:: History: 
::   Version�� Date:       Author:   Modification:
::   V0.1      2012.12.14  lifei     �����ļ�
::********************************************************#
@echo off

call c:\CSDTK4\CSDTKvars.bat
::������Ŀ¼�л�����bat�ļ����ڵ�Ŀ¼
PUSHD %~dp0

::+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
:: ��ȡ��ص�·����Ϣ
::+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
::��ȡ��Ŀ����Ŀ¼·��
set PROJ_BUILD_DIR=%cd%

::��ȡ��Ŀ·��
cd ..
set PROJ_DIR=%cd:\=/%
:: ��ȡ��Ŀ����
for /f %%i in ("%cd%") do set PROJ_NAME=%%~ni
echo PROJ_NAME=%PROJ_NAME%

::��ȡ�����Ŀ¼
cd ../..
set ROOT_DIR=%cd:\=/%

::�ص���Ŀ����Ŀ¼
cd %PROJ_BUILD_DIR%


::+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
:: ���û���·��
::+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
:BuildStart
:echo Cygwin install path: %CYGWIN_HOME%
::��Cygwin��bin�Լ�cross-compile����Ŀ¼�ӵ�PATH��
set SOFT_WORKDIR=%ROOT_DIR%
set PATH=%SOFT_WORKDIR%/env/utils;%SOFT_WORKDIR%/env/win32;%SOFT_WORKDIR%;%PATH%;

::+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
set CT_COMPILER=gcc4_xcpu
:: ����bash��ʼ����
::+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
echo root_dir  %ROOT_DIR%
bash %ROOT_DIR%/cust_build.sh
echo.
goto End




:End

@echo on