#include "iot_fs.h"

/*******************************************
**              FILE SYSTEM               **
*******************************************/

/**打开文件
*@param		pszFileNameUniLe:		文件全路径名称
*@param		iFlag:		打开标志详细请参见 E_AMOPENAT_FILE_OPEN_FLAG
*@return	INT32: 	    返回文件句柄
**/
INT32 iot_fs_open_file(                          
                        char* pszFileNameUniLe,
                        UINT32 iFlag          
                  )
{
    return IVTBL(open_file)(pszFileNameUniLe, iFlag, 0);
}

/**关闭文件
*@param		iFd:		文件句柄，open_file 或 create_file 返回的
*@return	INT32: 	    返回值小于0失败, 其余成功
**/
INT32 iot_fs_close_file(                      
                        INT32 iFd             
                   )
{
    return IVTBL(close_file)(iFd);
}

/**读取文件
*@param		iFd:		文件句柄，open_file 或 create_file 返回的
*@param		pBuf:		数据保存指针
*@param		iLen:		buf长度
*@return	INT32: 	    返回读取长度, 小于0表示失败,其余成功
**/
INT32 iot_fs_read_file(                    
                        INT32 iFd,          
                        UINT8 *pBuf,       
                        UINT32 iLen           
                  )
{
    return IVTBL(read_file)(iFd, pBuf, iLen);
}

/**写入文件
*@param		iFd:		文件句柄，open_file 或 create_file 返回的
*@param		pBuf:		需要写入的数据指针
*@param		iLen:		数据长度
*@return	INT32: 	    返回写入长度, 小于0表示失败,其余成功
**/
INT32 iot_fs_write_file(                    
                        INT32 iFd,              
                        UINT8 *pBuf,          
                        UINT32 iLen           
                   )
{
    return IVTBL(write_file)(iFd, pBuf, iLen);
}

/**立即写入flash
*@param		iFd:		文件句柄，open_file 或 create_file 返回的
*@return	INT32: 	    返回立即写入长度, 小于0表示失败,其余成功
**/
INT32 iot_fs_flush_file(                         
                        INT32 iFd             
                   )
{
    return IVTBL(flush_file)(iFd);
}

/**文件定位
*@note      参数iOffset的含义取决于iOrigin的值.
*@param		iFd:		文件句柄，open_file 或 create_file 返回的
*@param		iOffset:	偏移量
*@param		iOrigin:	参数详细请参见E_AMOPENAT_FILE_SEEK_FLAG
*@return	INT32: 	    返回文件的偏移量
**/
INT32 iot_fs_seek_file(                         
                        INT32 iFd,            
                        INT32 iOffset,        
                        UINT8 iOrigin           
                  )
{
    return IVTBL(seek_file)(iFd, iOffset, iOrigin);
}

/**创建文件
*@param		pszFileNameUniLe:	文件全路径名称
*@return	INT32: 	            返回文件句柄, 小于0表示失败,其余成功
**/
INT32 iot_fs_create_file(                        
                        char* pszFileNameUniLe       
                    )
{
    return IVTBL(create_file)(pszFileNameUniLe, 0);
}

/**删除文件
*@param		pszFileNameUniLe:	文件全路径名称
*@return	INT32: 	            返回值小于0表示失败,其余成功
**/
INT32 iot_fs_delete_file(                         
                        char* pszFileNameUniLe
                    )
{
    return IVTBL(delete_file)(pszFileNameUniLe);
}

/**改变文件大小
*@param		iFd:	文件句柄
*@param		uSize:	设置大小
*@return	INT32: 	返回值小于0表示失败,其余成功
**/
INT32 iot_fs_change_size(
                    INT32 iFd,
                    UINT32 uSize
                )
{
    return IVTBL(change_size)(iFd, uSize);
}

/**切换当前工作目录
*@param		pszDirNameUniLe:	目录路径
*@return	INT32: 	返回值小于0表示失败,其余成功
**/
INT32 iot_fs_change_dir(                         
                        char* pszDirNameUniLe  
                   )
{
    return IVTBL(change_dir)(pszDirNameUniLe);
}

/**创建目录
*@param		pszDirNameUniLe:	目录路径
*@param		iMode:	            目录属性，详细请参见E_AMOPENAT_FILE_ATTR_TAG
*@return	INT32: 	返回值小于0表示失败,其余成功
**/
INT32 iot_fs_make_dir(                            
                        char* pszDirNameUniLe,
                        UINT32 iMode            
                 )
{
    return IVTBL(make_dir)(pszDirNameUniLe, iMode);
}

/**删除目录
*@param		pszDirNameUniLe:	目录路径,该目录必须为空，接口才能返回成功
*@return	INT32: 	返回值小于0表示失败,其余成功
**/
INT32 iot_fs_remove_dir(                            
                        char* pszDirNameUniLe  
                   )
{
    return IVTBL(remove_dir)(pszDirNameUniLe);
}

/**获取当前路径
*@param		pCurDirUniLe:	目录路径
*@param		uUnicodeSize:	    存储目录信息空间大小
*@return	INT32: 	返回值小于0表示失败,其余成功
**/
INT32 iot_fs_get_current_dir(                      
                        char* pCurDirUniLe,  
                        UINT32 uUnicodeSize    
                        )
{
    return IVTBL(get_current_dir)(pCurDirUniLe, uUnicodeSize);
}


/**获取文件系统信息
*@param		devName:	查找文件句柄
*@return	fileInfo: 	返回值小于0表示失败,其余成功
**/
INT32 iot_fs_get_fs_info(                           
                        E_AMOPENAT_FILE_DEVICE_NAME       devName,           
                        T_AMOPENAT_FILE_INFO               *fileInfo                 
                   )
{
    return IVTBL(get_fs_info)(devName, fileInfo);
}
