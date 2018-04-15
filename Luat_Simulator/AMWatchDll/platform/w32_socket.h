/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    w32_socket.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          This file is intends for Socket APIs on WIN32 environment.
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/

#if !defined(__W32_SOCKET__)
#define __W32_SOCKET__

#include "w32_abm.h"

#define MAX_IP_SOCKET_NUM (11)

#define GPRS_MAX_PDP_SUPPORT (1)
#define MTHL_GPRS_MAX_PDP_NUM        GPRS_MAX_PDP_SUPPORT

#define CBM_INVALID_APP_ID  (0)
#define CBM_MRE_ACCT_ID     (0x33)
#define CBM_INVALID_NWK_ACCT_ID (0x3e)


/*
 * peek at incoming message and keep 
 * the received data in receive buffer without
 * removing it from the buffer.
 */
#define SOC_MSG_PEEK        0x2

/*
 * Max DNS NAPTR flag length 
 */
#define SOC_MAX_NAPTR_FLAGS_LEN      (3)

/*
 * Max DNS NAPTR service length 
 */
#define SOC_MAX_NAPTR_SRV_LEN        (70)

/*
 * MAX NAPTR regular expression length 
 */
#define SOC_MAX_NAPTR_REGEXP_LEN     (100)

/*
 * Max DNS address entry for soc_gethostbyname 
 */
#define SOC_MAX_A_ENTRY              (5)

/*
 * Protocol Familty.  Currently, we only support Internet 
 */
#define SOC_PF_INET (0)

/*
 * max socket address length 
 */
#define MAX_SOCK_ADDR_LEN           (28)

/*
 * Max DNS name length 
 */
#define SOC_MAX_DNS_NAME_LEN         (255)

/*
 * Max DNS name entry for soc_gethostbyaddr 
 */
#define SOC_MAX_PTR_ENTRY            (1)

/*
 * Max DNS name length 
 */
#define SOC_MAX_DNS_NAME_LEN         (255)

/*
 * check if the IP address is 0.0.0.0 
 */
#define IP_ALL_ZEROS(a)     (!(a[0] | a[1] | a[2] | a[3]))

/*
 * Max socket number: Max IP socket number +  Max SMS socket number 
 */
#define MAX_SOCKET_NUM      (MAX_IP_SOCKET_NUM)

/*
 * check if this is a valid socket id 
 */
#define IS_VALID_SOCKET_NUM(s)  (0 <= (s) && (s) < MAX_SOCKET_NUM)

typedef enum
{
	CBM_OK                  = 0,  /* success */
	CBM_ERROR               = -1, /* error */
	CBM_WOULDBLOCK          = -2, /* would block */
	CBM_LIMIT_RESOURCE      = -3, /* limited resource */
	CBM_INVALID_ACCT_ID     = -4, /* invalid account id*/
	CBM_INVALID_AP_ID       = -5, /* invalid application id*/
	CBM_INVALID_SIM_ID      = -6, /* invalid SIM id */
	CBM_BEARER_FAIL         = -7, /* bearer fail */
	CBM_DHCP_ERROR          = -8, /* DHCP get IP error */
	CBM_CANCEL_ACT_BEARER   = -9, /* cancel the account query screen, such as always ask or bearer fallback screen */
	CBM_DISC_BY_CM          = -10 /* bearer is deactivated by the connection management */
} cbm_result_error_enum;

/* This structure defines MMI CBM service error cause */
typedef enum
{
    SRV_CBM_OK                  = CBM_OK,  /* success */
    SRV_CBM_ERROR               = CBM_ERROR, /* error */
    SRV_CBM_LIMIT_RESOURCE      = CBM_LIMIT_RESOURCE, /* limited resource */
    SRV_CBM_INVALID_ACCT_ID     = CBM_INVALID_ACCT_ID, /* invalid account id*/
    SRV_CBM_INVALID_AP_ID       = CBM_INVALID_AP_ID, /* invalid application id*/
    SRV_CBM_INVALID_SIM_ID      = CBM_INVALID_SIM_ID, /* invalid SIM id */
    SRV_CBM_BEARER_FAIL         = CBM_BEARER_FAIL, /* bearer fail */
    SRV_CBM_DHCP_ERROR          = CBM_DHCP_ERROR, /* DHCP get IP error */
    SRV_CBM_CANCEL_ACT_BEARER   = CBM_CANCEL_ACT_BEARER, /* cancel the account query screen, such as always ask or bearer fallback screen */
    SRV_CBM_DISC_BY_CM          = CBM_DISC_BY_CM /* bearer is deactivated by the connection management */
} srv_cbm_result_error_enum;

