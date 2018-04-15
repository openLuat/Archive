/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    w32_socket.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          This file is intends for Socket APIs on WIN32 environment.
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "assert.h"
#include "platform_rtos.h"
#include "w32_socket.h"
#include "winsock2.h"

#define _W32_SOCKET_C_

/* if you want to print dbg info in console, pls turn on SOC_DBG_ENABLE */
//#define SOC_DBG_ENABLE
#define SOC_MAX_APP_NWK_NUM  50
#define SOC_SMS_ID -2
#define SOC_DNS_SOC_ID  -3

void soc_hostname_ind(BOOL result,
                 INT32 request_id,
                 UINT8 access_id,
                 UINT8 *ip_addr,
                 UINT8 addr_len,
                 INT8 error_cause,
                 INT32 detail_cause);

INT8 socgethostbyaddr(char *domain_name,
                               UINT32 *domain_name_len,
                               UINT8 *addr,
                               UINT8 addr_len);

void soc_start_timer(UINT8 timer_id,
                         INT8 invoke_id,
                         UINT8 sub_id,
                         UINT32 time_out);

INT8 sooconnect(INT8 s, sockaddr_struct *addr);
void soc_notify_thread(void);
static void soc_notify_app(int socketid,
                    soc_event_enum event,
                    BOOL result,
                    INT8 cause);
static int soc_find_soc_app_slot(int app_id,
                          int nwk_account_id, 
                          int *idx);

static int soc_find_free_soc_app_slot(int *idx);

typedef enum
{
    W32_BEARER_CLOSED         = 0,
    W32_BEARER_OPENED         = 1,
    W32_BEARER_OPENING        = 2,
    W32_BEARER_CLOSING        = 4
} bearer_state_enum;

typedef struct
{
    int app_id;
	int nwk_account_id;
    int force_to_release;
    bearer_state_enum state;
} soc_app_nwk_state_struct;

typedef struct
{
    BOOL is_blocking;
    INT32 request_id;
    char *domain_name;
    UINT8 addr[16];
    UINT8 addr_len;
    UINT8 access_id;
    UINT32 nwk_account_id;
} soc_app_hostname_struct;

static HANDLE soc_lock;
#define SOC_LOCK          WaitForSingleObject(soc_lock, INFINITE)
#define SOC_UNLOCK        ReleaseSemaphore(soc_lock, 1, NULL)

static fd_set read_fd, write_fd, except_fd;
static struct timeval soc_timeout_win32;

int last_socket_id = -1;
int soc_free[MAX_IP_SOCKET_NUM];    /* storing socket id */
int soc_state[MAX_IP_SOCKET_NUM];   /* state of each socket */
int soc_type[MAX_IP_SOCKET_NUM];

static int soc_appid[MAX_IP_SOCKET_NUM];

sockaddr_struct soc_peer_addr[MAX_IP_SOCKET_NUM];
static int soc_account_id[MAX_IP_SOCKET_NUM]; /* storing network account id */

static int soc_options[MAX_IP_SOCKET_NUM];
static int soc_events[MAX_IP_SOCKET_NUM];
static int soc_want_events[MAX_IP_SOCKET_NUM];
static soc_app_nwk_state_struct soc_app_nwk[SOC_MAX_APP_NWK_NUM];
static soc_app_hostname_struct soc_host_name[MAX_IP_SOCKET_NUM];
static int created_socket_num = 0;      /* total created sockets */
static int soc_polling_timer_running = 0;
static int csd_timer_val = 0;

typedef enum
{
    W32_SOC_S_NOUSE             = 0,
    W32_SOC_S_INUSE             = 0x0001,
    W32_SOC_S_CONNECTING        = 0x0200,   /* in process of connecting to peer */
    W32_SOC_S_CONNECTED         = 0x0400,   /* socket connected to a peer */
    W32_SOC_S_SENDING           = 0x0800,
    W32_SOC_S_RECVING           = 0x1000,
    W32_SOC_S_FINED             = 0x00200000,     /* received TCP FIN from peer */
    W32_SOC_S_DNS_RESOLVING     = 0x00400000,     /* resolving DNS address */
    W32_SOC_S_CANTRCVMORE       = 0x00800000	/* cannot receive data */
} soc_state_enum;


static unsigned char state_str[][16] = {
    "ACTIVATING",
    "ACTIVATED",
    "DEACTIVATED",
    "DEACTIVATING",
    "UNKNOWN"
};

static unsigned char event_str[][16] = {
    "1 READ",
    "2 WRITE",
    "4 ACCEPT",
    "8 CONNECT",
    "16 CLOSE",
    "UNKNOWN"
};

static unsigned char winsock_errstr[][32] = {
    "0 WSABASEERR",            /* No Error */
    "10004 WSAEINTR",          /* Interrupted system call */
    "10009 WSAEBADF",          /* Bad file number */
    "10013 WSAEACCES",         /* Permission denied */
    "10014 WSAEFAULT",         /* Bad address */
    "10022 WSAEINVAL",         /* Invalid argument */
    "10024 WSAEMFILE",         /* Too many open files */
    "10035 WSAEWOULDBLOCK",    /* Operation would block */
    "10036 WSAEINPROGRESS",    /* Operation now in progress */
    "10037 WSAEALREADY",       /* Operation already in progress */
    "10038 WSAENOTSOCK",       /* Socket operation on non-socket */
    "10039 WSAEDESTADDRREQ",   /* Destination address required */
    "10040 WSAEMSGSIZE",       /* Message too long */
    "10041 WSAEPROTOTYPE",     /* Protocol wrong type for socket */
    "10042 WSAENOPROTOOPT",    /* Bad protocol option */
    "10043 WSAEPROTONOSUPPORT",/* Protocol not supported */
    "10044 WSAESOCKTNOSUPPORT",/* Socket type not supported */
    "10045 WSAEOPNOTSUPP",     /* Operation not supported on socket */
    "10046 WSAEPFNOSUPPORT",   /* Protocol family not supported */
    "10047 WSAEAFNOSUPPORT",   /* Address family not supported by protocol family */
    "10048 WSAEADDRINUSE",     /* Address already in use */
    "10049 WSAEADDRNOTAVAIL",  /* Can't assign requested address */
    "10050 WSAENETDOWN",       /* Network is down */
    "10051 WSAENETUNREACH",    /* Network is unreachable */
    "10052 WSAENETRESET",      /* Net dropped connection or reset */
    "10053 WSAECONNABORTED",   /* Software caused connection abort */
    "10054 WSAECONNRESET",     /* Connection reset by peer */
    "10055 WSAENOBUFS",        /* No buffer space available */
    "10056 WSAEISCONN",        /* Socket is already connected */
    "10057 WSAENOTCONN",       /* Socket is not connected */
    "10058 WSAESHUTDOWN",      /* Can't send after socket shutdown */
    "10059 WSAETOOMANYREFS",   /*  Too many references, can't splice */
    "10060 WSAETIMEDOUT",      /*  Connection timed out */
    "10061 WSAECONNREFUSED",   /*  Connection refused */
    "10062 WSAELOOP",          /*  Too many levels of symbolic links */
    "10063 WSAENAMETOOLONG",   /*  File name too long */
    "10064 WSAEHOSTDOWN",      /*  Host is down */
    "10065 WSAEHOSTUNREACH",   /*  No Route to Host */
    "10066 WSAENOTEMPTY",      /*  Directory not empty */
    "10067 WSAEPROCLIM",       /*  Too many processes */
    "10068 WSAEUSERS",         /*  Too many users */
    "10069 WSAEDQUOT",         /*  Disc Quota Exceeded */
    "10070 WSAESTALE",         /*  Stale NFS file handle */
    "10091 WSASYSNOTREADY",    /*  Network SubSystem is unavailable */
    "10092 WSAVERNOTSUPPORTED",/*  WINSOCK DLL Version out of range */
    "10093 WSANOTINITIALISED", /*  Successful WSASTARTUP not yet performed */
    "10071 WSAEREMOTE",        /*  Too many levels of remote in path */
    "11001 WSAHOST_NOT_FOUND", /*  Host not found  */
    "11002 WSATRY_AGAIN",      /*  Non-Authoritative Host not found */
    "11003 WSANO_RECOVERY",    /*  Non-Recoverable errors: FORMERR, REFUSED, NOTIMP */
    "11004 WSANO_DATA",        /* Valid name, no data record of requested type */
    "11004 WSANO_ADDRESS"      /* No address, look for MX record */
};


static INT8 socgetlocalip(UINT8 *local_ip);
static unsigned char *err_str(int error);



static void soc_dump_hex(unsigned char *buf, int len)
{
    int i, pos;

    for (pos = 0; pos < len; pos += 16)
    {
        LogWriter::LOGX("%06d  ", pos);
        for (i = 0; i < 16; i++)
            if (pos + i < len)
                LogWriter::LOGX("%02x%s", buf[pos+i], (i+1)%8 ? " " : "  ");
            else
                LogWriter::LOGX("  %s", (i+1)%8 ? " " : "  ");

        LogWriter::LOGX("  ");

        for (i = 0; i < 16 && pos+i < len; i++)
            LogWriter::LOGX("%c%s", isprint(buf[pos+i]) ? buf[pos+i] : '.',
                               (i+1)%8 ? "" : " ");
    }
}

INT8 soc_create_dns_socket(BOOL is_blocking,
                           UINT8 ap_id,
                           INT32 request_id,
                           const char *domain_name,
                           UINT8 *addr,
                           UINT8 addr_len,
                           UINT8 access_id,
                           UINT32 nwk_account_id)
{
    int idx, i;
  
    /* Find the first slot with soc_free[i] equals -1(unused) */
    for (i = last_socket_id + 1; i < MAX_IP_SOCKET_NUM; i++)
    {
        if (soc_free[i] == -1)
            break;
    }
    if (i == MAX_IP_SOCKET_NUM) /* find again from 0 */
    {
        for (i = 0; i < MAX_IP_SOCKET_NUM; i++)
            if (soc_free[i] == -1)
                break;
    }

    if (i < MAX_IP_SOCKET_NUM)
    {
        /* found free slot */
        soc_free[i] = SOC_DNS_SOC_ID;
        soc_appid[i] = ap_id;
        soc_account_id[i] = nwk_account_id;
        soc_state[i] = W32_SOC_S_INUSE;
        soc_state[i] |= W32_SOC_S_DNS_RESOLVING;
        if (soc_find_soc_app_slot((int)ap_id,
            nwk_account_id, 
            &idx) == 0)
        {
            if (soc_find_free_soc_app_slot(&idx))
            {
                soc_app_nwk[idx].app_id = ap_id;
                soc_app_nwk[idx].nwk_account_id = nwk_account_id;
                soc_app_nwk[idx].state = W32_BEARER_CLOSED;
            }
        }
        
        created_socket_num++;
        assert(created_socket_num <= MAX_IP_SOCKET_NUM);
        
        /* fill in DNS info */
        soc_host_name[i].access_id = access_id;
        soc_host_name[i].is_blocking = is_blocking;
        soc_host_name[i].request_id = request_id;
        if (domain_name)
        {
            soc_host_name[i].domain_name = (char*)WinUtil::L_MALLOC(strlen(domain_name) + 1);
            strcpy(soc_host_name[i].domain_name, domain_name);
        }
        
        soc_host_name[i].addr_len = addr_len;
        soc_host_name[i].nwk_account_id = nwk_account_id;
        if (addr)
        {
            memcpy(soc_host_name[i].addr, addr, 4);
        }
        else
        {
            memset(soc_host_name[i].addr, 0, 4);
        }
        
    }
    else
    {
        return SOC_LIMIT_RESOURCE;
    }
    
    return i;
}

void soc_free_dns_socket(INT8 soc_id)
{
    soc_free[soc_id]        = -1;
    soc_appid[soc_id]       = -1;
    soc_state[soc_id]       = W32_SOC_S_NOUSE;
    soc_account_id[soc_id]  = -1;
    soc_events[soc_id]      = 0;
    soc_want_events[soc_id] = 0;
    soc_options[soc_id] = 0;
    assert(created_socket_num > 0);
    created_socket_num--;

    soc_host_name[soc_id].access_id = 0;
    soc_host_name[soc_id].is_blocking = 0;
    soc_host_name[soc_id].request_id = 0;
    if (soc_host_name[soc_id].domain_name)
    {
        free(soc_host_name[soc_id].domain_name);
        soc_host_name[soc_id].domain_name = NULL;
    }
    
    soc_host_name[soc_id].addr_len = 0;
    memset(soc_host_name[soc_id].addr, 0, sizeof(soc_host_name[soc_id].addr));
    soc_host_name[soc_id].nwk_account_id = 0;

    return;
}

