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
#define FS_FILE_NAME_UNICODE_LEN    (2*FS_FILE_NAME_LEN) 

// Max path size for unicode.
#define FS_PATH_UNICODE_LEN              (2*FS_PATH_LEN) 

// Length of terminated character('\0' for OEM).
#define LEN_FOR_NULL_CHAR                   1

// Size of terminated character('\0' for Unicode).
#define LEN_FOR_UNICODE_NULL_CHAR   2

/****************************** FILE SYSTEM ******************************/
typedef enum E_AMOPENAT_FILE_OPEN_FLAG_TAG
{
    SF_APPEND = (1 << 0),
    SF_TRUNC  = (1 << 1),
    SF_CREAT =  (1 << 2),
    SF_RDONLY = (1 << 3),
    SF_WRONLY = (1 << 4),
    SF_RDWR = (SF_RDONLY | SF_WRONLY),
    SF_DIRECT = (1 << 5),
    SF_EXCL  =(1 << 6)
}E_AMOPENAT_FILE_OPEN_FLAG;

typedef enum E_AMOPENAT_FILE_SEEK_FLAG_TAG
{
    // Seek from beginning of file.
    SF_SEEK_SET = 0,

    // Seek from current position.
    SF_SEEK_CUR = 1,

    // Set file pointer to EOF plus "offset"
    SF_SEEK_END = 2,
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
    UINT8   st_name[ FS_FILE_NAME_UNICODE_LEN + LEN_FOR_UNICODE_NULL_CHAR ]; // The name of file. 
}AMOPENAT_FS_FIND_DATA,*PAMOPENAT_FS_FIND_DATA;

typedef enum E_AMOPENAT_FS_ERR_CODE_TAG
{
    ERR_FS_NOT_MOUNTED = -10000,
    ERR_FS_FULL = -10001,
    ERR_FS_NOT_FOUND = -10002,
    ERR_FS_END_OF_OBJECT = -10003,
    ERR_FS_DELETED = -10004,
    ERR_FS_NOT_FINALIZED = -10005,
    ERR_FS_NOT_INDEX = -10006,
    ERR_FS_OUT_OF_FILE_DESCS = -10007,
    ERR_FS_FILE_CLOSED = -10008,
    ERR_FS_FILE_DELETED = -10009,
    ERR_FS_BAD_DESCRIPTOR = -10010,
    ERR_FS_IS_INDEX = -10011,
    ERR_FS_IS_FREE = -10012,
    ERR_FS_INDEX_SPAN_MISMATCH = -10013,
    ERR_FS_DATA_SPAN_MISMATCH = -10014,
    ERR_FS_INDEX_REF_FREE = -10015,
    ERR_FS_INDEX_REF_LU = -10016,
    ERR_FS_INDEX_REF_INVALID = -10017,
    ERR_FS_INDEX_FREE = -10018,
    ERR_FS_INDEX_LU = -10019,
    ERR_FS_INDEX_INVALID = -10020,
    ERR_FS_NOT_WRITABLE = -10021,
    ERR_FS_NOT_READABLE = -10022,
    ERR_FS_CONFLICTING_NAME = -10023,
    ERR_FS_NOT_CONFIGURED = -10024,
    ERR_FS_NOT_A_FS = -10025,
    ERR_FS_MOUNTED = -10026,
    ERR_FS_ERASE_FAIL = -10027,
    ERR_FS_MAGIC_NOT_POSSIBLE = -10028,
    ERR_FS_NO_DELETED_BLOCKS = -10029,
    ERR_FS_FILE_EXISTS = -10030,
    ERR_FS_NOT_A_FILE = -10031,
    ERR_FS_RO_NOT_IMPL = -10032,
    ERR_FS_RO_ABORTED_OPERATION = -10033,
    ERR_FS_PROBE_TOO_FEW_BLOCKS = -10034,
    ERR_FS_PROBE_NOT_A_FS = -10035,
    ERR_FS_NAME_TOO_LONG = -10036,
    ERR_FS_IX_MAP_UNMAPPED = -10037,
    ERR_FS_IX_MAP_MAPPED = -10038,
    ERR_FS_IX_MAP_BAD_RANGE = -10039,
    ERR_FS_SEEK_BOUNDS = -10040,
    ERR_FS_WRONG_PARAMETER = -10041,
    ERR_FS_PATH_NOT_FOUND  = -10042,
    ERR_FS_INTERNAL = -10050,
    ERR_FS_TEST = -10100,
    ERR_FS_OPEN_DEV_FAILED  = -10101,
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

