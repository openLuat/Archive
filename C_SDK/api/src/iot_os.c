#include "iot_os.h"

/*******************************************
**                 SYSTEM                 **
*******************************************/

/**创建线程
*@note  nPriority值的返回在0-20, 值越大优先级越低
*@param	pTaskEntry:		线程主函数
*@param	pParameter:		作为参数传递给线程主函数
*@param	nStackSize: 	线程栈大小
*@param	nPriority: 		线程优先级，该参数越大，线程优先级越低
*@param nCreationFlags: 线程启动标记， 请参考E_AMOPENAT_OS_CREATION_FLAG
*@param pTaskName: 		线程名称
*@return	HANDLE: 	创建成功返回线程句柄
**/
HANDLE iot_os_create_task(                         
                            PTASK_MAIN pTaskEntry, 
                            PVOID pParameter,     
                            UINT16 nStackSize,      
                            UINT8 nPriority,        
                            UINT16 nCreationFlags, 
                            PCHAR pTaskName         
                          )
{
    return IVTBL(create_task)(pTaskEntry, pParameter, NULL, 
                nStackSize, nPriority+OPENAT_CUST_TASKS_PRIORITY_BASE, nCreationFlags, 0, pTaskName); 
}
             
/**启动线程
*@param	hTask:		线程句柄，create_task接口返回值
*@param	pParameter:		作为参数传递给线程主函数，会替换create_task接口传入的pParameter参数
**/
VOID iot_os_start_task(                         
                        HANDLE hTask,         
                        PVOID pParameter      
                      )
{
    IVTBL(start_task)(hTask, pParameter);
}

/**停止线程
*@param	hTask:		线程句柄
**/	
VOID iot_os_stop_task(                             
                        HANDLE hTask            
                 )
{
    IVTBL(stop_task)(hTask);
}

/**删除线程
*@param		hTask:		线程句柄
*@return	TURE:		删除线程成功
*			FALSE: 		删除线程失败
**/	
BOOL iot_os_delete_task(                           
                        HANDLE hTask            
                   )
{
    return IVTBL(delete_task)(hTask);
}

/**挂起线程
*@param		hTask:		线程句柄
*@return	TURE: 		挂起线程成功
*			FALSE  : 	挂起线程失败
**/
BOOL iot_os_suspend_task(                      
                            HANDLE hTask          
                        )
{
    return IVTBL(suspend_task)(hTask);
}

/**恢复线程
*@param		hTask:		线程句柄
*@return	TURE: 		恢复线程成功
*			FALSE  : 	恢复线程失败
**/
BOOL iot_os_resume_task(                           
                        HANDLE hTask          
                   )
{
    return IVTBL(resume_task)(hTask);
}

/**获取当前线程
*@return	HANDLE:		返回当前线程句柄
*
**/		
HANDLE iot_os_current_task(                         
                            VOID
                          )
{
    return IVTBL(current_task)();
}

/**获取当前线程创建信息
*@param		hTask:		线程句柄
*@param		pTaskInfo:		线程信息存储接口
*@return	TURE: 		成功
*			FALSE  : 	失败
**/
BOOL iot_os_get_task_info(                         
                            HANDLE hTask,         
                            T_AMOPENAT_TASK_INFO *pTaskInfo 
                         )
{
    return IVTBL(get_task_info)(hTask, pTaskInfo);
}




/**获取线程消息
*@note 会阻塞
*@param		hTask:		线程句柄
*@param		ppMessage:	存储消息指针
*@return	TURE: 		成功
*			FALSE  : 	失败
**/
BOOL iot_os_wait_message(                          
                        HANDLE hTask,         
                        PVOID* ppMessage    
                    )
{
    return IVTBL(wait_message)(hTask, ppMessage, 0);
}

/**发送线程消息
*@note 添加到消息队列尾部
*@param		hTask:		线程句柄
*@param		pMessage:	存储消息指针
*@return	TURE: 		成功
*			FALSE  : 	失败
**/
BOOL iot_os_send_message(                       
                        HANDLE hTask,         
                        PVOID pMessage         
                    )
{
    return IVTBL(send_message)(hTask, pMessage);
}

/**发送高优先级线程消息
*@note      添加到消息队列头部
*@param		hTask:		线程句柄
*@param		pMessage:	存储消息指针
*@return	TURE: 		成功
*			FALSE  : 	失败
**/
BOOL iot_os_send_high_priority_message(          
                        HANDLE hTask,          
                        PVOID pMessage         
                                  )
{
    return IVTBL(send_high_priority_message)(hTask, pMessage);
}

/**检测消息队列中是否有消息
*@param		hTask:		线程句柄
*@return	TURE: 		成功
*			FALSE  : 	失败
**/
BOOL iot_os_available_message(                     
                        HANDLE hTask           
                         )
{
    return IVTBL(available_message)(hTask);
}


/**创建定时器
*@param		pFunc:			定时器到时处理函数
*@param		pParameter:		作为参数传递给定时器到时处理函数
*@return	HANDLE: 		返回定时器句柄
*			
**/	
HANDLE iot_os_create_timer(                        
                        PTIMER_EXPFUNC pFunc,   
                        PVOID pParameter        
                      )
{
    return IVTBL(create_timer)(pFunc, pParameter);
}

