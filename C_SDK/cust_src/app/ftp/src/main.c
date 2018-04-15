/***************************************************************************/
/*									   */
/* qftp.c - command line driven ftp file transfer program		   */
/* Copyright (C) 1996-2001, 2013 Thomas Pfau, tfpfau@gmail.com		   */
/*	1407 Thomas Ave, North Brunswick, NJ, 08902			   */
/*									   */
/* This library is free software.  You can redistribute it and/or	   */
/* modify it under the terms of the Artistic License 2.0.		   */
/* 									   */
/* This library is distributed in the hope that it will be useful,	   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	   */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	   */
/* Artistic License 2.0 for more details.				   */
/* 									   */
/* See the file LICENSE or 						   */
/* http://www.perlfoundation.org/artistic_license_2_0			   */
/*									   */
/***************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iot_network.h"
#include "ftppal.h"
#include "ftplib.h"
#include "iot_fs.h"


/* exit values */
#define EX_SYNTAX 2 	/* command syntax errors */
#define EX_NETDB 3	/* network database errors */
#define EX_CONNECT 4	/* network connect errors */
#define EX_LOGIN 5	/* remote login errors */
#define EX_REMCMD 6	/* remote command errors */
#define EX_SYSERR 7	/* system call errors */

#define FTP_SEND 1	/* send files */
#define FTP_GET 2	/* retreive files */
#define FTP_DIR 3	/* verbose directory */
#define FTP_RM 4	/* delete files */
#define FTP_LIST 5	/* terse directory */

#define DIRBUF_SIZE 1024 /* for wildcard processing */


#define PRIFSZ "u"

#define APP_FTP_SERVER_IP "36.7.87.100"
#define APP_FTP_USR   "user"
#define APP_FTP_PWD   "123456"
#define APP_FTP_GET_FILE  "app.bin"
#define APP_FTP_MSG_NETWORK_READY (0)
#define APP_FTP_MSG_NETWORK_LINKED (1)



static int logged_in = 0;
static char *host = APP_FTP_SERVER_IP;
static char *user = APP_FTP_USR;
static char *pass = APP_FTP_PWD;
static char mode = 'I';
static int action = 0;
static netbuf *conn = NULL;
static int wildcard = 0;

static HANDLE g_s_ftp_task;

void ftp_connect(void)
{
    if (conn)
        return;
    if (!logged_in)
    {
    	if (!FtpConnect(host,&conn))
    	{
    	    fprintf(stderr,"Unable to connect to node %s\n",host);
    	    exit(EX_CONNECT);
    	}
    	if (!FtpLogin(user,pass,conn))
    	{
    	    fprintf(stderr,"Login failure\n%s",FtpLastResponse(conn));
    	    exit(EX_LOGIN);
    	}
	    logged_in++;
    }
}

void change_directory(char *root)
{
    ftp_connect();
    if (!FtpChdir(root, conn))
    {
	fprintf(stderr,"Change directory failed\n%s",FtpLastResponse(conn));
	exit(EX_REMCMD);
    }
}

void site_cmd(char *cmd)
{
    ftp_connect();
    if (!FtpSite( cmd, conn ))
    {
	fprintf(stderr,"SITE command failed\n%s", FtpLastResponse(conn));
	exit(EX_REMCMD);
    }
}

struct REMFILE {
    struct REMFILE *next;
    fsz_t fsz;
    char *fnm;
};

static int log_progress(netbuf *ctl, fsz_t xfered, void *arg)
{
    struct REMFILE *f = (struct REMFILE *) arg;
    if ( f->fsz )
    {
    	int pct = (xfered * 100.0) / f->fsz;
    	iot_debug_print("%s %d%% %" PRIFSZ "\r", f->fnm, pct, xfered);
    }
    else
    {
	    iot_debug_print("%s %" PRIFSZ "\r", f->fnm, xfered);
    }
    return 1;
}