/* This enum is used to define bearer type */
typedef enum
{
    SRV_CBM_BEARER_INVALID = CBM_BEARER_NONE, /* invalid bearer type*/
    SRV_CBM_BEARER_CSD = CBM_CSD, /* csd bearer type */

    SRV_CBM_BEARER_GPRS = CBM_GPRS, /* gprs bearer type */
    SRV_CBM_BEARER_EDGE = CBM_EDGE, /* edge bearer type */
    SRV_CBM_BEARER_UMTS = CBM_UMTS, /* umts (3G) bearer type */

    SRV_CBM_BEARER_PS = CBM_PS, /* packet service type */

    SRV_CBM_BEARER_WIFI = CBM_WIFI, /* wireless lan bearer type */

    SRV_CBM_BEARER_DIALUP = CBM_BEARER_TOTAL+1, /* dialup bearer type, it is the interface used by non-phone application */
    SRV_CBM_BEARER_LOCAL = SRV_CBM_BEARER_DIALUP << 1, /* local bearer type, it is the interface used by phone application */
    SRV_CBM_BEARER_TOTAL
} srv_cbm_bearer_type_enum;

/* Bearer state */
typedef enum
{
	CBM_DEACTIVATED             = 0x01, /* deactivated */
	CBM_ACTIVATING              = 0x02, /* activating */
	CBM_ACTIVATED               = 0x04, /* activated */
	CBM_DEACTIVATING            = 0x08, /* deactivating */
	CBM_CSD_AUTO_DISC_TIMEOUT   = 0x10, /* csd auto disconnection timeout */
	CBM_GPRS_AUTO_DISC_TIMEOUT  = 0x20, /* gprs auto disconnection timeout */
	CBM_NWK_NEG_QOS_MODIFY      = 0x040, /* negotiated network qos modify notification */
	CBM_WIFI_STA_INFO_MODIFY      = 0x080, /* wifi hot spot sta number is changed */
	CBM_BEARER_STATE_TOTAL
} cbm_bearer_state_enum;

/* This enum is used to define bearer status */
typedef enum
{
    SRV_CBM_DEACTIVATED             = CBM_DEACTIVATED, /* bearer deactivated */
    SRV_CBM_ACTIVATING              = CBM_ACTIVATING, /* bearer activating */
    SRV_CBM_ACTIVATED               = CBM_ACTIVATED, /* bearer activated */
    SRV_CBM_DEACTIVATING            = CBM_DEACTIVATING, /* bearer deactivating */
    SRV_CBM_CSD_AUTO_DISC_TIMEOUT   = CBM_CSD_AUTO_DISC_TIMEOUT, /* csd auto disconnection timeout */
    SRV_CBM_GPRS_AUTO_DISC_TIMEOUT  = CBM_GPRS_AUTO_DISC_TIMEOUT, /* gprs auto disconnection timeout */
    SRV_CBM_NWK_NEG_QOS_MODIFY      = CBM_NWK_NEG_QOS_MODIFY, /* negotiated network qos modify notification */
    SRV_CBM_BEARER_STATE_TOTAL
} srv_cbm_bearer_state_enum;