/**启动定时器
*@param		hTimer:				定时器句柄，create_timer接口返回值
*@param		nMillisecondes:		定时器时间
*@return	TURE: 				成功
*			FALSE  : 			失败
**/
BOOL iot_os_start_timer(                         
                        HANDLE hTimer,         
                        UINT32 nMillisecondes   
                   )
{
    return IVTBL(start_timer)(hTimer, nMillisecondes);
}

/**停止定时器
*@param		hTimer:				定时器句柄，create_timer接口返回值
*@return	TURE: 				成功
*			FALSE  : 			失败
**/	
BOOL iot_os_stop_timer(                          
                        HANDLE hTimer
                    )
{
    return IVTBL(stop_timer)(hTimer);
}

/**删除定时器
*@param		hTimer:				定时器句柄，create_timer接口返回值
*@return	TURE: 				成功
*			FALSE  : 			失败
**/	
BOOL iot_os_delete_timer(                         
                        HANDLE hTimer          
                    )
{
    return IVTBL(delete_timer)(hTimer);
}

/**检查定时器是否已经启动
*@param		hTimer:				定时器句柄，create_timer接口返回值
*@return	TURE: 				成功
*			FALSE  : 			失败
**/
BOOL iot_os_available_timer(                      
                        HANDLE hTimer         
                       )
{
    return IVTBL(available_timer)(hTimer);
}

/**获取系统时间
*@param		pDatetime:		存储时间指针
*@return	TURE: 			成功
*			FALSE  : 		失败
**/	
BOOL iot_os_get_system_datetime(                   
                        T_AMOPENAT_SYSTEM_DATETIME* pDatetime
                       )
{
    return IVTBL(get_system_datetime)(pDatetime);
}

/**设置系统时间
*@param		pDatetime:		存储时间指针
*@return	TURE: 			成功
*			FALSE  : 		失败
**/	
BOOL iot_os_set_system_datetime(                   
                        T_AMOPENAT_SYSTEM_DATETIME* pDatetime
                       )
{
    return IVTBL(set_system_datetime)(pDatetime);
}



BOOL iot_os_init_alarm(                                       
                        T_AMOPENAT_ALARM_CONFIG *pConfig 
                   )
{
    return IVTBL(init_alarm)(pConfig);
}

BOOL iot_os_set_alarm(                                        
                        T_AMOPENAT_ALARM_PARAM *pAlarmSet    
                   )
{
    return IVTBL(set_alarm)(pAlarmSet);
}

HANDLE iot_os_enter_critical_section(            
                        VOID
                                )
{
    return IVTBL(enter_critical_section)(); 
}

VOID iot_os_exit_critical_section(                
                        HANDLE hSection       
                             )
{
    IVTBL(exit_critical_section)(hSection); 
}

HANDLE iot_os_create_semaphore(                   
                        UINT32 nInitCount     
                          )
{
    return IVTBL(create_semaphore)(nInitCount); 
}

BOOL iot_os_delete_semaphore(                      
                        HANDLE hSem            
                        )
{
    return IVTBL(delete_semaphore)(hSem);  
}

BOOL iot_os_wait_semaphore(                  
                        HANDLE hSem,           
                        UINT32 nTimeOut        
                      )
{
    return IVTBL(wait_semaphore)(hSem, nTimeOut);
}

BOOL iot_os_release_semaphore(
                        HANDLE hSem           
                         )
{
    return IVTBL(release_semaphore)(hSem);
}

UINT32 iot_os_get_semaphore_value (                  
                        HANDLE hSem            
                        )
{
    return IVTBL(get_semaphore_value)(hSem);
}


PVOID iot_os_malloc(                                
                        UINT32 nSize           
               )
{
    return IVTBL(malloc)(nSize);
}

PVOID iot_os_realloc(                            
                        PVOID pMemory,          
                        UINT32 nSize           
                )
{
    return IVTBL(realloc)(pMemory, nSize);
}

VOID iot_os_free(                                  
                        PVOID pMemory          
            )
{
    IVTBL(free)(pMemory);
}


BOOL iot_os_sleep(                                 
                        UINT32 nMillisecondes  
             )
{
    return IVTBL(sleep)(nMillisecondes*16384/1000);
}

UINT32 iot_os_get_system_tick(                   
                        VOID
                         )
{
    return IVTBL(get_system_tick)();
}

UINT32 iot_os_rand(                                
                        VOID
              )
{
    return IVTBL(rand)();
}

VOID iot_os_srand(                              
                        UINT32 seed           
             )
{
    IVTBL(srand)(seed);
}

VOID iot_os_shut_down(                             
                        VOID
                 )
{
    IVTBL(shut_down)();
}

VOID iot_os_restart(                              
                        VOID
               )
{
    IVTBL(restart)();
}

VOID iot_os_sys_request_freq(                     
                        E_AMOPENAT_SYS_FREQ freq
               )
{
    IVTBL(sys_request_freq)(freq);
}