void process_file(char *fnm)
{
    int sts=0;
    fsz_t fsz;
    struct REMFILE *filelist = NULL;
    struct REMFILE rem;

    ftp_connect();
    FtpClearCallback(conn);
    if ((action == FTP_SEND) || (action == FTP_GET) || (action == FTP_RM))
    {
    	if (action == FTP_SEND)
    	{
    	    
    		if (!FtpMkdir(fnm, conn))
    		    fprintf(stderr,"mkdir %s failed\n%s",fnm,FtpLastResponse(conn));
    		else
    		    if (ftplib_debug)
    			fprintf(stderr,"Directory %s created\n",fnm);
    		return;
    	}
        else
        {
    		if (!wildcard)
    	    {
        		struct REMFILE *f;
        		f = (struct REMFILE *) malloc(sizeof(struct REMFILE));
        		memset(f,0,sizeof(struct REMFILE));
        		f->next = filelist;
        		filelist = f;
        		f->fnm = strdup(fnm);
    	    } else {
        		netbuf *dir;
        		char *buf;
        		if (!FtpAccess(fnm, FTPLIB_DIR, FTPLIB_ASCII, conn, &dir))
        		{
        		    fprintf(stderr,"error requesting directory of %s\n%s\n",
        			    fnm, FtpLastResponse(conn));
        		    return;
        		}
        		buf = malloc(DIRBUF_SIZE);
        		while (FtpRead(buf, DIRBUF_SIZE, dir) > 0)
        		{
        		    struct REMFILE *f;
        		    char *p;
        		    f = (struct REMFILE *) malloc(sizeof(struct REMFILE));
        		    memset(f,0,sizeof(struct REMFILE));
        		    f->next = filelist;
        		    p = strchr(buf,'\n');
        		    if (p)
        			*p = '\0';
        		    f->fnm = strdup(buf);
        		    filelist = f;
        		}
        		free(buf);
        		FtpClose(dir);
    	    }
        }
    }
    switch (action)
    {
      case FTP_DIR :
    	sts = FtpDir(NULL, fnm, conn);
    	break;
      case FTP_LIST :
    	sts = FtpNlst(NULL, fnm, conn);
    	break;
      case FTP_SEND :
    	rem.next = NULL;
    	rem.fnm = fnm;
    	rem.fsz = fsz;
    	fsz /= 100;
	    if (fsz > 100000)
	    fsz = 100000;
        if (ftplib_debug && fsz)
        {
    	    FtpCallbackOptions opt;
    	    opt.cbFunc = log_progress;
    	    opt.cbArg = &rem;
    	    opt.idleTime = 1000;
    	    opt.bytesXferred = fsz;
    	    FtpSetCallback(&opt,conn);
        }
    	sts = FtpPut(fnm, fnm, mode,conn);
    	if (ftplib_debug && sts)
    	    printf("%s sent\n",fnm);
	    break;
      case FTP_GET :
    	while (filelist)
    	{
    	    struct REMFILE *f = filelist;
    	    filelist = f->next;
#if defined(__UINT64_MAX)
    	    if (!FtpSizeLong(f->fnm, &fsz, mode, conn))
#else
    	    if (!FtpSize(f->fnm, (unsigned int*)&fsz, mode, conn))
#endif
    		fsz = 0;
    	    f->fsz = fsz;
    	    fsz /= 100;
    	    if (fsz > 100000)
    		fsz = 100000;
    	    if ( fsz == 0 )
    		fsz = 32768;
    	    if (ftplib_debug)
    	    {
        		FtpCallbackOptions opt;
        		opt.cbFunc = log_progress;
        		opt.cbArg = f;
        		opt.idleTime = 1000;
        		opt.bytesXferred = fsz;
        		FtpSetCallback(&opt,conn);
    	    }
    	    sts = FtpGet(f->fnm, APP_FTP_GET_FILE, mode,conn);
    	    if (ftplib_debug && sts)
    		    printf("%s retrieved\n",f->fnm);
    	    free(f->fnm);
    	    free(f);
    	}
	break;
      case FTP_RM :
    	while (filelist)
    	{
    	    struct REMFILE *f = filelist;
    	    filelist = f->next;
    	    sts = FtpDelete(f->fnm,conn);
    	    if (ftplib_debug && sts)
    		printf("%s deleted\n", f->fnm);
    	    free(f->fnm);
    	    free(f);
    	}
	break;
    }
    if (!sts)
	printf("ftp error\n%s\n",FtpLastResponse(conn));
    return;
}