/* Socket Options */
typedef enum
{
	SOC_OOBINLINE     = 0x01 << 0,  /* not support yet */
	SOC_LINGER        = 0x01 << 1,  /* linger on close */
	SOC_NBIO          = 0x01 << 2,  /* Nonblocking */
	SOC_ASYNC         = 0x01 << 3,  /* Asynchronous notification */   

	SOC_NODELAY       = 0x01 << 4,  /* disable Nagle algorithm or not */
	SOC_KEEPALIVE     = 0x01 << 5,  /* enable/disable the keepalive */
	SOC_RCVBUF        = 0x01 << 6,  /* set the socket receive buffer size */
	SOC_SENDBUF       = 0x01 << 7,  /* set the socket send buffer size */

	SOC_NREAD         = 0x01 << 8,  /* no. of bytes for read, only for soc_getsockopt */
	SOC_PKT_SIZE      = 0x01 << 9,  /* datagram max packet size */
	SOC_SILENT_LISTEN = 0x01 << 10, /* SOC_SOCK_SMS property */
	SOC_QOS           = 0x01 << 11, /* set the socket qos */

	SOC_TCP_MAXSEG    = 0x01 << 12, /* set the max segmemnt size */
	SOC_IP_TTL        = 0x01 << 13, /* set the IP TTL value */
	SOC_LISTEN_BEARER = 0x01 << 14, /* enable listen bearer */
	SOC_UDP_ANY_FPORT = 0x01 << 15, /* enable UDP any foreign port */

	SOC_WIFI_NOWAKEUP = 0x01 << 16, /* send packet in power saving mode */
	SOC_UDP_NEED_ICMP = 0x01 << 17, /* deliver NOTIFY(close) for ICMP error */
	SOC_IP_HDRINCL    = 0x01 << 18,  /* IP header included for raw sockets */
	SOC_IPSEC_POLICY      = 0x01 << 19, /* ip security policy */
	SOC_TCP_ACKED_DATA  = 0x01 << 20,  /* TCPIP acked data */
	SOC_TCP_DELAYED_ACK = 0x01 << 21, /* TCP delayed ack */
	SOC_TCP_SACK        = 0x01 << 22, /* TCP selective ack */
	SOC_TCP_TIME_STAMP  = 0x01 << 23,  /* TCP time stamp */
	SOC_TCP_ACK_MSEG  = 0x01 << 24   /* TCP ACK multiple segment */
} soc_option_enum;

typedef enum
{
	SOC_SUCCESS           = 0,     /* success */
	SOC_ERROR             = -1,    /* error */
	SOC_WOULDBLOCK        = -2,    /* not done yet */
	SOC_LIMIT_RESOURCE    = -3,    /* limited resource */
	SOC_INVALID_SOCKET    = -4,    /* invalid socket */
	SOC_INVALID_ACCOUNT   = -5,    /* invalid account id */
	SOC_NAMETOOLONG       = -6,    /* address too long */
	SOC_ALREADY           = -7,    /* operation already in progress */
	SOC_OPNOTSUPP         = -8,    /* operation not support */
	SOC_CONNABORTED       = -9,    /* Software caused connection abort */
	SOC_INVAL             = -10,   /* invalid argument */
	SOC_PIPE              = -11,   /* broken pipe */
	SOC_NOTCONN           = -12,   /* socket is not connected */
	SOC_MSGSIZE           = -13,   /* msg is too long */
	SOC_BEARER_FAIL       = -14,   /* bearer is broken */
	SOC_CONNRESET         = -15,   /* TCP half-write close, i.e., FINED */
	SOC_DHCP_ERROR        = -16,   /* DHCP error */
	SOC_IP_CHANGED        = -17,   /* IP has changed */
	SOC_ADDRINUSE         = -18,   /* address already in use */
	SOC_CANCEL_ACT_BEARER = -19    /* cancel the activation of bearer */
} soc_error_enum;

/* Socket Type */
typedef enum
{
	SOC_SOCK_STREAM = 0,  /* stream socket, TCP */
	SOC_SOCK_DGRAM,       /* datagram socket, UDP */
	SOC_SOCK_SMS,         /* SMS bearer */
	SOC_SOCK_RAW          /* raw socket */
} socket_type_enum;

/* event */
typedef enum
{
	SOC_READ    = 0x01,  /* Notify for read */
	SOC_WRITE   = 0x02,  /* Notify for write */
	SOC_ACCEPT  = 0x04,  /* Notify for accept */
	SOC_CONNECT = 0x08,  /* Notify for connect */
	SOC_CLOSE   = 0x10   /* Notify for close */
} soc_event_enum;

typedef enum
{
	SHUT_RD   = 0x01,         /* shut down the reading side */
	SHUT_WR   = 0x02,         /* shut down the writing side */
	SHUT_RDWR = SHUT_RD | SHUT_WR   /* shut down both sides */
} soc_shutdown_enum;