static
int soc_find_soc_app_slot(int app_id,
                          int nwk_account_id, 
                          int *idx)
{
    int i;
    assert(idx);
    for (i = 0 ; i < SOC_MAX_APP_NWK_NUM ; i++)
    {
        if (((app_id != 0 && soc_app_nwk[i].app_id == app_id) ||
             (soc_app_nwk[i].app_id == app_id)) &&
             (nwk_account_id == 0xff ||
              soc_app_nwk[i].nwk_account_id == nwk_account_id))
        {
            *idx = i;
            return 1;
        }
    }
    return 0;
}

static
int soc_find_free_soc_app_slot(int *idx)
{
    int found;

    assert(idx);
    found = soc_find_soc_app_slot(-1,
                                  -1, 
                                  idx);
    assert(found == 1);

    return found;
}


static
void soc_free_soc_app_slot(int idx)
{
    assert(idx >= 0 && idx < SOC_MAX_APP_NWK_NUM);

    soc_app_nwk[idx].app_id = -1;
    soc_app_nwk[idx].force_to_release = -1;

    soc_app_nwk[idx].nwk_account_id = -1;
    soc_app_nwk[idx].state = W32_BEARER_CLOSED;
}

static
INT8 soc_activate_account(int s)
{
    int id;

    if (soc_find_soc_app_slot(soc_appid[s],
                              soc_account_id[s], 
                              &id) == 0)
    {
        if (soc_find_free_soc_app_slot(&id))
        {
            soc_app_nwk[id].app_id = soc_appid[s];
            soc_app_nwk[id].nwk_account_id = soc_account_id[s];
            soc_app_nwk[id].state = W32_BEARER_CLOSED;
        }
    }

    if (soc_app_nwk[id].state == W32_BEARER_CLOSED ||
        soc_app_nwk[id].state == W32_BEARER_CLOSING)
    {
        soc_app_nwk[id].state = W32_BEARER_OPENING;
        soc_send_activate_req_to_abm(soc_account_id[s], 0, 0);
    }
    else if (soc_app_nwk[id].state == W32_BEARER_OPENED)
    {
        return SOC_SUCCESS;
    }

    return SOC_WOULDBLOCK;
}

void soc_deactivate_bearer(UINT32 nw_acct_id, 
                           UINT8 qos_profile_id,
                           BOOL is_deact_by_soc)
{
    int i;
    UINT8 ap_id;
    UINT8 ori_acct_id = 0;

    ori_acct_id = cbm_get_original_account(nw_acct_id);
    ap_id = cbm_get_app_id(nw_acct_id);
    
    for (i = 0; i < MAX_IP_SOCKET_NUM; i++)
    {
        if ((ap_id != 0 && soc_appid[i] == ap_id) &&
            soc_free[i] != -1)
        {
            break;
            
        }
    }

    if (i < MAX_IP_SOCKET_NUM)
    {
        if (!is_deact_by_soc)
        {
            for (i = 0; i < MAX_IP_SOCKET_NUM; i++)
            {
                if (ap_id != 0 && soc_app_nwk[i].app_id == ap_id)
                {
                    soc_app_nwk[i].force_to_release = 1;
                }
            }
        }
        return;
    }

    for (i = 0 ; i < SOC_MAX_APP_NWK_NUM ; i++)
    {
        if ((ap_id != 0 && soc_app_nwk[i].app_id == ap_id) &&
             (ori_acct_id == CBM_ALL_NWK_ACCT_ID ||
              soc_app_nwk[i].nwk_account_id == (int)nw_acct_id))
        {
            if (!is_deact_by_soc ||  soc_app_nwk[i].force_to_release > 0)
                soc_send_deactivate_req_to_abm((UINT32)soc_app_nwk[i].nwk_account_id,
                                                0,
                                                0,
                                                ABM_APP_NON_AUTO_DEACT);
            else
            {
                soc_send_deactivate_req_to_abm((UINT32)soc_app_nwk[i].nwk_account_id,
                                                0,
                                                0,
                                                0);
            }
            
            soc_app_nwk[i].state = W32_BEARER_CLOSING;
            
            return;
        }
    }

    return;
}

INT8 soc_app_activate_req_hdlr(UINT32 account_id)
{
    int app_id;
    int idx;
    
    SOC_LOCK;
    app_id = cbm_get_app_id(account_id);
    if (soc_find_soc_app_slot((int)app_id,
            account_id, 
            &idx) == 0)
    {
        if (soc_find_free_soc_app_slot(&idx))
        {
            soc_app_nwk[idx].app_id = app_id;
            soc_app_nwk[idx].nwk_account_id = account_id;
            soc_app_nwk[idx].state = W32_BEARER_CLOSED;
        }
    }

    if (soc_app_nwk[idx].state == W32_BEARER_CLOSED ||
        soc_app_nwk[idx].state == W32_BEARER_CLOSING)
    {
        soc_app_nwk[idx].state = W32_BEARER_OPENING;
        soc_send_activate_req_to_abm(account_id, 0, 0);
    }
   
    SOC_UNLOCK;
    return SOC_WOULDBLOCK;
}

void soc_abm_activate_cnf_hdlr(MSG* msg)
{
    int i, id, ret;
    INT8 cause;
    UINT8 app_id = 0;
    UINT8 domain_name[SOC_MAX_DNS_NAME_LEN];
    UINT32 domain_name_len = SOC_MAX_DNS_NAME_LEN;

    soc_abm_activate_cnf_struct *cnf = NULL;
    cnf = (soc_abm_activate_cnf_struct*)msg->wParam;

    SOC_LOCK;
    
    app_id = cbm_get_app_id(cnf->nw_acc_id);
    if (soc_find_soc_app_slot(app_id,
                              cnf->nw_acc_id, 
                              &id) == 0)
    {
        SOC_UNLOCK;
        return;
    }

    if (cnf->result)
    {
        soc_app_nwk[id].state = W32_BEARER_OPENED;
        
        for (i = 0; i < MAX_IP_SOCKET_NUM; i++)
        {                  
            if ((app_id != 0 && soc_appid[i] == app_id) ||
               (soc_appid[i] == app_id))
            {
                if (soc_state[i] & W32_SOC_S_CONNECTING)
                {
                    soc_state[i] &= ~W32_SOC_S_CONNECTING;
                    sooconnect((UINT8)i, &soc_peer_addr[i]);
                }
                else if (soc_state[i] & W32_SOC_S_SENDING)
                {
                    soc_events[i] |= SOC_WRITE;
                    soc_notify_thread();
                }
                else if (soc_state[i] & W32_SOC_S_RECVING)
                {
                    soc_events[i] |= SOC_READ;
                    soc_notify_thread();
                }
                else if (soc_state[i] & W32_SOC_S_DNS_RESOLVING)
                {
                    if (IP_ALL_ZEROS(soc_host_name[i].addr))
                    {
                    ret = socgethostbyname(soc_host_name[i].is_blocking,
                           soc_host_name[i].request_id,
                           soc_host_name[i].domain_name,
                            (UINT8 *)soc_host_name[i].addr,
                            soc_host_name[i].addr_len,
                           soc_host_name[i].access_id,
                            soc_host_name[i].nwk_account_id,
                            NULL);

                    if (ret == SOC_SUCCESS)
                    {
                        soc_hostname_ind(TRUE,
                            soc_host_name[i].request_id,
                            soc_host_name[i].access_id,
                            soc_host_name[i].addr,
                            soc_host_name[i].addr_len,
                            0,
                            0);
                    }
                    else
                    {
                        
                        soc_hostname_ind(FALSE,
                            soc_host_name[i].request_id,
                            soc_host_name[i].access_id,
                            soc_host_name[i].addr,
                            soc_host_name[i].addr_len,
                            SOC_BEARER_FAIL,
                            0);
                        soc_free_soc_app_slot(id);
                    }
                    }
                    else
                    {
                        ret = socgethostbyaddr(
                            (char*)domain_name,
                            &domain_name_len,
                            soc_host_name[i].addr,
                            soc_host_name[i].addr_len);

                        if (ret == SOC_SUCCESS)
                        {
                            soc_hostaddr_ind(TRUE,
                                soc_host_name[i].request_id,
                                soc_host_name[i].access_id,
                                domain_name,
                                0,
                                0);
                        }
                        else
                        {
                        
                            soc_hostaddr_ind(FALSE,
                                soc_host_name[i].request_id,
                                soc_host_name[i].access_id,
                                domain_name,
                                SOC_BEARER_FAIL,
                                0);
                            soc_free_soc_app_slot(id);
                        }
                    }
                    soc_free_dns_socket((INT8)i);
                }
            }
        }
    }
    else
    {
        
        for (i = 0; i < MAX_IP_SOCKET_NUM; i++)
        {
            if ((app_id != 0 && soc_appid[i] == app_id) ||
                (soc_appid[i] == app_id))
            {

                    if (cnf->err_cause == ABM_E_ABORT &&
                        cnf->cause == ABM_APP_ACCT_NEED_NOTIFY)
                    {
                        cause = SOC_CANCEL_ACT_BEARER;
                    }
                    else
                    {
                        cause = SOC_BEARER_FAIL;
                    }
                    
                    if (soc_state[i] & W32_SOC_S_CONNECTING)
                    {
                        soc_notify_app(soc_free[i], SOC_CONNECT, 
                        FALSE, cause);
                    }
                    else if (soc_state[i] & W32_SOC_S_SENDING) 
                    {
                        soc_notify_app(soc_free[i], SOC_WRITE, 
                        FALSE, cause);
                    }
                    else if (soc_state[i] & W32_SOC_S_RECVING)
                    {
                        soc_notify_app(soc_free[i], SOC_READ, 
                        FALSE, cause);
                    }
                    else if (soc_state[i] & W32_SOC_S_DNS_RESOLVING)
                    {
                        if (IP_ALL_ZEROS(soc_host_name[i].addr))
                        {
                        soc_hostname_ind(FALSE,
                            soc_host_name[i].request_id,
                            soc_host_name[i].access_id,
                            soc_host_name[i].addr,
                            soc_host_name[i].addr_len,
                            SOC_BEARER_FAIL,
                            0);
                        }
                        else
                        {
                            soc_hostaddr_ind(FALSE,
                                soc_host_name[i].request_id,
                                soc_host_name[i].access_id,
                                domain_name,
                                SOC_BEARER_FAIL,
                                0);
                        }
                        soc_free_dns_socket((INT8)i);
                        soc_free_soc_app_slot(id);
                    }
                    else
                    {
                        soc_notify_app(soc_free[i], SOC_CLOSE, 
                        FALSE, cause);
                    }
                    soc_appid[i]       = -1;   
            }
        }   
        soc_free_soc_app_slot(id);
        
    }

    SOC_UNLOCK;
    return;
}