void set_umask(char *m)
{
    char buf[80];
    sprintf(buf,"umask %s", m);
    ftp_connect();
    FtpSite(buf, conn);
}
static void show_file(const char* fnm)
{
    INT32 fd, size;
    char data[64+1];
    WCHAR* wfn = malloc((strlen(fnm)+1)*sizeof(WCHAR));
    strtows(wfn, fnm);
    fd = iot_fs_open_file(wfn, SF_RDONLY);

    if(fd < 0)
    {
        printf("open file %s failed", fnm);
        goto end;    
    }

    size = iot_fs_seek_file(fd, 0, SF_SEEK_END);

    printf("file %s size=%d", fnm, size);

    iot_fs_seek_file(fd, 0, SF_SEEK_SET);

    while(1)
    {
        memset(data, 0, sizeof(data));
        size = iot_fs_read_file(fd, data, 64);
        if(size < 0)
        {
            printf("read file %s failed", fnm);
            break;
        }
        else if(size == 0)
        {
            printf("read file %s end", fnm);
            break;
        }
        printf("file_show::::(%d)%s",size, data);
    }
    iot_fs_close_file(fd);
end:
    free(wfn);
}

void ftp_main()
{
    action = FTP_GET;
    FtpInit();
    {
    	ftp_connect();
    	if ((action == FTP_DIR) || (action == FTP_LIST))
    	    process_file(NULL);
    	else
    	{
    	    char* fnm = "app.bin";
    		process_file(fnm);
            show_file(fnm);
    	}
    }
    if (conn)
	    FtpClose(conn);
}


static void app_networkIndCallBack(E_OPENAT_NETWORK_STATE state)
{
    int* msgptr = iot_os_malloc(sizeof(int));
    printf("[ftp] network ind state %d", state);
    if(state == OPENAT_NETWORK_LINKED)
    {
        *msgptr = APP_FTP_MSG_NETWORK_LINKED;
        iot_os_send_message(g_s_ftp_task, (PVOID)msgptr);
        return;
    }
    else if(state == OPENAT_NETWORK_READY)
    {
        *msgptr = APP_FTP_MSG_NETWORK_READY;
        iot_os_send_message(g_s_ftp_task,(PVOID)msgptr);
        return;
    }
    iot_os_free(msgptr);
}

static void ftp_network_connetck(void)
{
    T_OPENAT_NETWORK_CONNECT networkparam;
    
    memset(&networkparam, 0, sizeof(T_OPENAT_NETWORK_CONNECT));
    memcpy(networkparam.apn, "CMNET", strlen("CMNET"));

    iot_network_connect(&networkparam);

}

static void app_ftp_task(PVOID pParameter)
{
    int*    msg;
    printf("[ftp] wait network ready....");
    BOOL enter = FALSE;

    while(1)
    {
        iot_os_wait_message(g_s_ftp_task, (PVOID)&msg);

        switch(*msg)
        {
            case APP_FTP_MSG_NETWORK_READY:
                printf("network connecting....");
                ftp_network_connetck();
                break;
            case APP_FTP_MSG_NETWORK_LINKED:
                printf("network connected");
                if(!enter)
                {
    				ftp_main();
                    enter = TRUE;
                }
                break;
        }
        iot_os_free(msg);
    }
}

void app_ftp_init(void)
{ 
    printf("[ftp] app_ftp_init");

    //×¢²áÍøÂç×´Ì¬»Øµ÷º¯Êý
    iot_network_set_cb(app_networkIndCallBack);

    g_s_ftp_task = iot_os_create_task(app_ftp_task,
                        NULL,
                        4096,
                        5,
                        OPENAT_OS_CREATE_DEFAULT,
                        "app_ftp");
}

VOID app_main(VOID)
{
    iot_debug_set_fault_mode(OPENAT_FAULT_HANG);
    app_ftp_init();
}