/* Data Account services result enum */
typedef enum
{
    SRV_DTCNT_RESULT_SUCCESS    = 0,    /* success */
    SRV_DTCNT_RESULT_FAILED,            /* general failed case */
    SRV_DTCNT_RESULT_PROCESSING,        /* processing state implies Event notify */
    SRV_DTCNT_RESULT_STATE_ERROR,       /* service status error */
    SRV_DTCNT_RESULT_INVALID_INPUT,     /* the input parameter is invalid */
    SRV_DTCNT_RESULT_ABORTED,           /* the process is aborted */
    SRV_DTCNT_RESULT_TERMINATED,        /* the process is terminated */
    SRV_DTCNT_RESULT_ONE_STORE_EXISTS,  /* currently only support one store */
    SRV_DTCNT_RESULT_STORE_NOT_COMMIT,  /* store doesn't be config commited before */
    SRV_DTCNT_RESULT_INVALID_STORE,     /* store invalid */
    SRV_DTCNT_RESULT_STORE_FULL,        /* reach maximum profile storage */
    SRV_DTCNT_RESULT_READ_ONLY,         /* can't delete or update read_only profile */
    SRV_DTCNT_RESULT_FIRST_BOOTUP,      /* first bootup rerurn code for store open */
    SRV_DTCNT_RESULT_RESET_FACTORY,     /* the data base sim profiles folder is deleted */
    SRV_DTCNT_RESULT_FS_ERROR,          /* the database file system error */
    SRV_DTCNT_RESULT_IGNORE,
    SRV_DTCNT_RESULT_TOTAL
} srv_dtcnt_result_enum;

/******************************************************
 * Add its APP type for data account registration here
 ******************************************************/
#define DTCNT_APPTYPE_SKIP_CSD          (0x10000000)
#define DTCNT_APPTYPE_NO_PX             (0x20000000)
#define DTCNT_APPTYPE_SPECIFIC_PROFILE  (0x40000000)
#define DTCNT_APPTYPE_SKIP_WIFI         (0x80000000)
#define DTCNT_APPTYPE_NEED_PX           (0x08000000)
#define DTCNT_APPTYPE_NO_SCR            (0x04000000)

typedef enum
{
    DTCNT_APPTYPE_NONE              = 0x0000,
    DTCNT_APPTYPE_DEF               = 0x0001,
    DTCNT_APPTYPE_BRW_WAP           = 0x0002,
    DTCNT_APPTYPE_BRW_HTTP          = 0x0004,
    DTCNT_APPTYPE_MMS               = 0x0008,
    DTCNT_APPTYPE_EMAIL             = 0x0010,
    DTCNT_APPTYPE_IMPS              = 0x0020,
    DTCNT_APPTYPE_DM                = 0x0040,
    DTCNT_APPTYPE_SYNCML            = (0x0080 | DTCNT_APPTYPE_SKIP_CSD),
    DTCNT_APPTYPE_JAVA              = 0x0100,
    DTCNT_APPTYPE_PLAYER            = 0x0200,
    DTCNT_APPTYPE_MRE_WAP           = (0x0400 | DTCNT_APPTYPE_SKIP_CSD),
    DTCNT_APPTYPE_MRE_NET           = (0x0800 | DTCNT_APPTYPE_SKIP_CSD),
    DTCNT_APPTYPE_TETHERING         = 0x1000,
    DTCNT_APPTYPE_WIFI_TETHERING    = 0x2000,
    DTCNT_APPTYPE_USB_TETHERING     = 0x4000,
    DTCNT_APPTYPE_NTP               = (0x8000 | DTCNT_APPTYPE_SKIP_CSD),
    DTCNT_APPTYPE_GPS               = 0x10000,
    DTCNT_APPTYPE_WIDGET            = 0x20000,
    DTCNT_APPTYPE_PUSH              = 0x40000,

    /* APP add its app type here, must below the max number 40 */ 
    DTCNT_APPTYPE_MAX

}dtcnt_apptype_enum;

typedef struct
{
	UINT8	fds_bits[MAX_IP_SOCKET_NUM]; /* select set based on socket id */
} soc_fd_set;

typedef struct 
{
    socket_type_enum	sock_type; /* socket type */
    INT16	addr_len; /* address length */
    UINT16	port; /* port number */
    UINT8	addr[MAX_SOCK_ADDR_LEN];
    /* IP address. For keep the 4-type boundary, 
     * please do not declare other variables above "addr"
    */
} sockaddr_struct;

/* The structure for DNS PTR RR entry */
typedef struct
{
    char ptrname[SOC_MAX_DNS_NAME_LEN];    /* resolved domain name
                                                 * for specific IP address */
} soc_dns_ptr_struct;