static void soc_notify_app(int socketid,
                    soc_event_enum event,
                    BOOL result,
                    INT8 cause)
{
    app_soc_notify_ind_struct *ind_ptr;
    INT8 s = -1;
    UINT8 i;
#ifdef SOC_DBG_ENABLE
    unsigned char *ev;
#endif

    for (i = 0; i < MAX_IP_SOCKET_NUM; i++)
    {
        if (soc_free[i] == socketid)
        {
            s = i;
            break;
        }
    }

    if (s == -1)
        return;

    ind_ptr = (app_soc_notify_ind_struct*)
		WinUtil::L_MALLOC(sizeof(app_soc_notify_ind_struct));

    ind_ptr->socket_id   = s;
    ind_ptr->event_type  = event;
    ind_ptr->result      = result;
    ind_ptr->error_cause = (soc_error_enum)cause;

    if (event == SOC_CONNECT)
    {
        soc_state[s] &= ~W32_SOC_S_CONNECTING;
        if (result == TRUE)
            soc_state[s] |= W32_SOC_S_CONNECTED;
    }

    if (event == SOC_READ)
    {
        soc_state[s] &= ~W32_SOC_S_RECVING;
    }

    if (event == SOC_WRITE)
    {
        soc_state[s] &= ~W32_SOC_S_SENDING;
    }

#ifdef SOC_DBG_ENABLE
    for (i = 0; i < sizeof(event_str) / sizeof(event_str[0]); i++)
        if (atoi(event_str[i]) == event)
        {
            ev = strchr(event_str[i], ' ') + 1;
            break;
        }
    if (i == sizeof(event_str) / sizeof(event_str[0]))
        ev = event_str[i-1];
    LogWriter::LOGX("notify socket=%d event=%s", s, ev);
#endif /* SOC_DBG_ENABLE */

#ifdef SOC_DBG_ENABLE
    LogWriter::LOGX("notify socket %d ", soc_state[s]);
    switch(event)
    {
    case SOC_READ:
        LogWriter::LOGX("SOC_READ");
        break;

    case SOC_WRITE:
        LogWriter::LOGX("SOC_WRITE");
        break;

    case SOC_CONNECT:
        LogWriter::LOGX("SOC_CONNECT");
        break;

    case SOC_CLOSE:
        LogWriter::LOGX("SOC_CLOSE");
        break;

    case SOC_ACCEPT:
        LogWriter::LOGX("SOC_ACCEPT");
        break;
    }
#endif /* SOC_DBG_ENABLE */

    soc_send_msg_to_app(ind_ptr->socket_id, MSG_ID_APP_SOC_NOTIFY_IND, (local_para_struct*)ind_ptr);
}


/*
 * s:           MoDIS socket layer socket id
 * socket_id:   Winsock layer socket id
 */
static
int soc_reselect(UINT8 s, int socket_id, int wantEventType, int *eventType)
{
    int rt;
    int ret = 0;  //reset
    int i = 0;
    int max_fd = 0;

    FD_ZERO(&read_fd);
    FD_ZERO(&write_fd);
    FD_ZERO(&except_fd);

    if ((wantEventType & SOC_READ) && (*eventType & SOC_READ))
        FD_SET((SOCKET)socket_id, &read_fd);

    if ((wantEventType & SOC_ACCEPT) && (*eventType & SOC_ACCEPT))
        FD_SET((SOCKET)socket_id, &read_fd);

    if ((wantEventType & SOC_CONNECT) && (*eventType & SOC_CONNECT))
        FD_SET((SOCKET)socket_id, &write_fd);

    if ((wantEventType & SOC_WRITE) && (*eventType & SOC_WRITE))
        FD_SET((SOCKET)socket_id, &write_fd);

    if ((wantEventType & SOC_CLOSE) && (*eventType & SOC_CLOSE))
        FD_SET((SOCKET)socket_id, &read_fd);


    for (i = 0; i < sizeof(read_fd.fd_array) / sizeof(read_fd.fd_array[0]); i++)
        max_fd = max_fd < (int)read_fd.fd_array[i] ? read_fd.fd_array[i] : max_fd;
    
    for (i = 0; i < sizeof(write_fd.fd_array) / sizeof(write_fd.fd_array[0]); i++)
        max_fd = max_fd < (int)write_fd.fd_array[i] ? write_fd.fd_array[i] : max_fd;
    
    for (i = 0; i < sizeof(except_fd.fd_array) / sizeof(except_fd.fd_array[0]); i++)
        max_fd = max_fd < (int)except_fd.fd_array[i] ? except_fd.fd_array[i] : max_fd;
        
    max_fd += 1;

    if (max_fd > 1)
        rt = select(max_fd, &read_fd, &write_fd, &except_fd, &soc_timeout_win32);
    else
        return 0;

    if (rt == SOCKET_ERROR) /* error */
    {
    #ifdef SOC_DBG_ENABLE
        int error = WSAGetLastError();
        LogWriter::LOGX("select error: %s (%d)", err_str(error), error);
    #endif /* SOC_DBG_ENABLE */
    }
    else if (0 < rt)    /* success, rt = the number of descriptors */
    {
        if (FD_ISSET(socket_id, &read_fd))
        {
            int sockopt_ret = 0;
            int sotype = 0;
            int sotype_len = sizeof(sotype);
            int data_rcvd = 1;  /* init to 1 as there is data for receive */
            INT8 buf[1];
            
            sockopt_ret = getsockopt(socket_id, SOL_SOCKET, SO_TYPE, (char*)&sotype, &sotype_len);
            if (sotype == SOCK_STREAM)
                data_rcvd = recv(socket_id, (char*)buf, sizeof(buf), MSG_PEEK);
            
            if ((*eventType & SOC_READ) && data_rcvd == 1)
            {
                soc_notify_app(socket_id, SOC_READ, TRUE, 0);
                *eventType &= ~SOC_READ;
                ret++;
            }
            else if (*eventType & SOC_ACCEPT)
            {
                soc_notify_app(socket_id, SOC_ACCEPT, TRUE, 0);
                *eventType &= ~SOC_ACCEPT;
                ret++;
            }
            else if (*eventType & SOC_CLOSE && data_rcvd == 0) /* data_rcvd == 0 */
            {
            #ifdef SOC_DBG_ENABLE
                LogWriter::LOGX("socket %d closed by peer", s);
            #endif /* SOC_DBG_ENABLE */

                if (!(soc_state[s] & W32_SOC_S_FINED))  /* notify once */
                {
                    soc_state[s] |= W32_SOC_S_FINED;
                    soc_notify_app(socket_id, SOC_CLOSE, FALSE, SOC_CONNRESET);
                }
                *eventType &= ~SOC_CLOSE;
                ret++;
            }
        }

        if (FD_ISSET(socket_id, &write_fd))
        {
            if (*eventType & SOC_CONNECT)
            {
                soc_notify_app(socket_id, SOC_CONNECT, TRUE, 0);
                *eventType &= ~SOC_CONNECT;
                ret++;
            }
            else if (*eventType & SOC_WRITE)
            {
                soc_notify_app(socket_id, SOC_WRITE, TRUE, 0);
                *eventType &= ~SOC_WRITE;
                ret++;
            }
        }

        if (FD_ISSET(socket_id, &except_fd))
        {
            if (*eventType & SOC_CONNECT)
            {
                soc_notify_app(socket_id, SOC_CONNECT, FALSE, 0);
                *eventType &= ~SOC_CONNECT;
                ret++;
            }
        }
    }

    return ret;
}


void soc_notify_thread(void)
{
    UINT8 i;

    int batch   = 0;
    int wait_wb = 0;

    if (created_socket_num <= 0 || soc_polling_timer_running == 1)
        return;

    for (i = 0 ; i < MAX_IP_SOCKET_NUM ; i++)
    {
        if (soc_free[i] >= 0 && soc_want_events[i] && soc_events[i] > 0)
        {
            batch += soc_reselect(i,
                                  soc_free[i],
                                  soc_want_events[i],
                                  &soc_events[i]);

            if (batch == 10)
            {
                /* do not send too many IND to application at the same time,
                 * otherwise, application's queue may be full */
                Sleep(100); //100ms butterfly.pan-check ???
                batch = 0;
            }
            wait_wb++;
        }
    }

    /* start a polling timer */
    if (wait_wb > 0)
    {
        soc_start_timer(0 /* dummy timer_id */,
                        0 /* dummy invoke_id */,
                        0 /* dummy sub_id */,
                        100 * 2);  /* 200ms */
        soc_polling_timer_running = 1;
    }
}

/*
 * Called by soc_expiry_hanlder() at the condition of
 * !defined(__MTK_TARGET__) || defined(WIN_SOC)
 */
void soc_polling_timer_expiry(void)
{
    SOC_LOCK;

    assert(soc_polling_timer_running == 1);
    soc_polling_timer_running = 0;
    soc_notify_thread();

    SOC_UNLOCK;
}


void soc_init_win32(void)
{
    int i;
    WSADATA info;

    soc_lock = CreateSemaphore(NULL, 1, 1, NULL);

    for (i = 0 ; i < MAX_IP_SOCKET_NUM ; i++)
    {
        soc_free[i]        = -1;
        soc_appid[i]       = -1;
  
        soc_account_id[i]  = -1;
        soc_events[i]      = 0;
        soc_type[i] = 0;
        soc_want_events[i] = 0;
        soc_options[i] = 0;
        
        soc_host_name[i].access_id = 0;
        soc_host_name[i].is_blocking = 0;
        soc_host_name[i].request_id = 0;
        soc_host_name[i].domain_name = NULL;
    
        soc_host_name[i].addr_len = 0;
        memset(soc_host_name[i].addr, 0, sizeof(soc_host_name[i].addr));
        soc_host_name[i].nwk_account_id = 0;
    }

    for (i = 0 ; i < SOC_MAX_APP_NWK_NUM ; i++)
    {
        soc_app_nwk[i].app_id         = -1;
        soc_app_nwk[i].force_to_release = -1;
    
        soc_app_nwk[i].nwk_account_id = -1;
        soc_app_nwk[i].state          = W32_BEARER_CLOSED;
    }

    if (WSAStartup(MAKEWORD(1,1), &info) != 0)
    {
        MessageBox(NULL, "Cannot initialize WinSock!", "WSAStartup", MB_OK);
    }
    else /* initiate WinSock successfully, start periodic timer */
    {
        soc_timeout_win32.tv_sec = 0;
        soc_timeout_win32.tv_usec = 50; /* wait for 50 usec */

        #ifdef SOC_DBG_ENABLE
        LogWriter::LOGX("socket simulation activated");
        #endif		
    }
}

void soc_hostname_ind(BOOL result,
                 INT32 request_id,
                 UINT8 access_id,
                 UINT8 *ip_addr,
                 UINT8 addr_len,
                 INT8 error_cause,
                 INT32 detail_cause)
{
    app_soc_get_host_by_name_ind_struct *ind_ptr;
  
    ind_ptr = (app_soc_get_host_by_name_ind_struct*)
		WinUtil::L_MALLOC(sizeof(app_soc_get_host_by_name_ind_struct));
    ind_ptr->result = result;

    if (result == TRUE)
    {
        ind_ptr->num_entry = addr_len/4;
        if (ind_ptr->num_entry > SOC_MAX_A_ENTRY)
        {
            ind_ptr->num_entry = SOC_MAX_A_ENTRY;
            addr_len = SOC_MAX_A_ENTRY * 4;
        }
        memcpy(ind_ptr->entry, ip_addr, addr_len);
    }
    else    /* fail */
    {
        ind_ptr->num_entry = 0;
        memset(ind_ptr->entry, 0, sizeof(ind_ptr->entry));
    }
    
    memcpy(ind_ptr->addr, ip_addr, addr_len);
    ind_ptr->addr_len = addr_len;

    ind_ptr->request_id = request_id;
    ind_ptr->access_id = access_id;
    ind_ptr->error_cause = error_cause;
    ind_ptr->detail_cause = detail_cause;

    soc_send_msg_to_app(request_id, MSG_ID_APP_SOC_GET_HOST_BY_NAME_IND,
                        (local_para_struct*)ind_ptr);
 
}

void soc_hostaddr_ind(BOOL result,
                 INT32 request_id,
                 UINT8 access_id,
                 UINT8 *name,
                 INT8 error_cause,
                 INT32 detail_cause)
{
    app_soc_get_host_by_addr_ind_struct *ind_ptr;
  
    ind_ptr = (app_soc_get_host_by_addr_ind_struct*)
		WinUtil::L_MALLOC(sizeof(app_soc_get_host_by_addr_ind_struct));
    ind_ptr->result = result;

    if (result == TRUE)
    {
        strcpy(ind_ptr->name, (char*)name);
        ind_ptr->num_entry = 1;
        strcpy((char *)ind_ptr->entry, (char*)name);
    }
    else    /* fail */
    {
        strcpy(ind_ptr->name, "");
        ind_ptr->num_entry = 0;
        memset(ind_ptr->entry, 0, sizeof(ind_ptr->entry));
    }
    
    ind_ptr->request_id = request_id;
    ind_ptr->access_id = access_id;
    ind_ptr->error_cause = error_cause;
    ind_ptr->detail_cause = detail_cause;

    soc_send_msg_to_app(request_id, MSG_ID_APP_SOC_GET_HOST_BY_ADDR_IND,
                        (local_para_struct*)ind_ptr);
 
}

