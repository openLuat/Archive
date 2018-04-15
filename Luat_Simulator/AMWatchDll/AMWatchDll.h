/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    AMWatchDll.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          
 *          
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/

#ifndef __AMWATCH_DLL_H__
#define __AMWATCH_DLL_H__

#if defined(_WIN32) && !defined(WIN32_PLATFORM_CONF)
	#if defined(AMWATCH_DLL)
		#define AMWATCH_DLL_API extern "C" _declspec(dllexport) 
	#else
		#define AMWATCH_DLL_API extern "C" _declspec(dllimport) 
	#endif
#else
	#define AMWATCH_DLL_API extern "C"
#endif

#endif //__AMWATCH_DLL_H__