typedef struct
{
    UINT8   ref_count;
    UINT16  msg_len;
    
    INT32   request_id;                     /* request id */
    UINT32  account_id;                     /* network account id */
    INT32   detail_cause;  /* refer to ps_cause_enum if error_cause
                                * is SOC_BEARER_FAIL */
    soc_dns_ptr_struct entry[SOC_MAX_PTR_ENTRY]; /* entries */
    
    BOOL	result;                         /* the result of soc_gethostbyaddr */
    UINT8   access_id;                      /* access id */
    char    name[SOC_MAX_DNS_NAME_LEN];     /* DNS name. null terminated */
    UINT8   num_entry;                      /* number of entry */  
    INT8    error_cause;   /* bearer fail */
} app_soc_get_host_by_addr_ind_struct;

/* The structure for DNS A RR entry */
typedef struct
{
	UINT8 address[4];   /* resolved IP address for queried domain name */
} soc_dns_a_struct;

typedef struct
{
    UINT8   ref_count;
    UINT16  msg_len;
    INT32	request_id; /* request id */
    UINT32  account_id; /* network account id */
    INT32   detail_cause;  /* refer to ps_cause_enum if error_cause
                                * is SOC_BEARER_FAIL */
    soc_dns_a_struct entry[SOC_MAX_A_ENTRY]; /* entries */
    
    BOOL    result;     /* the result of soc_gethostbyname */
    UINT8   access_id;  /* access id */
    
    UINT8	addr_len;   /* the first record in entry,  */
    UINT8	addr[16];   /* for backward compatibility */
    UINT8   num_entry;  /* number of entries. field of num_entry and entry are made 
                               to carry multiple answers for one domain name mapping to 
                               multiple IP addresses. */
    INT8    error_cause;   /* bearer fail */

} app_soc_get_host_by_name_ind_struct;

typedef struct
{
   UINT8       ref_count;
   UINT16      msg_len;	
   INT8        socket_id;    /* socket ID */
   soc_event_enum  event_type;   /* soc_event_enum */
   BOOL        result;       /* notification result. KAL_TRUE: success, KAL_FALSE: error */
   soc_error_enum  error_cause;  /* used only when EVENT is close/connect */
   INT32       detail_cause; /* refer to ps_cause_enum if error_cause is
                                  * SOC_BEARER_FAIL */
} app_soc_notify_ind_struct;

typedef struct
{
	void *iov_buf;
	INT32 iov_len;
} soc_iovec_struct;

typedef struct 
{
	sockaddr_struct addr;
	soc_iovec_struct *iov;
	INT32              iov_num;
} soc_msghdr_struct;

/* select timeout structure */
typedef struct 
{
	UINT32 tv_sec;  /* select timeout: no. of second */
	UINT32 tv_usec; /* select timeout: no. of microsecond */
} soc_timeval_struct;

/* The structure for DNS SRV RR entry */
typedef struct
{
    UINT16  priority;   /* the client MUST attempt to use target host 
                             * with the lowest-numbered priority. */
    UINT16  weight;     /* relative weight for entries with the same
                             * priority.
                             * Larger weights SHOULD be given a proportionately
                             * higher probability of being selected. */
    UINT16  port;       /* port number */
    char    target[SOC_MAX_DNS_NAME_LEN];  /* service. null terminated */

} soc_dns_srv_struct;

/* The structure for DNS NAPTR RR entry */
typedef struct
{
	UINT16  order;                              /* order */
	UINT16  pref;                              /* preference */
	char    flag;     /* flags, RFC 3403, some chars */
	char    flags[SOC_MAX_NAPTR_FLAGS_LEN];     /* flags, RFC 3403, some chars */
	char    service[SOC_MAX_NAPTR_SRV_LEN];    /* null terminated */
	char    regexp[SOC_MAX_NAPTR_REGEXP_LEN];  /* regular expression, null terminated */
	char    replacement[SOC_MAX_DNS_NAME_LEN]; /* null terminated */
} soc_dns_naptr_struct;

typedef struct {
    UINT16 app_str_id; /* applicaiton string id used to identify itself */
    UINT16 app_icon_id; /* application icon id used to identify itself */
    UINT64 app_type; /* application type which will be used 
                            in data account profile, such as MMS, etc.*/
} cbm_app_info_struct;

/* 
 * Data account
 */