void soc_convert_addr(struct sockaddr_in *win32_addr,
                      sockaddr_struct *addr)
{
    addr->port = ntohs(win32_addr->sin_port);
    addr->addr_len = 4;

    memset(addr->addr, 0, sizeof(addr->addr));
    memcpy(addr->addr, &win32_addr->sin_addr, 4);
}


INT8 soc_create(UINT8  domain,
                    socket_type_enum  type,
                    UINT8  protocol,
                    UINT32 nwk_account_id)
{

    int ap_id;
    BOOL always_ask = FALSE;
    int idx;
    int i;
    
    /* with initial values */
    INT8 s = SOC_ERROR;
    INT8 error = SOC_SUCCESS;
    SOCKET id = INVALID_SOCKET;

    SOC_LOCK;
    
    ap_id = cbm_get_app_id(nwk_account_id);
    
    switch (type)
    {
    case SOC_SOCK_DGRAM:
        id = socket(AF_INET, SOCK_DGRAM, 0);
        break;

    case SOC_SOCK_STREAM:
        id = socket(AF_INET, SOCK_STREAM, 0);
        break;

    case SOC_SOCK_SMS:
        id = SOC_SMS_ID;
        break;

    default:
        id = socket(AF_INET, SOCK_STREAM, 0);
        break;
    }

    if (id == INVALID_SOCKET)
    {
    #ifdef SOC_DBG_ENABLE
        int err = WSAGetLastError();
		LogWriter::LOGX("socket create error: %s (%d)", err_str(err), err);
    #endif /* SOC_DBG_ENABLE */
        SOC_UNLOCK;
        return -1;
    }

    /* Find the first slot with soc_free[i] equals -1(unused) */
    for (i = last_socket_id + 1; i < MAX_IP_SOCKET_NUM; i++)
    {
        if (soc_free[i] == -1)
            break;
    }
    if (i == MAX_IP_SOCKET_NUM) /* find again from 0 */
    {
        for (i = 0; i < MAX_IP_SOCKET_NUM; i++)
            if (soc_free[i] == -1)
                break;
    }

    if (i < MAX_IP_SOCKET_NUM)
    {
        /* found free slot */
        soc_free[i] = id;
        soc_appid[i] = ap_id;
        soc_account_id[i] = nwk_account_id;
        soc_type[i] = type;
        soc_state[i] = W32_SOC_S_INUSE;
        soc_events[i] |= SOC_READ;
        soc_events[i] |= SOC_CLOSE;

        if (soc_find_soc_app_slot((int)ap_id,
            nwk_account_id, 
            &idx) == 0)
        {
            if (soc_find_free_soc_app_slot(&idx))
            {
                soc_app_nwk[idx].app_id = ap_id;
                soc_app_nwk[idx].nwk_account_id = nwk_account_id;
                soc_app_nwk[idx].state = W32_BEARER_CLOSED;
            }
        }
        
        created_socket_num++;
    }
    else
    {
        /* All MAX_IP_SOCKET_NUM socket slots are occupied */
        i = SOC_LIMIT_RESOURCE;
    }

#ifdef SOC_DBG_ENABLE
	LogWriter::LOGX("return socket id = %d, (win32 socket id = %d)", i, id);
#endif /* SOC_DBG_ENABLE */

    last_socket_id = i;
    SOC_UNLOCK;

    return i;
}


INT8 soc_close(INT8 s)
{
    int account_id;

    if (s < 0 || MAX_IP_SOCKET_NUM <= s)
        return SOC_INVALID_SOCKET;

    SOC_LOCK;

    if (soc_free[s] < 0 && soc_free[s] != SOC_SMS_ID)
    {
        SOC_UNLOCK;
        return SOC_INVALID_SOCKET;
    }

    if (soc_free[s] != SOC_SMS_ID)
        closesocket(soc_free[s]);
      
    soc_free[s] = -1;
    soc_state[s] = W32_SOC_S_NOUSE;
    soc_type[s] = 0;

    assert(created_socket_num > 0);

    created_socket_num--;
    account_id = soc_account_id[s];

    soc_deactivate_bearer(account_id, 0, TRUE);
    
    SOC_UNLOCK;
    return SOC_SUCCESS;
}


INT8 soc_bind(INT8 s, sockaddr_struct *addr)
{
    struct sockaddr_in A;
    int socketid;
    char buf[128];
    int last_error;

    if (s < 0 || s >= MAX_IP_SOCKET_NUM)
        return SOC_INVALID_SOCKET;

    SOC_LOCK;

    socketid = soc_free[s];

    if (socketid == SOC_SMS_ID)
    {
        SOC_UNLOCK;
        return 0;
    }

    if (socketid < 0)
    {
        SOC_UNLOCK;
        return SOC_INVALID_SOCKET;
    }

    if (addr->addr_len == 0)
    {
        memset(addr->addr, 0, MAX_SOCK_ADDR_LEN);
    }

    A.sin_family = AF_INET;
    A.sin_port = htons(addr->port);

    sprintf(buf,"%d.%d.%d.%d",
            addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3]);
    A.sin_addr.s_addr = inet_addr(buf);

#ifdef SOC_DBG_ENABLE
    LogWriter::LOGX("Socket %d bind %s:%d", s, buf, addr->port);
#endif /* SOC_DBG_ENABLE */

    if (bind(socketid, (const struct sockaddr*)&A, sizeof(A)) == SOCKET_ERROR)
    {
        last_error = WSAGetLastError();
        SOC_UNLOCK;
        if (last_error == WSAEADDRINUSE)
            return SOC_ADDRINUSE;
        else
            return -1;
    }

    SOC_UNLOCK;

    return SOC_SUCCESS;
}


INT8 soc_listen(INT8 s, UINT8 backlog)
{
    int socketid;

    if (s < 0 || s >= MAX_IP_SOCKET_NUM)
        return SOC_INVALID_SOCKET;

    SOC_LOCK;

    socketid = soc_free[s];

    if (socketid < 0)
    {
        SOC_UNLOCK;
       
        if(socketid == SOC_SMS_ID)
            return SOC_OPNOTSUPP;
        else
            return SOC_INVALID_SOCKET;
    }
    
    if (listen(socketid, backlog))
    {
        SOC_UNLOCK;
        return -1;
    }

    soc_events[s] |= SOC_ACCEPT;

    if ((soc_free[s] != SOC_SMS_ID) && 
        ((soc_options[s] & SOC_SILENT_LISTEN) != SOC_SILENT_LISTEN))
    {
        soc_activate_account(s);
        
    }
  
    SOC_UNLOCK;
    return 0;
}


INT8 soc_accept(INT8 s, sockaddr_struct *addr)
{
    int socketid, i;
    SOCKET newid;

    struct sockaddr_in their_addr; /* connector's address information */
    int addr_len;

    if (s < 0 || s >= MAX_IP_SOCKET_NUM)
        return SOC_INVALID_SOCKET;

    SOC_LOCK;

    socketid = soc_free[s];

    if (socketid < 0)
    {
        SOC_UNLOCK;
        return SOC_INVALID_SOCKET;
    }

    addr_len = sizeof(struct sockaddr);

    newid = accept(socketid, (struct sockaddr *)&their_addr, &addr_len);
    if (newid == INVALID_SOCKET)
    {
        int last_error = WSAGetLastError();

        soc_events[s] |= SOC_ACCEPT;

        if (last_error != WSAEWOULDBLOCK)
        {
        #ifdef SOC_DBG_ENABLE
            LogWriter::LOGX("accept(%d) error: %s (%d)",
                       s, err_str(last_error), last_error);
        #endif /* SOC_DBG_ENABLE */
            SOC_UNLOCK;
            return -1;
        }

        soc_notify_thread();

        SOC_UNLOCK;
        return SOC_WOULDBLOCK;
    }

    soc_events[s] |= SOC_ACCEPT;

    soc_convert_addr(&their_addr, addr);


    for (i = last_socket_id + 1; i < MAX_IP_SOCKET_NUM; i++)
    {
        if (soc_free[i] == -1)
            break;
    }
    if (i == MAX_IP_SOCKET_NUM) /* find again from 0 */
    {
        for (i = 0; i < MAX_IP_SOCKET_NUM; i++)
            if (soc_free[i] == -1)
                break;
    }

    if (i < MAX_IP_SOCKET_NUM)
    {
        soc_free[i] = newid;
        
        soc_state[i] = W32_SOC_S_INUSE;
        soc_events[i] |= SOC_READ;
        soc_events[i] |= SOC_CLOSE;

        soc_appid[i] = soc_appid[s];
        soc_account_id[i] = soc_account_id[s];
    
        created_socket_num++;
    }
    else
    {
        closesocket(newid);
        SOC_UNLOCK;
        return SOC_LIMIT_RESOURCE;
    }

    last_socket_id = i;
    SOC_UNLOCK;
    return (INT8)i;
}


INT8 sooconnect(INT8 s, sockaddr_struct *addr)
{
    char buf[128];
    struct sockaddr_in A;
    int socketid;
    INT8 ret;

    if (s < 0 || s >= MAX_IP_SOCKET_NUM)
        return SOC_INVALID_SOCKET;

    if (soc_state[s] & (W32_SOC_S_CONNECTING | W32_SOC_S_CONNECTED))
        return SOC_ALREADY;

    socketid = soc_free[s];

    if (socketid == SOC_SMS_ID)
    {
        return SOC_OPNOTSUPP;
    }

    if (socketid < 0)
    {
        return SOC_INVALID_SOCKET;
    }

    A.sin_family = AF_INET;
    A.sin_port = htons(addr->port);

    sprintf(buf,"%d.%d.%d.%d",
            addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3]);
    A.sin_addr.s_addr = inet_addr(buf);

#ifdef SOC_DBG_ENABLE
    LogWriter::LOGX("Socket %d connect to %s:%d", s, buf, addr->port);
#endif /* SOC_DBG_ENABLE */   

    
    soc_state[s] |= W32_SOC_S_CONNECTING;
    memcpy(&soc_peer_addr[s], addr, sizeof(sockaddr_struct));

    if ((soc_free[s] != SOC_SMS_ID) && 
        ((soc_options[s] & SOC_SILENT_LISTEN) != SOC_SILENT_LISTEN) &&
        soc_type[s] != SOC_SOCK_DGRAM)
    {
        ret = soc_activate_account(s);
        if (ret == SOC_WOULDBLOCK)
        {
            return SOC_WOULDBLOCK;
        }
    }
    if (connect(socketid, (const struct sockaddr*)&A, sizeof(A)) == SOCKET_ERROR)
    {
        int last_error = WSAGetLastError();

        if (last_error != WSAEWOULDBLOCK)
        {
        #ifdef SOC_DBG_ENABLE
            LogWriter::LOGX("connect(%d) error: %s (%d)",
                       s, err_str(last_error), last_error);
        #endif /* SOC_DBG_ENABLE */
            soc_state[s] &= ~W32_SOC_S_CONNECTING;
            return -1;
        }

        soc_events[s] |= SOC_CONNECT;

        soc_notify_thread();
        
        return SOC_WOULDBLOCK;
    }

    soc_state[s] &= ~W32_SOC_S_CONNECTING;
    soc_state[s] |= W32_SOC_S_CONNECTED;

    return 0;
}


INT8 soc_connect(INT8 s, sockaddr_struct *addr)
{
    INT8 ret;
    
    SOC_LOCK;

    ret = sooconnect(s, addr);
        
    SOC_UNLOCK;

    return ret;
}


