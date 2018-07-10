/*********************************************************
  Copyright (C), AirM2M Tech. Co., Ltd.
  Author: lifei
  Description: AMOPENAT 开放平台
  Others:
  History: 
    Version： Date:       Author:   Modification:
    V0.1      2012.12.14  lifei     创建文件
*********************************************************/
#ifndef AM_OPENAT_FS_H
#define AM_OPENAT_FS_H

#include "am_openat_common.h"


//--------------------------------------------------------------------------------------------------
// Length or number define.
//--------------------------------------------------------------------------------------------------

// Folder or File name size in byts, to support fat long file name.
#define FS_FILE_NAME_LEN                     255 

// Max path length,to support fat long file name.
#define FS_PATH_LEN                               260 

// Max number of  open file
#define FS_NR_OPEN_FILES_MAX             32

// Max number of directory layer.
#define FS_NR_DIR_LAYERS_MAX             16

// Max folder or File name size in byts for uincode.

// Max path size for unicode.

// Length of terminated character('\0' for OEM).
#define LEN_FOR_NULL_CHAR                   1

// Size of terminated character('\0' for Unicode).

/****************************** FILE SYSTEM ******************************/
typedef enum E_AMOPENAT_FILE_OPEN_FLAG_TAG
{
    O_APPEND = (8),
    O_TRUNC  = 0x400,
    O_CREAT =  0x200,
    O_RDONLY = (0),
    O_WRONLY = (1),
    O_RDWR = (2),
    O_EXCL  = 0x800
}E_AMOPENAT_FILE_OPEN_FLAG;

typedef enum E_AMOPENAT_FILE_SEEK_FLAG_TAG
{
    // Seek from beginning of file.
    SEEK_SET = 0,

    // Seek from current position.
    SEEK_CUR = 1,

    // Set file pointer to EOF plus "offset"
    SEEK_END = 2,
}E_AMOPENAT_FILE_SEEK_FLAG;

#define OPENAT_VALIDATE_FILE_HANDLE_START 0

//--------------------------------------------------------------------------------------------------
// File attribute define.
//--------------------------------------------------------------------------------------------------
typedef enum E_AMOPENAT_FILE_ATTR_TAG
{
    FS_ATTR_DEFAULT     = 0x0,
    // read-only 
    FS_ATTR_RO          = 0x00000001,
    // hidden 
    FS_ATTR_HIDDEN      = 0x00000002,
    // system 
    FS_ATTR_SYSTEM      = 0x00000004, 
    // volume label 
    FS_ATTR_VOLUME      = 0x00000008,
    // directory 
    FS_ATTR_DIR         = 0x00000010,
    // archived 
    FS_ATTR_ARCHIVE     = 0x00000020,
}E_AMOPENAT_FILE_ATTR;

//--------------------------------------------------------------------------------------------------
// Find file information.
//--------------------------------------------------------------------------------------------------
typedef struct T_AMOPENAT_FS_FIND_DATA_TAG
{
    UINT32  st_mode;     // Mode of file 
    UINT32  st_size;       // For regular files, the file size in bytes
    UINT32  atime;    // Time of last access to the file
    UINT32  mtime;   // Time of last data modification
    UINT32  ctime;    // Time of last status(or inode) change
    UINT8   st_name[ FS_FILE_NAME_LEN + LEN_FOR_NULL_CHAR ]; // The name of file. 
}AMOPENAT_FS_FIND_DATA,*PAMOPENAT_FS_FIND_DATA;

typedef enum E_AMOPENAT_FS_ERR_CODE_TAG
{
    ERR_FS_OK = 0,
    ERR_FS_FILE_EXIST        = -4200001,
    ERR_FS_NO_DEVICE         = -4200002,
    ERR_FS_NO_SPACE          = -4200003,
    ERR_FS_ASSERTION_FAIL    = -4200004,
    ERR_FS_NOT_INIT          = -4200005,
    ERR_FS_NO_ENTRY          = -4200006,
    ERR_FS_NO_DIR            = -4200007,
    ERR_FS_TOO_MANY_FILES    = -4200008,
    ERR_FS_INVALID_PARAMETER = -4200009,
    ERR_FS_DENIED            = -4200010,
    ERR_FS_WRITE_PROTECTED   = -4200011,
    ERR_FS_NO_MORE_MEMORY    = -4200012,
    ERR_FS_DEAD_LOCK         = -4200013,
    ERR_FS_TIMEOUT           = -4200014,
    ERR_FS_INVALID_OBJECT    = -4200015,
    ERR_FS_IO                = -4200016,
    ERR_FS_NOT_IMPLEMENTED   = -4200017,
    ERR_FS_ERANGE            = -4200018,
    ERR_FS_DIR_NOT_EMTPY     = -4200019,
    ERR_FS_OPERATION_NOT_IMP = -4200020,
    ERR_FS_DISK_NO_SPACE     = -4200021,
}E_AMOPENAT_FS_ERR_CODE;

typedef struct T_AMOPENAT_TFLASH_INIT_PARAM_TAG
{
/*+\BUG WM-719\maliang\2013.3.21\文件系统接口和播放音频文件接口的文件名改为unicode little ending类型*/
    const WCHAR* pszMountPointUniLe; // T卡挂载节点,访问时T卡文件均在此目录下 UNICODE little endian
/*-\BUG WM-719\maliang\2013.3.21\文件系统接口和播放音频文件接口的文件名改为unicode little ending类型*/
}AMOPENAT_TFLASH_INIT_PARAM, *PAMOPENAT_TFLASH_INIT_PARAM;

/*+\NewReq WM-743\maliang\2013.3.28\[OpenAt]增加接口获取文件系统信息*/
typedef enum E_AMOPENAT_FILE_DEVICE_NAME_TAG
{
    E_AMOPENAT_FS_INTERNAL,
    E_AMOPENAT_FS_SDCARD
}E_AMOPENAT_FILE_DEVICE_NAME;

/** 文件系统信息
*/
typedef struct T_AMOPENAT_FILE_INFO_TAG
{
    UINT64  totalSize;    ///< Total size
    UINT64  usedSize;     ///< Has used  size 
}T_AMOPENAT_FILE_INFO;
/*-\NewReq WM-743\maliang\2013.3.28\[OpenAt]增加接口获取文件系统信息*/
/*+\BUG\AMOPENAT-74\brezen\2013.9.24\添加FLASH NV接口，用来适应掉电机制*/
#define  NV_SUCCESS                (0)
#define  NV_ERR_NO_MORE_MEM        (-1)
#define  NV_ERR_WRITE_FLASH        (-2)
#define  NV_ERR_READ_FLASH         (-3)
#define  NV_ERR_ERASE_FLASH        (-4)
#define  NV_ERR_CH_SECTOR          (-5)
#define  NV_ERR_ADD_PBD            (-6)
#define  NV_ERR_VTB_UNKNOWN_STATUS (-7)
#define  NV_ERR_DEVICE_BUSY        (-8)
#define  NV_ERR_OTHER              (-9)
#define  NV_ERR_NV_NOT_FOUND       (-10)
#define  NV_ERR_DATA_ERR           (-11)
#define  NV_ERR_NV_ALREADY_EXIST   (-12)
#define  NV_ERR_WRITE_FLASH_TIMEOUT (-13)
#define  NV_ERR_ERASE_FLASH_TIMEOUT (-14)
#define  NV_ERR_OPER_NOT_SUPPORT    (-15)
/*-\BUG\AMOPENAT-74\brezen\2013.9.24\添加FLASH NV接口，用来适应掉电机制*/
#endif /* AM_OPENAT_FS_H */