typedef struct
{
    // must align with qos_struct
    // Please see 3GPP TS24.008 Table 10.5.156 Quality of service information element for detail explaination

    UINT8 qos_length;   /* QOS identifer bit, this value must be 16, or the GPRS bearer will not accept */
    UINT8 unused1;      /* Unused Bit */
    
    UINT8 delay_class;
    /* 
      * Delay Class 
      * 001 Delay Class 1 
      * 010 Delay Class 2
      * 011 Delay Class 3
      * 100 Delay Class 4 (best effort)
      * ...
      */
    
    UINT8 reliability_class;
    /* 
      * Reliability Class 
      * 001 Interpreted as 010
      * 010 UnACK GTP, ACK LLC, ACK RLC, Protected Data
      * 011 UnACK GTP, UnACK LLC, ACK RLC, Protected Data
      * 100 UnACK GTP, UnACK LLC, UnACK RLC, Prottected Data
      * 101 UnACK GTP, UnACK LLC, UnACK RLC, Unprotected Data
      * ...
      */
    
    UINT8 peak_throughput;
    /* 
      * Peak Throughput
      * 0001 Up to 1000 octet/s
      * 0010 Up to 2000 octet/s
      * 0011 Up to 4000 octet/s
      * 0100 Up to 8000 octet/s
      * 0101 Up to 16000 octet/s
      * 0110 Up to 32000 octet/s
      * 0111 Up to 64000 octet/s
      * 1000 Up to 128000 octet/s
      * 1001 Up to 256000 octet/s
      * ...
      */
    
    UINT8 unused2;      /* Unused Bit */
    
    UINT8 precedence_class;
    /* 
      * Precedence Class
      * 001 High Priority
      * 010 Normal Priority
      * 011 Low Priority
      * ...
      */
    
    UINT8 unused3;      /* Unused Bit */
    
    UINT8 mean_throughput;
    /* 
      * Mean Throughput
      * 00001 Up to 100 octet/h
      * 00010 Up to 200 octet/h
      * 00011 Up to 500 octet/h
      * ...
      */
    
    UINT8 traffic_class;
    /* 
      * Traffic Class
      * 001 Conversational class
      * 010 Streaming class
      * 011 Interactive class
      * 100 Background class
      * ...
      */
    
    UINT8 delivery_order;
    /* 
      * Delivery Order
      * 01 With delivery order (yes)
      * 10 Without delivery order (no)
      * ...
      */
    
    UINT8 delivery_of_err_sdu;
    /* 
      * Delivery of erroneous SDUs
      * 001 No detect (-)
      * 010 Erroneous SDUs are delivered (yes)
      * 011 Erroneous SDUs are not delivered (no)
      * ...
      */
    
    UINT8 max_sdu_size;
    /* 
      * Maximum SDU Size
      * ...
      * 10010111 1502 octets
      * 10011000 1510 octets
      * 10011001 1520 octets
      * ...
      */

    UINT8 max_bitrate_up_lnk; // see 24.008 Table 10.5.156
    
    UINT8 max_bitrate_down_lnk; // see 24.008 Table 10.5.156
    
    UINT8 residual_bit_err_rate;
    /* 
      * Residual Bit Error Rate
      * 0001 5*10^-2
      * 0010 1*10^-2
      * 0011 5*10^-3
      * 0100 4*10^-3      
      * 0101 1*10^-3      
      * 0110 1*10^-4      
      * ...
      */
    
    UINT8 sdu_err_ratio;
    /* 
      * SDU Error Ratio
      * 0001 1*10^-2
      * 0010 7*10^-3
      * 0011 1*10^-3
      * 0100 1*10^-4     
      * 0101 1*10^-5      
      * 0110 1*10^-6      
      * ...
      */
    
    UINT8 transfer_delay; // see 24.008 Table 10.5.156
    
    UINT8 traffic_hndl_priority;
    /* 
      * Traffic Handling Priority
      * 01 Priority level 1
      * 10 Priority level 2
      * 11 Priority level 3
      * ...
      */
    
    UINT8 guarntd_bit_rate_up_lnk; // see 24.008 Table 10.5.156
    
    UINT8 guarntd_bit_rate_down_lnk; // see 24.008 Table 10.5.156
    
    UINT8 unused4; /* Unused Bit */
    
    UINT8 signalling_indication;
    /* 
      * Signaling Indication
      * 0 Not optimised for signalling traffic
      * 1 Optimised for signalling traffic
      * ...
      */
    
    UINT8 source_statistics_descriptor;
    /* 
      * Source Statistics Descriptor
      * 0000 Unknown
      * 0001 Speech
      * ...
      */
    
    UINT8 ext_max_bitrate_down_lnk; // see 24.008 Table 10.5.156
    
    UINT8 ext_guarntd_bit_rate_down_lnk; // see 24.008 Table 10.5.156
    
    UINT8 ext_max_bitrate_up_lnk; // see 24.008 Table 10.5.156
    
    UINT8 ext_guarntd_bit_rate_up_lnk; // see 24.008 Table 10.5.156
} nvram_editor_qos_struct;