INT8 soc_shutdown(INT8 s, UINT8 how)
{
    int sd;
	INT8 ret;

    if (s < 0 || MAX_IP_SOCKET_NUM <= s)
        return SOC_INVALID_SOCKET;

    SOC_LOCK;

    if (soc_free[s] == SOC_SMS_ID)
    {
        SOC_UNLOCK;
        return 0;
    }

    if (soc_free[s] < 0)
    {
        SOC_UNLOCK;
        return SOC_INVALID_SOCKET;
    }

	switch (how)
	{
    case SHUT_RD:
		sd = SD_RECEIVE;
                soc_state[s] |= W32_SOC_S_CANTRCVMORE;
		break;

	case SHUT_WR:
		sd = SD_SEND;
		break;

	case SHUT_RDWR:
		sd = SD_BOTH;
		break;

	default:
		return SOC_INVAL;
	}

    if (shutdown(soc_free[s], sd)) /* error */
        ret = SOC_ERROR;
    else
        ret = SOC_SUCCESS;
    SOC_UNLOCK;

    return ret;
}


INT32 soc_sendto(INT8 s,
                     void *buf,
                     INT32 len,
                     UINT8 flags,
                     sockaddr_struct *addr)
{
    int send_size;
    int socketid;
    struct sockaddr_in A;
    char buf_tmp[128];
    INT8 ret;

    if (s < 0 || s >= MAX_IP_SOCKET_NUM)
        return SOC_INVALID_SOCKET;

    SOC_LOCK;

    socketid = soc_free[s];

    if (socketid == SOC_SMS_ID)
    {
        SOC_UNLOCK;
        return SOC_OPNOTSUPP;
    }

    if (socketid < 0)
    {
        SOC_UNLOCK;
        return SOC_INVALID_SOCKET;
    }
    
    soc_state[s] |= W32_SOC_S_SENDING;
    
     if ((soc_free[s] != SOC_SMS_ID) && 
        ((soc_options[s] & SOC_SILENT_LISTEN) != SOC_SILENT_LISTEN))
    {
        ret = soc_activate_account(s);
        if (ret == SOC_WOULDBLOCK)
        {
            SOC_UNLOCK;
            return SOC_WOULDBLOCK;
        }
    }
        
    A.sin_family = AF_INET;
    A.sin_port = htons(addr->port);

    sprintf(buf_tmp, "%d.%d.%d.%d",
            addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3]);
    A.sin_addr.s_addr = inet_addr(buf_tmp);

    send_size = sendto(socketid, (char*)buf, len, 0,
                       (const struct sockaddr*)&A, sizeof(A));
    if (send_size < 0)
    {
        int last_error = WSAGetLastError();

        if (last_error == WSAEWOULDBLOCK)
        {
            soc_events[s] |= SOC_WRITE;
            soc_notify_thread();
        #ifdef SOC_DBG_ENABLE
            LogWriter::LOGX("sendto(%d) wouldblock, event: %x", s, soc_events[s]);
        #endif /* SOC_DBG_ENABLE */
            SOC_UNLOCK;
            return SOC_WOULDBLOCK;        
        }

    #ifdef SOC_DBG_ENABLE
        LogWriter::LOGX("send error %d", last_error);
    #endif /* SOC_DBG_ENABLE */

        soc_state[s] &= ~W32_SOC_S_SENDING;
        SOC_UNLOCK;
        return -1;
    }

#ifdef SOC_DBG_ENABLE
    LogWriter::LOGX("Send to: %d.%d.%d.%d port=%d %d bytes", 
               addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3],
               addr->port, send_size);
    soc_dump_hex(buf, send_size);
#endif /* SOC_DBG_ENABLE */

    soc_state[s] &= ~W32_SOC_S_SENDING;
    SOC_UNLOCK;

    return send_size;
}

INT32 soc_send(INT8 s,
                   void *buf,
                   INT32 len,
                   UINT8 flags)
{
    int send_size;
    int socketid;
    INT8 ret;

    if (s < 0 || s >= MAX_IP_SOCKET_NUM)
        return SOC_INVALID_SOCKET;

    SOC_LOCK;

    socketid = soc_free[s];

    if (socketid == SOC_SMS_ID)
    {
        SOC_UNLOCK;
        return SOC_OPNOTSUPP;
    }

    if (socketid < 0)
    {
        SOC_UNLOCK;
        return SOC_INVALID_SOCKET;
    }

    soc_state[s] |= W32_SOC_S_SENDING;

     if ((soc_free[s] != SOC_SMS_ID) && 
        ((soc_options[s] & SOC_SILENT_LISTEN) != SOC_SILENT_LISTEN))
    {
        ret = soc_activate_account(s);
        if (ret == SOC_WOULDBLOCK)
        {
            SOC_UNLOCK;
            return SOC_WOULDBLOCK;
        }
    }

    send_size = send(socketid, (char*)buf, len, 0);
    if (send_size <0)
    {
        int last_error = WSAGetLastError();

        if (last_error == WSAEWOULDBLOCK)
        {
            soc_events[s] |= SOC_WRITE;
            soc_notify_thread();
        #ifdef SOC_DBG_ENABLE
            LogWriter::LOGX("send(%d) wouldblock, event: %x", s, soc_events[s]);
        #endif /* SOC_DBG_ENABLE */
            SOC_UNLOCK;
            return SOC_WOULDBLOCK;        
        }

    #ifdef SOC_DBG_ENABLE
        LogWriter::LOGX("send error %d", last_error);
    #endif /* SOC_DBG_ENABLE */

        soc_state[s] &= ~W32_SOC_S_SENDING;
        SOC_UNLOCK;
        return -1;
    }

#ifdef SOC_DBG_ENABLE
    LogWriter::LOGX("Send: %d bytes", send_size);
    soc_dump_hex(buf, send_size);
#endif /* SOC_DBG_ENABLE */

    soc_state[s] &= ~W32_SOC_S_SENDING;
    SOC_UNLOCK;

    return send_size;
}


INT32 soc_recvfrom(INT8 s,
                       void *buf,
                       INT32 len,
                       UINT8 flags,
                       sockaddr_struct *fromaddr)
{
    int recv_size;
    int socketid;
    struct sockaddr_in their_addr; /* connector's address information */
    int addr_len;
    int error;
    int opt_val;
    int opt_len = sizeof(int);
    int flag = 0;
    INT8 ret;
    UINT8 peek_buf[1];

    if (s < 0 || s >= MAX_IP_SOCKET_NUM)
        return SOC_INVALID_SOCKET;

    SOC_LOCK;

    socketid = soc_free[s];

    if (socketid == SOC_SMS_ID)
    {
        SOC_UNLOCK;
        return SOC_WOULDBLOCK;
    }

    if (socketid < 0)
    {
        SOC_UNLOCK;
        return SOC_INVALID_SOCKET;
    }

    if (soc_state[s] & W32_SOC_S_CANTRCVMORE)
    {
        /* receive side of the connection is closed */
        SOC_UNLOCK;
        return 0;
    }

     if (getsockopt(socketid, 
           SOL_SOCKET, 
           SO_TYPE, 
           (char*)&opt_val, 
           &opt_len) == SOCKET_ERROR)        
    {
        SOC_UNLOCK;
        return SOC_ERROR;
           
    }

    
    soc_state[s] |= W32_SOC_S_RECVING;

     if ((soc_free[s] != SOC_SMS_ID) && 
        ((soc_options[s] & SOC_SILENT_LISTEN) != SOC_SILENT_LISTEN))
    {
        ret = soc_activate_account(s);
        if (ret == SOC_WOULDBLOCK)
        {
            SOC_UNLOCK;
            return SOC_WOULDBLOCK;
        }
    }
    
    addr_len = sizeof(struct sockaddr);

    flag |= flags & SOC_MSG_PEEK ? MSG_PEEK : 0;
    recv_size = recvfrom(socketid, (char*)buf, len, flag,
                         (struct sockaddr *)&their_addr, &addr_len);
    if (recv_size < 0)
    {
        error = WSAGetLastError();
        switch (error)
        {
        case WSAEWOULDBLOCK:
            soc_events[s] |= SOC_READ;
            soc_notify_thread();
        #ifdef SOC_DBG_ENABLE
            LogWriter::LOGX("recvfrom(%d) wouldblock, evnent %x", s, soc_events[s]);
        #endif /* SOC_DBG_ENABLE */
            SOC_UNLOCK;
            return SOC_WOULDBLOCK;

        case WSAEMSGSIZE:
            soc_state[s] &= ~W32_SOC_S_RECVING;
            SOC_UNLOCK;
            return SOC_MSGSIZE;

        case WSAECONNABORTED:
        #ifdef SOC_DBG_ENABLE
            LogWriter::LOGX("WSAECONNABORTED");
        #endif /* SOC_DBG_ENABLE */
            /* fall through */

        default:
        #ifdef SOC_DBG_ENABLE
            LogWriter::LOGX("recvfrom(%d) returns error %d", s, recv_size);
            LogWriter::LOGX("recvfrom(%d) error %s (%d)", s, err_str(error), error);
        #endif /* SOC_DBG_ENABLE */

            soc_state[s] &= ~W32_SOC_S_RECVING;
            SOC_UNLOCK;
            return SOC_ERROR;
        }
    }
    else if (recv_size == 0)
    {
        /* found if there is available function to obtain the socket type (TCP or UDP)*/
        /* Only TCP need the len == 0 */
        if(opt_val == SOCK_STREAM)
        {
        #ifdef SOC_DBG_ENABLE
            LogWriter::LOGX("socket %d closed by peer", s);
        #endif /* SOC_DBG_ENABLE */
            if (!(soc_state[s] & W32_SOC_S_FINED))  /* notify once */
            {
                soc_state[s] |= W32_SOC_S_FINED;
                soc_notify_app(socketid, SOC_CLOSE, FALSE, SOC_CONNRESET);
            }
        }
       
        soc_state[s] &= ~W32_SOC_S_RECVING;

        /*  Windows recvfrom API didn't return address for TCP. Find another API to get the address*/
        if(fromaddr)
        {
            memset(fromaddr, 0, sizeof(sockaddr_struct));
            if(opt_val == SOCK_STREAM)
            {
                addr_len = sizeof(struct sockaddr);
                error = getpeername(socketid,(struct sockaddr *)&their_addr, &addr_len);
                if(error < 0)
                {
                    SOC_UNLOCK;
                    return SOC_ERROR;
                }

                fromaddr->sock_type = SOC_SOCK_STREAM;      
            }
            else
            {
                fromaddr->sock_type = SOC_SOCK_DGRAM;
            }
        
            soc_convert_addr(&their_addr, fromaddr);
        }

        SOC_UNLOCK;
        return 0;
    }

    /* For UDP sockets, recvfrom() returns 0 indicating that
     * the received datagram contains data of length 0.
     */
#ifdef SOC_DBG_ENABLE
    LogWriter::LOGX("Recv: %d bytes", recv_size);
    soc_dump_hex(buf, recv_size);
#endif /* SOC_DBG_ENABLE */

    /*  Windows recvfrom API didn't return address for TCP. Find another API to get the address*/
   if(fromaddr)
    {
        memset(fromaddr, 0, sizeof(sockaddr_struct));
        if(opt_val == SOCK_STREAM)
        {
            addr_len = sizeof(struct sockaddr);
            error = getpeername(socketid,(struct sockaddr *)&their_addr, &addr_len);
            if(error < 0)
            {
                SOC_UNLOCK;
                return SOC_ERROR;
            }

            fromaddr->sock_type = SOC_SOCK_STREAM;      
        }
        else
        {
            fromaddr->sock_type = SOC_SOCK_DGRAM;
        }
        
        soc_convert_addr(&their_addr, fromaddr);
        
    }

    if (recvfrom(socketid, (char*)peek_buf, sizeof(peek_buf), MSG_PEEK, NULL, 0) == 1)
        soc_events[s] &= ~SOC_READ;
	else
    soc_events[s] |= SOC_READ;

    soc_state[s] &= ~W32_SOC_S_RECVING;
    SOC_UNLOCK;

    return recv_size;
}

INT32 soc_recvmsg(INT8 s, soc_msghdr_struct *buf, UINT32 flags)
{
   // not support in modis
    return SOC_ERROR;
}

