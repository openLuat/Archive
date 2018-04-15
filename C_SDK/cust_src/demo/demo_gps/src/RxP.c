//---------------------------------------------------------------------------
#include "iot_os.h"
#include "iot_debug.h"
#include <string.h>
#include "RxP.h"
#include "TypeDef.h"

//---------------------------------------------------------------------------

#define SetEvent(d) //iot_debug_print("0x%x", d)
struct
{
    short   inst_id;  // 1 - NMEA
    short   dat_idx;
    short   dat_siz;
} id_que[NMEA_ID_QUE_SIZE];

Byte rx_que[NMEA_RX_QUE_SIZE];
Word id_que_head ;
Word id_que_tail ;
Word rx_que_head ;
RX_SYNC_STATE_T rx_state;
unsigned short int u2PktLength;
rxp_inst_stat_type inst_stat;

HANDLE hRxEvent;

unsigned int u4SyncPkt;
unsigned int u4OverflowPkt;
unsigned int u4PktInQueue;
unsigned int u4SkipByte;
unsigned int u4SkipCRLFByte;
int i4Que_Skip_Idx;
unsigned char Que_Skip[SKIP_BYTE_QUE_SIZE];
	

bool rxp_init_pcrx( void )
{
    /*----------------------------------------------------------
    variables
    ----------------------------------------------------------*/
    short   i;

    /*----------------------------------------------------------
    initialize queue indexes
    ----------------------------------------------------------*/
    id_que_head = 0;
    id_que_tail = 0;
    rx_que_head = 0;

    inst_stat.f.inst_avail = false;
    inst_stat.f.inst_ovfl  = false;

    /*----------------------------------------------------------
    initialize identification queue
    ----------------------------------------------------------*/
    for( i=0; i< NMEA_ID_QUE_SIZE; i++)
    {
        id_que[i].inst_id = -1;
        id_que[i].dat_idx =  0;
    }

    /*----------------------------------------------------------
    initialize receive state
    ----------------------------------------------------------*/
    rx_state = RXS_ETX;

    /*----------------------------------------------------------
    initialize statistic information
    ----------------------------------------------------------*/
    u4SyncPkt = 0;
    u4OverflowPkt = 0;
    u4PktInQueue = 0;
    u4SkipByte = 0;
    u4SkipCRLFByte = 0;

    i4Que_Skip_Idx = 0;
    memset(Que_Skip, 0, SKIP_BYTE_QUE_SIZE);

    return TRUE;

}


/*********************************************************************
*
*   PROCEDURE NAME:
*       rxp_exit_pcrx - End Receive
*
*
*   DESCRIPTION:
*       [procedure description]
*
*********************************************************************/
void rxp_exit_pcrx(void)
{
    ;
}


/*********************************************************************
*
*   PROCEDURE NAME:
*       rxp_inst_avail - packet available
*
*   DESCRIPTION:
*       [procedure description]
*
*********************************************************************/
bool rxp_inst_avail(short *inst_id, short *dat_idx,
                                     short *dat_siz)
{
    /*----------------------------------------------------------
    variables
    ----------------------------------------------------------*/
    bool    inst_avail;

    /*----------------------------------------------------------
    if packet is available then return id and index
    ----------------------------------------------------------*/
    if ( id_que_tail != id_que_head )
    {
        *inst_id = id_que[ id_que_tail ].inst_id;
        *dat_idx = id_que[ id_que_tail ].dat_idx;
        *dat_siz = id_que[ id_que_tail ].dat_siz;
        id_que[ id_que_tail ].inst_id = -1;
        id_que_tail = (++id_que_tail) & (unsigned short)(NMEA_ID_QUE_SIZE - 1);
        inst_avail = true;
        if (id_que_tail == id_que_head)
        {
            inst_stat.f.inst_avail = false;
        }
        if (u4PktInQueue > 0)
        {
            u4PktInQueue--;
        }
    }
    else
    {
        inst_avail = false;
    }
    return ( inst_avail );
}   /* rxp_inst_avail() end */