typedef struct
{
    UINT8               ref_count;
    UINT16              msg_len;
    
    UINT32              account_id; /* network account id contains multiple info */
    UINT32              obtain_account_id; /* The account we really use to activate bearer */
    INT32               error_cause; /* bearer error cause, refer ps_cause_enum */

    cbm_bearer_state_enum   state; /* bearer state. If the bearer state is "CBM_BEARER_NEED_FALLBACK", 
                                      it indicates that the situation of bearer fallback occurs. 
                                      Application needs to stop the timer if it has started one.  
                                      If application doesn't stop the timer based on this state, it has to 
                                      start the timer after the bearer is in activated state. 
                                      If application doesn't start the timer when it receive the 
                                      APP_CBM_BEARER_INFO_IND(ACTIVATING), it has to start the timer 
                                      when the bearer is activated. That is to say, it can start the timer 
                                      when it receives the APP_CBM_BEARER_INFO_IND(ACTIVATED).*/
    cbm_bearer_enum         bearer; /* bearer type */
    
    
    
    cbm_result_error_enum   error;       /* error reason.
                                            1.CBM_BEARER_FAILED indicates that the data link layer is disconnected, 
                                            we are out of coverage. 
                                            2.CBM_DHCP_ERROR indicates that the data link 
                                            layer is connected but get IP failed, VoIP should retry login after 
                                            timeout
                                            3.CBM_CANCEL_ACT_BEARER  indicates that the activation operation is 
                                            canceled by user. Thus, when AP receives this error message, it 
                                            has not to display any error message. This error cause is defined 
                                            for the function of "always-ask".  */
     nvram_editor_qos_struct  nwk_neg_qos;
                                            
    UINT8               qos_profile_id; /* qos profile id */
    UINT8               ip_addr[4];    /* IP address. valid only when state is CBM_ACTIVATED */
} app_cbm_bearer_info_ind_struct;

/* socket linger timeout structure */
typedef struct
{
    BOOL   onoff; /* KAL_TRUE: enable linger timeout FALSE: disable linger timeout */
    UINT16 linger_time; /* linger timeout (unit in seconds) */
} soc_linger_struct;

extern void SendToLuaSocketMessage(const MSG *msg);

void soc_init_win32(void);

INT8 soc_gethostbyaddr(BOOL is_blocking,
                      INT32 request_id,
                      char *domain_name,
                      UINT32 *domain_name_len,
                      const UINT8 *addr,
                      UINT8 addr_len,
                      UINT8 access_id,
                      UINT32 nwk_account_id);

void soc_hostaddr_ind(BOOL result,
                 INT32 request_id,
                 UINT8 access_id,
                 UINT8 *name,
                 INT8 error_cause,
                 INT32 detail_cause);

INT8 soc_gethostbyname(BOOL is_blocking,
					   INT32 request_id,
					   const char *domain_name,
					   UINT8 *addr,
					   UINT8 *addr_len,
					   UINT8 access_id,
					   UINT32 nwk_account_id);

INT8 socgethostbyname(BOOL is_blocking,
                          INT32 request_id,
                          const char *domain_name,
                          UINT8 *addr,
                          UINT8 addr_len,
                          UINT8 access_id,
                          UINT32 nwk_account_id,
                          UINT8 *out_entry_num);

INT32 soc_sendto(INT8 s,
				 void *buf,
				 INT32 len,
				 UINT8 flags,
				 sockaddr_struct *addr);

INT8 soc_create(UINT8  domain,
				socket_type_enum  type,
				UINT8  protocol,
				UINT32 nwk_account_id);