INT32 soc_recv(INT8 s,
                   void *buf,
                   INT32 len,
                   UINT8 flags)
{
    int recv_size;
    int socketid;
    int error;
    INT8 ret;
    UINT8 peek_buf[1];
    int flag = 0;

    if (s < 0 || s >= MAX_IP_SOCKET_NUM)
        return SOC_INVALID_SOCKET;

    SOC_LOCK;

    socketid = soc_free[s];

    if (socketid == SOC_SMS_ID)
    {
        SOC_UNLOCK;
        return SOC_WOULDBLOCK;
    }

    if (socketid < 0)
    {
        SOC_UNLOCK;
        return SOC_INVALID_SOCKET;
    }

    if (soc_state[s] & W32_SOC_S_CANTRCVMORE)
    {
        /* receive side of the connection is closed */
        SOC_UNLOCK;
        return 0;
    }

    soc_state[s] |= W32_SOC_S_RECVING;

    if ((soc_free[s] != SOC_SMS_ID) && 
        ((soc_options[s] & SOC_SILENT_LISTEN) != SOC_SILENT_LISTEN))
    {
        ret = soc_activate_account(s);
        if (ret == SOC_WOULDBLOCK)
        {
            SOC_UNLOCK;
            return SOC_WOULDBLOCK;
        }
    }
    
    flag |= flags & SOC_MSG_PEEK ? MSG_PEEK : 0;
    recv_size = recv(socketid, (char*)buf, len, flag);
    
    if (recv_size < 0)
    {
        error = WSAGetLastError();
        switch (error)
        {
        case WSAEWOULDBLOCK:
            soc_events[s] |= SOC_READ;
            soc_notify_thread();
        #ifdef SOC_DBG_ENABLE
            LogWriter::LOGX("recv(%d) wouldblock", s);
        #endif /* SOC_DBG_ENABLE */
            SOC_UNLOCK;
            return SOC_WOULDBLOCK;

        case WSAEMSGSIZE:
            soc_state[s] &= ~W32_SOC_S_RECVING;
            SOC_UNLOCK;
            return SOC_MSGSIZE;

        case WSAECONNABORTED:
        #ifdef SOC_DBG_ENABLE
            LogWriter::LOGX("WSAECONNABORTED");
        #endif /* SOC_DBG_ENABLE */
            /* fall through */

        default:
        #ifdef SOC_DBG_ENABLE
            LogWriter::LOGX("recv(%d) returns error %d", s, recv_size);
            LogWriter::LOGX("recv(%d) error %s (%d)", s, err_str(error), error);
        #endif /* SOC_DBG_ENABLE */
            soc_state[s] &= ~W32_SOC_S_RECVING;
            SOC_UNLOCK;
            return SOC_ERROR;
        }
    }
    else if (recv_size == 0)
    {
    #ifdef SOC_DBG_ENABLE
        LogWriter::LOGX("socket %d closed by peer", s);
    #endif /* SOC_DBG_ENABLE */
        if (!(soc_state[s] & W32_SOC_S_FINED))  /* notify once */
        {
            soc_state[s] |= W32_SOC_S_FINED;
            soc_notify_app(socketid, SOC_CLOSE, FALSE, SOC_CONNRESET);
        }

        soc_state[s] &= ~W32_SOC_S_RECVING;
        SOC_UNLOCK;
        return 0;
    }

#ifdef SOC_DBG_ENABLE
    LogWriter::LOGX("Recv: %d bytes", recv_size);
    soc_dump_hex(buf, recv_size);
#endif /* SOC_DBG_ENABLE */

    if (soc_options[s] & SOC_NBIO)
    {
        if (recv(socketid, (char*)peek_buf, sizeof(peek_buf), MSG_PEEK) == 1)
            soc_events[s] &= ~SOC_READ;
        else
            soc_events[s] |= SOC_READ;
    }

    soc_state[s] &= ~W32_SOC_S_RECVING;
    SOC_UNLOCK;
    return recv_size;
}


INT8 soc_select(UINT8 ndesc,
                    soc_fd_set *in,
                    soc_fd_set *out,
                    soc_fd_set *ex,
                    soc_timeval_struct *tv)
{
    int rt;
    int max_ndesc = -1;
    int i;
    int s;
    soc_fd_set *fd[3];
    fd_set win_fd[3];

    struct timeval to_val, *to_val_ptr = NULL;

    SOC_LOCK;

    FD_ZERO(&win_fd[0]);
    FD_ZERO(&win_fd[1]);
    FD_ZERO(&win_fd[2]);
    
    fd[0] = in;
    fd[1] = out;
    fd[2] = ex;

    #define FD_WANT(sock, fdset)    ((fdset)->fds_bits[sock] & 0x01)
    for (s = 0; s < MAX_IP_SOCKET_NUM; s++)
    {
        for (i = 0; i < 3; i++)
        {
            if (fd[i] && FD_WANT(s, fd[i]))   /* SOC_FD_SET */
            {
                FD_SET(soc_free[s], &win_fd[i]);
            }
        }
    }
    #undef FD_WANT

    if (tv)
    {
        to_val.tv_sec = tv->tv_sec;
        to_val.tv_usec = tv->tv_usec;
        to_val_ptr = &to_val;
    }

    /* ndesc is the largest socket id in the SET plus one,
     *               i =   0,  1,  2,  3,  4,  5,  6
     * say, soc_free[] = {34, 27, -1, 14, 21, -1, -1};
     * the app wants to select the socket 1, 3, and 4, it will call
     * FD_SET(1, &readfds);
     * FD_SET(3, &readfds);
     * FD_SET(4, &readfds);
     * select(4+1, &readfds, NULL, NULL, &tv);
     *
     * To this adaptation layer, we have to
     * 1. translate FD_SET(1, &readfds); to
     *              FD_SET(soc_free[1], &readfds);
     *    translate FD_SET(3, &readfds); to
     *              FD_SET(soc_free[3], &readfds);
     *    translate FD_SET(4, &readfds); to
     *              FD_SET(soc_free[4], &readfds);
     * 2. try to get the real ndesc, which is given as (4+1) = 5.
     *    loop over the soc_free[], from 0 to (5-1) to find the max socket id.
     *    e.g., max(37, 27, -1, 24, 21) = 37, the real ndesc is (37+1) = 38.
     */

    for (i = 0; i < ndesc; i++)
        max_ndesc = soc_free[i] > max_ndesc ? soc_free[i] : max_ndesc;
    max_ndesc += 1;
    
    rt = select((int)max_ndesc,
                (in ? &win_fd[0] : NULL),
                (out ? &win_fd[1] : NULL),
                (ex ? &win_fd[2] : NULL), to_val_ptr);

    if (rt < 0)
    {
    #ifdef SOC_DBG_ENABLE
        int error = WSAGetLastError();
        LogWriter::LOGX("select() error %s (%d)", err_str(error), error);
    #endif /* SOC_DBG_ENABLE */
        SOC_UNLOCK;
        return -1;
    }

    #define FD_SET_READY(sock, fdset)   ((fdset)->fds_bits[sock] |= 0x02)
    for (s = 0; s < MAX_IP_SOCKET_NUM; s++)
    {
        for (i = 0; i < 3; i++)
        {
            if (fd[i] && FD_ISSET(soc_free[s], &win_fd[i]))
            {
                FD_SET_READY(s, fd[i]);
            }
        }
    }
    #undef FD_SET_READY

#ifdef SOC_DBG_ENABLE
    LogWriter::LOGX("select return: %d", rt);
#endif /* SOC_DBG_ENABLE */
            
    SOC_UNLOCK;
    return (INT8)rt;
}


INT8 soc_setsockopt(INT8 s,
                        UINT32 option,
                        void *val,
                        UINT8 val_size)
{
    u_long non_blocking;
    int socketid;

    if (s < 0 || s >= MAX_IP_SOCKET_NUM)
        return SOC_INVALID_SOCKET;

    SOC_LOCK;

    socketid = soc_free[s];

    if (socketid == SOC_SMS_ID)
    {
        SOC_UNLOCK;
        return 0;
    }

    if (socketid < 0)
    {
        SOC_UNLOCK;
        return SOC_INVALID_SOCKET;
    }

    if (option == SOC_NBIO)
    {
        if (val == NULL ||
            (val_size != sizeof(BOOL) && val_size != sizeof(UINT8)))
        {
            SOC_UNLOCK;
            return SOC_INVAL;
        }
       
        if (val_size  == sizeof(UINT8))
        non_blocking = *(UINT8*)val ? 1 : 0;
        else /* val_size  == sizeof(BOOL) */
            non_blocking = *(BOOL*)val ? 1 : 0;
        ioctlsocket(socketid, FIONBIO, &non_blocking);
        if (non_blocking)
            soc_options[s] |= SOC_NBIO;
        else
            soc_options[s] &= ~SOC_NBIO;
    }
    else if (option == SOC_SILENT_LISTEN)
    {
        if (val == NULL || val_size != sizeof(UINT8) || 
            ((*(UINT8 *)val != TRUE) && (*(UINT8 *)val != FALSE)))
        {
            SOC_UNLOCK;
            return SOC_INVAL;
        }
        
        soc_options[s] |= SOC_SILENT_LISTEN;
    }
    else if (option == SOC_ASYNC)
    {
        if (val == NULL || val_size != sizeof(UINT8))
        {
           SOC_UNLOCK;
           return SOC_INVAL;
        }
        soc_want_events[s] = (int)(*(UINT8 *)val);
        if (soc_events[s] & soc_want_events[s])
            soc_notify_thread();
    }
    else
    {
        /* do nothing */
    }

    SOC_UNLOCK;

    return 0;
}


INT8 soc_getsockopt(INT8 s,
                        UINT32 option,
                        void *val,
                        UINT8 val_size)
{
    if (s < 0 || s >= MAX_IP_SOCKET_NUM)
        return SOC_INVALID_SOCKET;

    SOC_LOCK;

    if (soc_free[s] == SOC_SMS_ID)
    {
        SOC_UNLOCK;
        return 0;
    }

    if (soc_free[s] < 0)
    {
        SOC_UNLOCK;
        return SOC_INVALID_SOCKET;
    }

    if (option == SOC_NBIO)
    {
        if (val == NULL || val_size != sizeof(BOOL))
        {
            SOC_UNLOCK;
            return SOC_INVAL;
        }

        if (soc_options[s] & SOC_NBIO)
            *(BOOL*)val = TRUE;
        else
            *(BOOL*)val = FALSE;
    }
    else if (option == SOC_ASYNC)
    {
        *(UINT8*)val = (UINT8)soc_want_events[s];
    }
    else
    {
        /* do nothing */
    }

    SOC_UNLOCK;

    return 0;
}


INT8 socgethostbyname(BOOL is_blocking,
                          INT32 request_id,
                          const char *domain_name,
                          UINT8 *addr,
                          UINT8 addr_len,
                          UINT8 access_id,
                          UINT32 nwk_account_id,
                          UINT8 *out_entry_num)
{
    unsigned char *p;
    struct hostent *HOST;
    UINT8 i;
    UINT8 out_entry = 0;

    HOST = gethostbyname(domain_name);

    if (HOST == NULL)
        return SOC_ERROR;
    else
    {
        if (HOST->h_aliases[0])
            LogWriter::LOGX("[w32_socket] %s is an alias for %s.", HOST->h_aliases[0], HOST->h_name);
        for (i = 0, p = (unsigned char *)HOST->h_addr_list[0]; HOST->h_addr_list[i++]; p+=HOST->h_length)
            LogWriter::LOGX("[w32_socket] %s has address %d.%d.%d.%d", HOST->h_name, p[0], p[1], p[2], p[3]); //%u.%u.%u.%u

        memset(addr, 0, addr_len);
        for (i = 0; i < (addr_len/4); i++)
        {
            if (HOST->h_addr_list[i])
            {
                memcpy((addr+(i*4)), HOST->h_addr_list[i], 4);
                out_entry++;
            }
            else
            {
                break;
            }
        }

        if (out_entry_num)
        {
            *out_entry_num = out_entry;
        }
        return SOC_SUCCESS;
    }
}

