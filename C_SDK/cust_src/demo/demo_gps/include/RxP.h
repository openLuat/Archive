//---------------------------------------------------------------------------

#ifndef __RxP_H__
#define __RxP_H__


//---------------------------------------------------------------------------
#include "TypeDef.h"

//---------------------------------------------------------------------------
// CONSTANT
//---------------------------------------------------------------------------
#define RXP_LF_DATA 		0x0A
#define RXP_CR_DATA 		0x0D
#define RXP_START_NMEA 		0x24

#define NMEA_ID_QUE_SIZE 	0x0100
#define NMEA_RX_QUE_SIZE 	0x8000
#define SKIP_BYTE_QUE_SIZE 	0x00010000


typedef enum        
{
  RXS_PRM,                // receive preamble
  RXS_DAT,                // receive NMEA data
  RXS_ETX,                // End-of-packet
} RX_SYNC_STATE_T;


typedef union                       /* instrumentation status data      */
    {
    unsigned short  all;            /* all fields                       */
    struct
        {
        unsigned short  inst_ovfl  : 1;
        unsigned short  inst_avail : 1;
        unsigned short  :14;
        } f;
    } rxp_inst_stat_type;
    

//---------------------------------------------------------------------------
// CLASS
//---------------------------------------------------------------------------

bool rxp_init_pcrx(void);
void rxp_exit_pcrx(void);
bool rxp_inst_avail(short *inst_id, short *dat_idx, short *dat_siz);
void rxp_get_inst(short idx, short size, void *data);
void rxp_pcrx_nmea(Byte data);


#endif