/*********************************************************************
*
*   PROCEDURE NAME:
*       rxp_get_inst - Get packet
*
*   DESCRIPTION:
*       [procedure description]
*
*********************************************************************/
void rxp_get_inst(short idx, short size, void *data)
{
    /*----------------------------------------------------------
    variables
    ----------------------------------------------------------*/
    short i;
    unsigned char *ptr;

    /*----------------------------------------------------------
    copy data from the receive queue to the data buffer
    ----------------------------------------------------------*/
    ptr = (unsigned char *)data;
    for (i = 0; i < size; i++)
    {
        *ptr = rx_que[idx];
        ptr++;
        idx = ++idx & (unsigned short)(NMEA_RX_QUE_SIZE - 1);
    }
}   /* rxp_get_inst() end */


/*********************************************************************
*   PROCEDURE NAME:
*       rxp_pcrx_nmea - Receive NMEA code
*
*   DESCRIPTION:
*     The procedure fetch the characters between(includes) '$' and <CR>.
*     That is, character <LF> is skipped.
*
*********************************************************************/
void rxp_pcrx_nmea( unsigned char data )
{
    /*----------------------------------------------------------
    determine the receive state
    ----------------------------------------------------------*/
    switch (rx_state)
        {
        case RXS_DAT:
            switch (data)
                {
                case RXP_LF_DATA:
                    // count total number of sync packets
                    u4SyncPkt += 1;

                    id_que_head = ++id_que_head & (unsigned short)(NMEA_ID_QUE_SIZE - 1);
                    inst_stat.f.inst_avail = true;
                    if (id_que_tail == id_que_head)
                    {
                      // count total number of overflow packets
                      u4OverflowPkt += 1;

                      id_que_tail = ++id_que_tail & (unsigned short)(NMEA_ID_QUE_SIZE - 1);
                      inst_stat.f.inst_ovfl = true;
                    }
                    else
                    {
                      u4PktInQueue++;
                    }

                    rx_state = RXS_ETX;
                    /*----------------------------------------------------------
                    set RxEvent signaled
                    ----------------------------------------------------------*/
                    SetEvent(hRxEvent);
                    break;

                case RXP_START_NMEA:
                {
                  u4SkipByte += id_que[id_que_head].dat_siz;

                  // Restart NMEA sentence collection
                  rx_state = RXS_DAT;
                  id_que[id_que_head].inst_id = 1;
                  id_que[id_que_head].dat_idx = rx_que_head;
                  id_que[id_que_head].dat_siz = 0;
                  rx_que[rx_que_head] = data;
                  rx_que_head = ++rx_que_head & (unsigned short)(NMEA_RX_QUE_SIZE - 1);
                  id_que[id_que_head].dat_siz++;


                  break;
                }

                default:
                    rx_que[rx_que_head] = data;
                    rx_que_head = ++rx_que_head & (unsigned short)(NMEA_RX_QUE_SIZE - 1);
                    id_que[id_que_head].dat_siz++;
                    break;
                }
            break;

        case RXS_ETX:
            if (data == RXP_START_NMEA)
            {
                rx_state = RXS_DAT;
                id_que[id_que_head].inst_id = 1;
                id_que[id_que_head].dat_idx = rx_que_head;
                id_que[id_que_head].dat_siz = 0;
                rx_que[rx_que_head] = data;
                rx_que_head = ++rx_que_head & (unsigned short)(NMEA_RX_QUE_SIZE - 1);
                id_que[id_que_head].dat_siz++;
            }
            else
            {
              u4SkipByte += 1;
              if ((data == RXP_LF_DATA) ||
                  (data == RXP_CR_DATA))
              {
                u4SkipCRLFByte++;
              }
            }
            break;

        default:
            rx_state = RXS_ETX;

            u4SkipByte += 1;
            break;
        }
}   /* rxp_pcrx_nmea() end */