INT8 soc_gethostbyname(BOOL is_blocking,
                           INT32 request_id,
                           const char *domain_name,
                           UINT8 *addr,
                           UINT8 *addr_len,
                           UINT8 access_id,
                           UINT32 nwk_account_id)
{
    INT8 ret, soc_id;
    UINT8 ap_id;
    BOOL always_ask = FALSE;

    SOC_LOCK;

    ap_id = cbm_get_app_id(nwk_account_id);
    *addr_len = 4;

    soc_id = soc_create_dns_socket(is_blocking,
                                ap_id,
                                request_id,
                                domain_name,
                                NULL,
                                *addr_len,
                                access_id,
                                nwk_account_id);
    
    if (soc_id == SOC_LIMIT_RESOURCE)
    {
        SOC_UNLOCK;
        return SOC_LIMIT_RESOURCE;
    }
    
    ret = soc_activate_account((int)soc_id);
    
    if (ret == SOC_WOULDBLOCK)
    {
        SOC_UNLOCK;
        return SOC_WOULDBLOCK;
    }
    soc_free_dns_socket(soc_id);
    
    ret = socgethostbyname(is_blocking,
                           request_id,
                           domain_name,
                           addr,
                           *addr_len,
                           access_id,
                           nwk_account_id,
                           NULL);
        
    SOC_UNLOCK;
    return ret;

}


INT8 soc_gethostbyname2(BOOL is_blocking,
                           INT32 request_id,
                           const char *domain_name,
                           UINT8 *addr,
                           UINT8 *addr_len,
                           UINT8 access_id,
                           UINT32 nwk_account_id,
                           UINT8 in_entry_num,
                           UINT8 *out_entry_num)
{
    INT8 ret, soc_id;
    UINT8 ap_id;
    BOOL always_ask = FALSE;

    SOC_LOCK;

    ap_id = cbm_get_app_id(nwk_account_id);

    if (addr_len)
    {
        *addr_len = in_entry_num * 4;
    }
    soc_id = soc_create_dns_socket(is_blocking,
                                ap_id,
                                request_id,
                                domain_name,
                                NULL,
                                *addr_len,
                                access_id,
                                nwk_account_id);
    if (soc_id == SOC_LIMIT_RESOURCE)
    {
        SOC_UNLOCK;
        return SOC_LIMIT_RESOURCE;
}

    ret = soc_activate_account((int)soc_id);
    if (ret == SOC_WOULDBLOCK)
    {
        SOC_UNLOCK;
        return SOC_WOULDBLOCK;
    }
    soc_free_dns_socket(soc_id);
    
    ret = socgethostbyname(is_blocking,
                           request_id,
                           domain_name,
                           addr,
                           *addr_len,
                           access_id,
                           nwk_account_id,
                           out_entry_num);
        
    SOC_UNLOCK;
    return ret;
}

INT8 socgethostbyaddr(char *domain_name,
                               UINT32 *domain_name_len,
                               UINT8 *addr,
                               UINT8 addr_len)
{
    UINT32 *ip_addr;
	struct in_addr in_addr;
	struct hostent *remoteHost;

    
    ip_addr = (UINT32 *)addr;
    in_addr.s_addr = (UINT32)(*ip_addr);
    remoteHost = gethostbyaddr((char *) &in_addr, 4, AF_INET);
    if (remoteHost == NULL)
        return SOC_ERROR;
    else
    {
        if (strlen(remoteHost->h_name) > *domain_name_len)
        {
            memcpy(domain_name, remoteHost->h_name, *domain_name_len - 1);
            domain_name[*domain_name_len] = '\0';
            *domain_name_len = strlen(remoteHost->h_name);
            return SOC_NAMETOOLONG;
        }
        else
        {
            strcpy(domain_name, remoteHost->h_name);
        }  
    }
    return SOC_SUCCESS;
}

INT8 soc_gethostbyaddr(BOOL is_blocking,
                           INT32 request_id,
                           char *domain_name,
                           UINT32 *domain_name_len,
                           const UINT8 *addr,
                           UINT8 addr_len,
                           UINT8 access_id,
                           UINT32 nwk_account_id)
{
    UINT8 ap_id;
    INT8 ret, soc_id;

    if (domain_name == NULL || domain_name_len == NULL)
    {
        return SOC_ERROR;
    }
    
	SOC_LOCK;
    
    ap_id = cbm_get_app_id(nwk_account_id);

    soc_id = soc_create_dns_socket(is_blocking,
                                ap_id,
                                request_id,
                                NULL,
                                (UINT8 *)addr,
                                addr_len,
                                access_id,
                                nwk_account_id);
    if (soc_id == SOC_LIMIT_RESOURCE)
    {
        SOC_UNLOCK;
        return SOC_LIMIT_RESOURCE;
    }
    
    ret = soc_activate_account((int)soc_id);
    if (ret == SOC_WOULDBLOCK)
    {
        SOC_UNLOCK;
        return SOC_WOULDBLOCK;
    }
    soc_free_dns_socket(soc_id);
    ret = socgethostbyaddr(domain_name,
                           domain_name_len,
                           (UINT8 *)addr,
                           addr_len);
    
    SOC_UNLOCK;
    return ret;
}


INT8 soc_getsockaddr(INT8 s,
                         BOOL is_local,
                         sockaddr_struct *addr)
{
    struct sockaddr_in saddr;  /* connector's address information */
    int addr_len = sizeof(saddr);
    int socketid;

    if (s < 0 || s >= MAX_IP_SOCKET_NUM)
        return SOC_INVALID_SOCKET;

    SOC_LOCK;

    socketid = soc_free[s];

    if (socketid < 0)
    {
        SOC_UNLOCK;
        return SOC_INVALID_SOCKET;
    }

    if (is_local)
    {
        if (getsockname(socketid, (struct sockaddr *)&saddr, &addr_len))
        {
        #ifdef SOC_DBG_ENABLE
            LogWriter::LOGX("getsockname error %d", WSAGetLastError());
        #endif /* SOC_DBG_ENABLE */
            SOC_UNLOCK;
            return -1;
        }
    }
    else
    {
        if (getpeername(socketid, (struct sockaddr *)&saddr, &addr_len))
        {
        #ifdef SOC_DBG_ENABLE
            LogWriter::LOGX("getpeername error %d", WSAGetLastError());
        #endif /* SOC_DBG_ENABLE */
            SOC_UNLOCK;
            return -1;
        }
    }

    soc_convert_addr(&saddr, addr);
    SOC_UNLOCK;
    return 0;
}

BOOL soc_ip_check(char *asci_addr,
                      UINT8 *ip_addr,
                      BOOL *ip_validity)
{
    UINT8 len;
    UINT8 i,octet_cnt;
    INT32 ip_digit;

    len = strlen(asci_addr);

    for (i = 0 ; i < len ; i++)
    {
        if (!isdigit((int)*(asci_addr+i)) && *(asci_addr+i) != '.' )
            return FALSE;
    }

    *ip_validity = TRUE;

    /* Extract the IP adress from character array */
    for (octet_cnt = 0 ; octet_cnt < 4 ; octet_cnt++)
    {
        if (*asci_addr == '\0') /* in case of "1.2.3." */
        {
            *ip_validity = FALSE;
            return FALSE;
        }

        ip_digit = atoi(asci_addr);

        if (ip_digit < 0 || ip_digit > 255)
        {
            *ip_validity = FALSE;
            return FALSE;
        }

        ip_addr[octet_cnt] = (UINT8)ip_digit;

        if (octet_cnt == 3)
            continue;

        asci_addr = strstr(asci_addr,".");
        if (asci_addr)
            asci_addr++;
        else
            break;
    }

    if (octet_cnt != 4)
    {
        return FALSE;
    }

    if (ip_addr[0] == 0 && ip_addr[1] == 0 && ip_addr[2] == 0 && ip_addr[3] == 0)
    {
        *ip_validity = FALSE;
    }

    return TRUE;
}


static INT8 socgetlocalip(UINT8 *local_ip)
{
    UINT8       ip_addr[4];
    UINT8       addr_len;

    if (local_ip == NULL)
        return SOC_INVAL;

    addr_len = 4;
    if (socgethostbyname(FALSE,
                        0,
                        "",
                        ip_addr,
                        addr_len,
                        0,
                        0,
                        NULL) != SOC_SUCCESS)
    {
        int last_error = WSAGetLastError();

        if (last_error != WSAEWOULDBLOCK)
        {
        #ifdef SOC_DBG_ENABLE
            LogWriter::LOGX("soc_getlocalip error: %d", last_error);
        #endif /* SOC_DBG_ENABLE */
        }
        return SOC_ERROR;
    }

    memcpy(local_ip, ip_addr, 4);

    return SOC_SUCCESS;
}


/*****************************************************************************
* FUNCTION
*  soc_getlocalip
* DESCRIPTION
*   This function returns the local IP address.
*
* PARAMETERS
*  a  IN/OUT      *local_ip, copy the IP address, used as return
* RETURNS
*  SOC_SUCCESS: success
*  negative: bearer is not activated, no IP address info
* GLOBALS AFFECTED
*  none
*****************************************************************************/
INT8 soc_getlocalip(UINT8 *local_ip)
{
    UINT8       ip_addr[4];
    UINT8       addr_len;

    if (local_ip == NULL)
        return SOC_INVAL;

    SOC_LOCK;

    addr_len = 4;
    if (socgethostbyname(FALSE,
                        0,
                        "",
                        ip_addr,
                        addr_len,
                        0,
                        0,
                        NULL) != SOC_SUCCESS)
    {
        int last_error = WSAGetLastError();

        if (last_error != WSAEWOULDBLOCK)
        {
        #ifdef SOC_DBG_ENABLE
            LogWriter::LOGX("soc_getlocalip error: %d", last_error);
        #endif /* SOC_DBG_ENABLE */
        }

        SOC_UNLOCK;

        return SOC_ERROR;
    }

    memcpy(local_ip, ip_addr, 4);

    SOC_UNLOCK;

    return SOC_SUCCESS;
}


/*****************************************************************************
* FUNCTION
*  soc_get_account_localip
* DESCRIPTION
*   This function returns the local IP address per account.
*
* PARAMETERS
*  s  IN          socket id
*  a  IN/OUT      *local_ip, copy the IP address, used as return
* RETURNS
*  SOC_SUCCESS: success
*  negative: bearer is not activated, no IP address info
* GLOBALS AFFECTED
*  none
*****************************************************************************/
INT8 soc_get_account_localip(INT8 s, UINT8 *local_ip)
{
    if (s < 0 || MAX_IP_SOCKET_NUM <= s)
        return SOC_INVALID_SOCKET;

    SOC_LOCK;

    if (soc_free[s] < 0)
    {
        SOC_UNLOCK;
        return SOC_INVALID_SOCKET;
    }

    SOC_UNLOCK;

    return soc_getlocalip(local_ip);
}

/*****************************************************************************
 * FUNCTION
 *  soc_get_localip_by_account
 *
 * DESCRIPTION
 *  This function returns the local IP address by account id.
 * PARAMETERS
 *  account_id      [IN]            profile id
 *  local_ip        [IN/OUT]        copy the IP address, used as return
 * RETURNS
 *  SOC_SUCCESS: success
 *  negative: bearer is not activated, no IP address info
 *****************************************************************************/
INT8 soc_get_localip_by_account(UINT32 account_id, UINT8 *local_ip)
{
    if (local_ip == NULL)
        return SOC_ERROR;

    return soc_getlocalip(local_ip);
}

INT8 soc_set_dns_srv_addr(UINT32 account_id, UINT8 srv_type, UINT8 *dns_addr)
{
    return SOC_SUCCESS;
}

void soc_app_deactivate_req_hdlr(UINT32 nw_acct_id, 
                                 UINT8 qos_profile_id)
{
    SOC_LOCK;

    soc_deactivate_bearer(nw_acct_id, qos_profile_id, FALSE);
    
    SOC_UNLOCK;
    return;
}

