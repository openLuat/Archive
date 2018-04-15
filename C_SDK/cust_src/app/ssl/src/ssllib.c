#include "ssllib.h"
#define DBG_INFO(X, Y...)	iot_debug_print("%s %d:"X, __FUNCTION__, __LINE__, ##Y)
#define DBG_ERROR(X, Y...)	iot_debug_print("%s %d:"X, __FUNCTION__, __LINE__, ##Y)

static SocketAPI gSocketRead;
static SocketAPI gSocketWrite;
int OS_SocketRead(int SocketFd, void *Buf, uint16_t Len)
{
	if (gSocketRead)
	{
		return gSocketRead(SocketFd, Buf, Len);
	}
	return -1;
}
int OS_SocketWrite(int SocketFd, void *Buf, uint16_t Len)
{
	if (gSocketWrite)
	{
		return gSocketWrite(SocketFd, Buf, Len);
	}
	return -1;
}

void SSL_RegSocketCallback(SocketAPI SendFun, SocketAPI ReceiveFun)
{
	gSocketWrite = SendFun;
	gSocketRead = ReceiveFun;
}

SSL_CTX * SSL_CreateCtrl(uint16_t NumSessions)
{
	return ssl_ctx_new(SSL_NO_DEFAULT_KEY|SSL_SERVER_VERIFY_LATER, NumSessions);
}

void SSL_FreeCtrl(SSL_CTX *SSLCtrl)
{
	ssl_ctx_free(SSLCtrl);
}

SSL * SSL_NewLink(
		SSL_CTX *SSLCtrl,
		int32_t ClientID,
		const uint8_t *SessionID,
		uint8_t SessionIDSize,
		const char **Hostname,
		uint16_t *MaxFragmentSize)
{
	SSL_EXTENSIONS * Ext = NULL;
	if (Hostname || MaxFragmentSize)
	{
		Ext = ssl_ext_new();
		if (!Ext)
		{
			return NULL;
		}
		if (Hostname)
		{
			Ext->host_name = *Hostname;
			DBG_INFO("%s", Ext->host_name);
		}
		if (MaxFragmentSize)
		{
			Ext->max_fragment_size = *MaxFragmentSize;
		}
	}
	return ssl_client_new(SSLCtrl, ClientID, SessionID, SessionIDSize, Ext);
}

int32_t SSL_ReHandshake(SSL *SSLLink)
{
	return ssl_renegotiate(SSLLink);
}

int32_t SSL_HandshakeStatus(const SSL *SSLLink)
{
	return ssl_handshake_status(SSLLink);
}

void SSL_FreeLink(SSL *SSLLink)
{
	ssl_free(SSLLink);
}

int32_t SSL_Read(SSL *SSLLink, uint8_t **InData)
{
	return ssl_read(SSLLink, InData);
}

int32_t SSL_Write(SSL *SSLLink, const uint8_t *OutData, uint16_t OutLen)
{
	return ssl_write(SSLLink, OutData, OutLen);
}

int32_t SSL_LoadKey(SSL_CTX *SSLCtrl, int32_t Type, const uint8_t *Data, int32_t Len, const int8_t *Password)
{
#ifndef CONFIG_SSL_SKELETON_MODE
    int ret = SSL_OK;
    SSLObjLoader *ssl_obj = NULL;

    ssl_obj = (SSLObjLoader *)calloc(1, sizeof(SSLObjLoader));
    if (!ssl_obj)
    {
    	ret = SSL_NOT_OK;
    	goto error;
    }
    ssl_obj->buf = (uint8_t *)malloc(Len + 1);
    if (!ssl_obj->buf)
    {
    	ret = SSL_NOT_OK;
    	goto error;
    }
    ssl_obj->len = Len;
    memcpy(ssl_obj->buf, Data, Len);
    ssl_obj->buf[Len] = 0;
    /* is the file a PEM file? */
    if (strstr((char *)ssl_obj->buf, "-----BEGIN") != NULL)
    {
#ifdef CONFIG_SSL_HAS_PEM
    	ret = new_pem_obj(SSLCtrl, (Type == SSL_OBJ_X509_CACERT), (char *)ssl_obj->buf, ssl_obj->len, Password);
#else
#ifdef CONFIG_SSL_FULL_MODE
        printf("%s", unsupported_str);
#endif
        ret = SSL_ERROR_NOT_SUPPORTED;
#endif
    }
    else
        ret = do_obj(SSLCtrl, Type, ssl_obj, Password);

error:
    ssl_obj_free(ssl_obj);
    return ret;
#else
#ifdef CONFIG_SSL_FULL_MODE
    printf("%s", unsupported_str);
#endif
    return SSL_ERROR_NOT_SUPPORTED;
#endif /* CONFIG_SSL_SKELETON_MODE */
}

int32_t SSL_VerifyCert(SSL *SSLLink)
{
    int ret;
    int pathLenConstraint = 0;

    ret = x509_verifyEx(SSLLink->ssl_ctx->ca_cert_ctx, SSLLink->x509_ctx,
            &pathLenConstraint);

    if (ret)        /* modify into an SSL error type */
    {
        ret = SSL_X509_ERROR(ret);
    }

    return ret;
}