INT8 soc_setsockopt(INT8 s,
                        UINT32 option,
                        void *val,
                        UINT8 val_size);

INT8 soc_getsockopt(INT8 s,
                        UINT32 option,
                        void *val,
                        UINT8 val_size);

INT32 soc_send(INT8 s,
                   void *buf,
                   INT32 len,
                   UINT8 flags);

INT32 soc_recv(INT8 s,
                   void *buf,
                   INT32 len,
                   UINT8 flags);

INT8 soc_bind(INT8 s, sockaddr_struct *addr);

INT8 soc_listen(INT8 s, UINT8 backlog);

INT8 soc_accept(INT8 s, sockaddr_struct *addr);

INT32 soc_recvfrom(INT8 s,
                       void *buf,
                       INT32 len,
                       UINT8 flags,
                       sockaddr_struct *fromaddr);

INT8 soc_close(INT8 s);

INT8 soc_connect(INT8 s, sockaddr_struct *addr);

void soc_send_msg_to_app(int socket_id, UINT16 msg_id, local_para_struct *local_para);

void soc_send_activate_req_to_abm(UINT32 acct_id, UINT8 qos_profile, UINT16 cause);

void soc_abm_activate_cnf_hdlr(MSG* msg);
void soc_abm_deactivate_cnf_hdlr(MSG* msg);

UINT32 cbm_encode_data_account_id(UINT32 acct_id, cbm_sim_id_enum sim_id, 
								  UINT8 app_id, BOOL always_ask);

INT8 cbm_register_bearer_info_by_app_id(UINT8 app_id, UINT32 type, UINT32 state);

INT8  cbm_open_bearer(UINT32 account_id);

UINT32 cbm_set_app_id(UINT32 acct_id, UINT8 app_id);

BOOL cbm_deregister_app_id(UINT8 app_id);

INT8 cbm_register_app_id_with_app_info(cbm_app_info_struct *info,
                                                            UINT8 *app_id);

UINT8 cbm_get_app_id(UINT32 acct_id);

BOOL cbm_decode_data_account_id(UINT32 acct_id, 
								cbm_sim_id_enum *sim_id, 
								UINT8 *app_id, 
								BOOL *always_ask, 
								UINT32* ori_acct_id);

INT8  cbm_decode_data_account_id_ext(UINT32 acct_id, 
									 cbm_account_info_struct *acct_info);

abm_app_proc_struct *abm_find_app_proc(UINT8 app_id);

void abm_send_bearer_info_ind2app(abm_app_id_struct *app_id,
								  UINT32 account_id,
								  UINT32 real_account_id,
								  UINT8 qos_profile_id,
								  cbm_bearer_state_enum state,
								  cbm_bearer_enum bearer,
								  UINT8 ip_addr[],
								  UINT16 error_cause,
								  cbm_result_error_enum error,
								  nvram_editor_qos_struct *qos);
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
						  UINT16 mtu);
void abm_send_rej2soc(msg_type msg_id,
					  cbm_bearer_enum bearer,
					  UINT16 err_cause,
					  UINT16 cause,
					  UINT32 nw_acc_id,
					  UINT8 qos_profile,
					  UINT8 bearer_ctx,
					  BOOL qos_fulfilled);
UINT8 cbm_get_original_account(UINT32 acct_id);
cbm_sim_id_enum cbm_get_sim_id(UINT32 acct_id);
void abm_send_deact_cnf2soc(UINT32 nw_acc_id,
							UINT8 qos_profile,
							UINT8 context_id);
INT8 cbm_register_app_id(UINT8 *app_id);
INT8 abm_get_bearer_type(UINT32 account_id, cbm_bearer_enum *bearer_type);
void abm_send_bearer_info_ind2mmi(UINT8  connect_type, cbm_bearer_enum bearer_type,
								  UINT32 profile_id, cbm_bearer_state_enum  status);
void soc_send_deactivate_req_to_abm(UINT32 acct_id,
									UINT8 qos_profile_id,
									UINT8 context_id,
									UINT16 cause);

void send_soc_app_activate_req_hdlr(MSG* msg);

INT32 abm_get_bearer_available_status(UINT32 bearer, cbm_sim_id_enum sim_id);

srv_dtcnt_result_enum srv_dtcnt_get_acc_id_by_apn(char* apn, UINT32* acc_id_out);

#endif //__W32_SOCKET__