void soc_abm_deactivate_cnf_hdlr(MSG* msg)
{
    int i, id;
    UINT8 app_id = 0;

    soc_abm_deactivate_cnf_struct *cnf = NULL;
    cnf = (soc_abm_deactivate_cnf_struct*)msg->wParam;

    SOC_LOCK;
    
    app_id = cbm_get_app_id(cnf->nw_acc_id);
    if (soc_find_soc_app_slot(app_id,
                              cnf->nw_acc_id, 
                              &id) == 0)
    {
        SOC_UNLOCK;
        return;
    }

    if (cnf->result == FALSE && 
        cnf->err_cause == ABM_E_NON_AUTO_DEACT)
    {
        soc_app_nwk[id].state = W32_BEARER_OPENED;
        SOC_UNLOCK;
        return;
    }
    else
    {
        for (i = 0; i < MAX_IP_SOCKET_NUM; i++)
        {                  
            if (((app_id != 0 && soc_appid[i] == app_id) ||
                (soc_appid[i] == app_id)))
            {
                if (soc_free[i] != -1)
                {
                    if (soc_state[i] & W32_SOC_S_CONNECTING)
                    {
                        soc_notify_app(soc_free[i], SOC_CONNECT, 
                        FALSE, SOC_BEARER_FAIL);
                    }
                    else if (soc_state[i] & W32_SOC_S_SENDING) 
                    {
                        soc_notify_app(soc_free[i], SOC_WRITE, 
                        FALSE, SOC_BEARER_FAIL);
                    }
                    else if (soc_state[i] & W32_SOC_S_RECVING)
                    {
                        soc_notify_app(soc_free[i], SOC_READ, 
                        FALSE, SOC_BEARER_FAIL);
                    }
                    else
                    {
                        soc_notify_app(soc_free[i], SOC_CLOSE, 
                        FALSE, SOC_BEARER_FAIL);
                    }
                }
                soc_appid[i]       = -1;
            }
        }
        
        soc_free_soc_app_slot(id);
    }

    SOC_UNLOCK;
    return;
}

INT8 soc_abort_dns_query(BOOL   by_mod_id,
                             BOOL   by_request_id,
                             UINT32 request_id,
                             BOOL   by_access_id,
                             UINT8  access_id,
                             BOOL   by_nwk_account_id,
                             UINT32 nwk_account_id)
{
    return 0;
}


INT8 soc_get_last_error(INT8 s,
                            INT8 *error,
                            INT32 *detail_cause)
{
    if (error)
        *error = 0;

    if (detail_cause)
        *detail_cause = 0;

    return 0;
}


INT8 soc_set_last_error(INT8 s,
                            INT8 error,
                            INT32 detail_cause)
{
    return 0;
}


INT8 soc_gethostbysrv(INT32 request_id,
                          const char *domain_name,
                          UINT8 access_id,
                          UINT32 nwk_account_id,
                          soc_dns_srv_struct *entry,
                          UINT8 in_entry_num,
                          UINT8 *out_entry_num)
{
    return 0;
}


INT8 soc_gethostbynaptr(INT32 request_id,
                          const char *domain_name,
                          UINT8 access_id,
                          UINT32 nwk_account_id,
                          soc_dns_naptr_struct *entry,
                          UINT8 in_entry_num,
                          UINT8 *out_entry_num)
{
    return 0;
}


/*
 * Return TRUE if there is data in the socket's receive queue indicating
 * the socket is ready for read.
 * Otherwise, return FALSE.
 */
BOOL soc_ready_for_read(INT8 s)
{
    int sock = soc_free[s];
    fd_set readfds;
    struct timeval tv;
    int ret;
    INT8 buf[1];
    int sotype = 0;
    int sotype_len = sizeof(sotype);

    if (sock == SOC_SMS_ID)
        return FALSE;
        
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    if (!IS_VALID_SOCKET_NUM(s) || soc_state[s] & W32_SOC_S_FINED)
        return FALSE;

    ret = select(sock + 1, &readfds, NULL, NULL, &tv);
    if (ret > 0)
    {
        ret = getsockopt(sock, SOL_SOCKET, SO_TYPE, (char*)&sotype, &sotype_len);
        if (sotype != SOCK_STREAM ||
            sotype == SOCK_STREAM &&
            recv(sock, (char*)buf, sizeof(buf), MSG_PEEK) == 1) /* received some read */
        {
            return TRUE;
        }
        else
        {
            if (!(soc_state[s] & W32_SOC_S_FINED))  /* notify once */
            {
            #ifdef SOC_DBG_ENABLE
                LogWriter::LOGX("socket is closed by peer, send close notify");
            #endif /* SOC_DBG_ENABLE */
                soc_state[s] |= W32_SOC_S_FINED;
                soc_notify_app(sock, SOC_CLOSE, FALSE, SOC_CONNRESET);
            }
            return FALSE;
        }
    }
    else /* XXX, error return value -1 is not handled */
        return FALSE;
}



void soc_read_notify(INT8 s, BOOL set)
{
    int socketid;

    if (s < 0 || s >= MAX_IP_SOCKET_NUM)
        return;

    SOC_LOCK;

    socketid = soc_free[s];

    if (socketid < 0)
    {
        SOC_UNLOCK;
        return;
    }

    if (set)
        soc_want_events[s] |= SOC_READ;
    else
        soc_want_events[s] &= ~SOC_READ;

    SOC_UNLOCK;
}

unsigned char *err_str(int error)
{
    int i;
    unsigned char *str;
    
    for (i = 0; i < sizeof(winsock_errstr) / sizeof(winsock_errstr[0]); i++)
        if (atoi((char*)winsock_errstr[i]) == error)
        {
            str = (unsigned char*)strchr((char*)winsock_errstr[i], ' ') + 1;
            break;
        }
    return str;
}

void soc_send_msg_to_app(int socket_id, UINT16 msg_id, local_para_struct *local_para)
{
	MSG msg;

	msg.message = msg_id;
	msg.wParam = (WPARAM)local_para;
	msg.lParam = socket_id;

	SendToLuaSocketMessage(&msg);
}

void soc_send_activate_req_to_abm(UINT32 acct_id, UINT8 qos_profile, UINT16 cause)
{
    MSG msg;
    soc_abm_activate_req_struct* soc_abm_activate = NULL;
    soc_abm_activate = (soc_abm_activate_req_struct*)WinUtil::L_MALLOC(sizeof(soc_abm_activate_req_struct));

    soc_abm_activate->nw_acc_id = acct_id;
    soc_abm_activate->qos_profile_id = qos_profile;
    soc_abm_activate->cause = cause;

    msg.message = MSG_ID_SOC_ABM_ACTIVATE_REQ;
    msg.wParam = (WPARAM)soc_abm_activate;
    msg.lParam = -1;

    SendToLuaSocketMessage(&msg);
}

UINT32 cbm_encode_data_account_id(UINT32 acct_id, cbm_sim_id_enum sim_id, 
									  UINT8 app_id, BOOL always_ask)
{
	UINT32 nwk_account_id = 0x05000000; //Default value.

	return nwk_account_id;
}

UINT32 cbm_set_app_id(UINT32 acct_id, UINT8 app_id)
{
    UINT32 account_id = CBM_INVALID_APP_ID;

    if(app_id != 0) account_id = app_id << 24;
    return account_id;
}

BOOL cbm_deregister_app_id(UINT8 app_id)
{
    return TRUE;
}

UINT8 cbm_get_app_id(UINT32 acct_id)
{
	UINT8 app_id;

	app_id = acct_id >> 24;
	if(app_id == 0xff || app_id == 0) app_id = CBM_INVALID_APP_ID;

	return app_id;
}

INT8  cbm_open_bearer(UINT32 account_id)
{
    return CBM_OK;
}

INT8 cbm_register_bearer_info_by_app_id(UINT8 app_id, UINT32 type, UINT32 state)
{
    return CBM_OK;
}

INT8 cbm_register_app_id_with_app_info(cbm_app_info_struct *info,
                                                            UINT8 *app_id)
{
    *app_id = 0x05;
    return CBM_OK;
}

BOOL cbm_decode_data_account_id(UINT32 acct_id, 
	cbm_sim_id_enum *sim_id, 
	UINT8 *app_id, 
	BOOL *always_ask, 
	UINT32* ori_acct_id)
{
	return TRUE;
}

INT8  cbm_decode_data_account_id_ext(UINT32 acct_id, 
	cbm_account_info_struct *acct_info)
{
	return CBM_OK;
}

INT32 abm_get_bearer_available_status(UINT32 bearer, cbm_sim_id_enum sim_id)
{
    return CBM_DEACTIVATED;
}

abm_app_proc_struct *abm_find_app_proc(UINT8 app_id)
{
	abm_app_proc_struct app_proc;

	return &app_proc;
}

void abm_send_bearer_info_ind2app(abm_app_id_struct *app_id,
								  UINT32 account_id,
								  UINT32 real_account_id,
								  UINT8 qos_profile_id,
								  cbm_bearer_state_enum state,
								  cbm_bearer_enum bearer,
								  UINT8 ip_addr[],
								  UINT16 error_cause,
								  cbm_result_error_enum error,
								  nvram_editor_qos_struct *qos)
{
}

void abm_send_bearer_info_ind2mmi(UINT8  connect_type, cbm_bearer_enum bearer_type,
								  UINT32 profile_id, cbm_bearer_state_enum  status)
{
}

void abm_send_act_cnf2soc(cbm_bearer_enum bearer,
						  UINT32 nw_acc_id,
						  UINT32  activate_nw_acc_id,
						  UINT8 qos_profile_id,
						  UINT8 bearer_ctx_id,
						  BOOL qos_fulfilled,
						  UINT8 *ip_addr,
						  UINT8 *pri_dns_addr,
						  UINT8 *sec_dns_addr,
						  UINT8 *gateway,
						  UINT8 *netmask,
						  UINT8 *mac_addr,
						  UINT8 dhcp_state,
						  UINT16 mtu)
{
}

void abm_send_rej2soc(msg_type msg_id,
					  cbm_bearer_enum bearer,
					  UINT16 err_cause,
					  UINT16 cause,
					  UINT32 nw_acc_id,
					  UINT8 qos_profile,
					  UINT8 bearer_ctx,
					  BOOL qos_fulfilled)
{
}

UINT8 cbm_get_original_account(UINT32 acct_id)
{
	return (UINT8)0x05;
}

cbm_sim_id_enum cbm_get_sim_id(UINT32 acct_id)
{
	return CBM_SIM_ID_SIM1;
}

void abm_send_deact_cnf2soc(UINT32 nw_acc_id,
							UINT8 qos_profile,
							UINT8 context_id)
{
}

INT8 abm_get_bearer_type(UINT32 account_id, cbm_bearer_enum *bearer_type)
{
	return CBM_GPRS;
}

INT8 cbm_register_app_id(UINT8 *app_id)
{
	*app_id = 0x05;
	return CBM_OK;
}

void soc_send_deactivate_req_to_abm(UINT32 acct_id,
									UINT8 qos_profile_id,
									UINT8 context_id,
									UINT16 cause)
{
	MSG msg;
	soc_abm_deactivate_req_struct* soc_abm_deactivate = NULL;

	soc_abm_deactivate = (soc_abm_deactivate_req_struct*)WinUtil::L_MALLOC(sizeof(soc_abm_deactivate_req_struct));

	soc_abm_deactivate->nw_acc_id = acct_id;
	soc_abm_deactivate->qos_profile_id = qos_profile_id;
	soc_abm_deactivate->bearer_ctx_id = context_id;
	soc_abm_deactivate->cause =cause;

	msg.message = MSG_ID_SOC_ABM_DEACTIVATE_REQ;
	msg.wParam = (WPARAM)soc_abm_deactivate;
	msg.lParam = -1;

	SendToLuaSocketMessage(&msg);
}

void send_soc_app_activate_req_hdlr(MSG* msg)
{
    MSG message;
    soc_abm_activate_req_struct* soc_abm_activate_req = (soc_abm_activate_req_struct*)msg->wParam;

    soc_abm_activate_cnf_struct *cnf = NULL;
    cnf = (soc_abm_activate_cnf_struct*)WinUtil::L_MALLOC(sizeof(soc_abm_activate_cnf_struct));
    cnf->nw_acc_id = soc_abm_activate_req->nw_acc_id;
    cnf->result = TRUE;

    message.message = MSG_ID_SOC_ABM_ACTIVATE_CNF;
    message.wParam = (WPARAM)cnf;
    message.lParam = -1;

    SendToLuaSocketMessage(&message);
}

void soc_start_timer(UINT8 timer_id,
					 INT8 invoke_id,
					 UINT8 sub_id,
					 UINT32 time_out)
{
	WinTimer::GetWinTimer()->start_soc_timer(time_out);
}

srv_dtcnt_result_enum srv_dtcnt_get_acc_id_by_apn(char *apn, UINT32 *acc_id_out)
{
    return SRV_DTCNT_RESULT_FAILED;
}

