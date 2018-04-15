#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "iot_os.h"
#include "iot_debug.h"
#include "iot_pmd.h"
#include "NmeaParser.h"
#include "example.h"
#include "RxP.h"

#define NMEA_DBG iot_debug_print
//---------------------------------------------------------------------------
NMEA_STN_DATA_T rRawData;     //Output Sentence

// 存放NMEA语句元素的结构体
NMEA_GGA_T 		rSGPGGA;
NMEA_GLL_T 		rSGPGLL;
GNSS_GSA_T 		rSGNGSA;
NMEA_GSV_T 		rsgpgsv;
GNSS_GSV_T 		rSGNGSV;
NMEA_RMC_T 		rSGPRMC;
NMEA_VTG_T 		rSGPVTG;


NMEA_STN_T m_eLastDecodedSTN = STN_OTHER;

static char* doubleFormatString(GDOUBLE num)
{
  static char strNum[32];
  int offset = 0;
  GINT32 integer = (GINT32)num;

  memset(strNum, 0, sizeof(strNum));
  offset += sprintf(strNum + offset, "%d", integer);

  offset += sprintf(strNum + offset, ".%u", (GUINT32)((num - integer)*1000000.0));

  return strNum;
}

//---------------------------------------------------------------------------
void ResetNmeaData()
{
    memset(&rSGPGLL, 0, sizeof(NMEA_GLL_T));
    memset(&rSGPGGA, 0, sizeof(NMEA_GGA_T));
    memset(&rSGNGSA, 0, sizeof(GNSS_GSA_T));
    memset(&rsgpgsv, 0, sizeof(NMEA_GSV_T));
    memset(&rSGNGSV, 0, sizeof(GNSS_GSV_T));
    memset(&rSGPRMC, 0, sizeof(NMEA_RMC_T));
    memset(&rSGPVTG, 0, sizeof(NMEA_VTG_T));
}


//---------------------------------------------------------------------------
void DetermineStnType()
{
  if ( (strncmp(&rRawData.Data[0], "$GPGGA", 6) == 0) || 
  	   (strncmp(&rRawData.Data[0], "$GNGGA", 6) == 0) || 
  	   (strncmp(&rRawData.Data[0], "$BDGGA", 6) == 0) || 
  	   (strncmp(&rRawData.Data[0], "$GLGGA", 6) == 0) || 
  	   (strncmp(&rRawData.Data[0], "$GBGGA", 6) == 0) )
  {
    rRawData.eType = STN_GGA;
  }
  else if ( (strncmp(&rRawData.Data[0], "$GPGLL", 6) == 0) || 
  	 		(strncmp(&rRawData.Data[0], "$GNGLL", 6) == 0) || 
  	 		(strncmp(&rRawData.Data[0], "$BDGLL", 6) == 0) || 
  	 		(strncmp(&rRawData.Data[0], "$GLGLL", 6) == 0) || 
  	 		(strncmp(&rRawData.Data[0], "$GBGLL", 6) == 0) )
  {
    rRawData.eType = STN_GLL;
  }
  else if (	(strncmp(&rRawData.Data[0], "$GPGSA", 6) == 0) || 
  			(strncmp(&rRawData.Data[0], "$GNGSA", 6) == 0) || 
  			(strncmp(&rRawData.Data[0], "$BDGSA", 6) == 0) || 
  			(strncmp(&rRawData.Data[0], "$GLGSA", 6) == 0) || 
  			(strncmp(&rRawData.Data[0], "$GAGSA", 6) == 0) || 
  			(strncmp(&rRawData.Data[0], "$GBGSA", 6) == 0) )
  {
    rRawData.eType = STN_GSA;
  }
  else if (	(strncmp(&rRawData.Data[0], "$GPQSA", 6) == 0) || 
  			(strncmp(&rRawData.Data[0], "$QZQSA", 6) == 0) ||
  			(strncmp(&rRawData.Data[0], "$GBQSA", 6) == 0) )
  {
    rRawData.eType = STN_QSA;
  }
  else if ( (strncmp(&rRawData.Data[0], "$GPGSV", 6) == 0) || 
  			(strncmp(&rRawData.Data[0], "$QZGSV", 6) == 0) || 
  			(strncmp(&rRawData.Data[0], "$GLGSV", 6) == 0) || 
  			(strncmp(&rRawData.Data[0], "$GAGSV", 6) == 0) || 
  			(strncmp(&rRawData.Data[0], "$BDGSV", 6) == 0) || 
  			(strncmp(&rRawData.Data[0], "$GBGSV", 6) == 0) )
  {
    rRawData.eType = STN_GSV;
  }
  else if ( (strncmp(&rRawData.Data[0], "$GPRMC", 6) == 0) || 
  			(strncmp(&rRawData.Data[0], "$GNRMC", 6) == 0) || 
  			(strncmp(&rRawData.Data[0], "$BDRMC", 6) == 0) || 
  			(strncmp(&rRawData.Data[0], "$GLRMC", 6) == 0) ||
  			(strncmp(&rRawData.Data[0], "$GBRMC", 6) == 0) )
  {
    rRawData.eType = STN_RMC;
  }
  else if (	(strncmp(&rRawData.Data[0], "$GPVTG", 6) == 0) || 
  			(strncmp(&rRawData.Data[0], "$GNVTG", 6) == 0) || 
  			(strncmp(&rRawData.Data[0], "$BDVTG", 6) == 0) || 
  			(strncmp(&rRawData.Data[0], "$GLVTG", 6) == 0) ||
  			(strncmp(&rRawData.Data[0], "$GBVTG", 6) == 0) )
  {
    rRawData.eType = STN_VTG;
  }
  else
  {
    rRawData.eType = STN_OTHER;
  }
}

//---------------------------------------------------------------------------
void  Invalid_Pre_STN_Data(NMEA_STN_T eThis_STN, NMEA_STN_T eLast_STN)
{
  if ((eThis_STN == STN_GSA) || (eThis_STN == STN_QSA))
  {
    if ((eLast_STN != STN_GSA) && (eLast_STN != STN_QSA))
    {
      memset(&rSGNGSA, 0x00, sizeof(GNSS_GSA_T));
    }
  }
}

//---------------------------------------------------------------------------
void  Invalid_Pre_GSV_Data(void)
{
  if ((rsgpgsv.i2MsgSqn == 1) && (m_eLastDecodedSTN != STN_GSV) && (m_eLastDecodedSTN != STN_OTHER))
  {
      memset(&rSGNGSV, 0x00, sizeof(GNSS_GSV_T));
  }
}


//---------------------------------------------------------------------------
void GNGSA_Assign_From_GPGSA(GNSS_GSA_T *rgngsa, NMEA_GSA_T *rgpgsa)
{
  int i;

#if 1
	rgngsa->u2ActiveGNSS[rgngsa->i2NumGNSS] = rgpgsa->i2NumActive;
	rgngsa->i2NumGNSS += 1;
#endif

  rgngsa->cOpMode = rgpgsa->cOpMode;
  rgngsa->cFixMode = rgpgsa->cFixMode;
  for (i = 0; i < rgpgsa->i2NumActive; i++)
  {
    if (rgngsa->i2NumActive < NMEA_MAX_NUM_SVUSED)
    {
      rgngsa->u2SatID[rgngsa->i2NumActive] = rgpgsa->u2SatID[i];
	  if( rgpgsa->eGnssType == GNSS_TYPE_GPS )	// QZSS & SBAS include in GPGSA
	  {
	  	if( rgpgsa->u2SatID[i]>=33 && rgpgsa->u2SatID[i]<=64 )			// SBAS
	  		rgngsa->eGnssType[rgngsa->i2NumActive] = GNSS_TYPE_SBAS;
		else if( rgpgsa->u2SatID[i]>=193 && rgpgsa->u2SatID[i]<=197 )	// QZSS
	  		rgngsa->eGnssType[rgngsa->i2NumActive] = GNSS_TYPE_QZSS;
		else															// GPS
			rgngsa->eGnssType[rgngsa->i2NumActive] = rgpgsa->eGnssType;
	  }
	  else																// others
	  	rgngsa->eGnssType[rgngsa->i2NumActive] = rgpgsa->eGnssType;
      rgngsa->i2NumActive++;
    }
  }
  rgngsa->fPDOP = rgpgsa->fPDOP;
  rgngsa->fHDOP = rgpgsa->fHDOP;
  rgngsa->fVDOP = rgpgsa->fVDOP;
}

//---------------------------------------------------------------------------
void GNGSA_Assign_From_QSA(GNSS_GSA_T *rgngsa, NMEA_QSA_T *rqsa)
{
  int i;

  rgngsa->cOpMode = rqsa->cOpMode;
  rgngsa->cFixMode = rqsa->cFixMode;
  for (i = 0; i < rqsa->i2NumActive; i++)
  {
    if (rgngsa->i2NumActive < NMEA_MAX_NUM_SVUSED)
    {
      rgngsa->u2SatID[rgngsa->i2NumActive] = rqsa->u2SatID[i];
	  rgngsa->eGnssType[rgngsa->i2NumActive]= rqsa->eGnssType;
      rgngsa->i2NumActive++;
    }
  }
  rgngsa->fPDOP = rqsa->fPDOP;
  rgngsa->fHDOP = rqsa->fHDOP;
  rgngsa->fVDOP = rqsa->fVDOP;  
}

//---------------------------------------------------------------------------
void GNGSV_Assign_From_GSV(GNSS_GSV_T *rgngsv, NMEA_GSV_T *rgsv)
{
  int i;


  if (rgsv->i2NumMsg == rgsv->i2MsgSqn)
  {
#if 1	
	rgngsv->eGnssType[rgngsv->i2NumGNSS] = rgsv->eGnssType;
  rgngsv->i2GnssStaView[rgngsv->i2NumGNSS] = rgsv->i2SatView;
	rgngsv->i2NumGNSS += 1;
#endif
  }

  if (rgsv->i2SatNum <= 0)
  {
    return;
  }


  if (rgsv->i2MaxSNR > rgngsv->i2MaxSNR)
  {
    rgngsv->i2MaxSNR = rgsv->i2MaxSNR;
  }
  if ((rgsv->i2MinSNR > 0) &&
      ((rgsv->i2MinSNR < rgngsv->i2MinSNR) || (rgngsv->i2MinSNR <= 0)))
  {
    rgngsv->i2MinSNR = rgsv->i2MinSNR;
  }

  for (i = 0; i < rgsv->i2SatNum; i++)
  {
    if (rgngsv->i2SatNum < NMEA_MAX_NUM_SVVIEW)
    {
      rgngsv->rSv[rgngsv->i2SatNum] = rgsv->rSv[i];
      rgngsv->i2SatNum++;
      if (rgsv->rSv[i].i2SNR > 0)
      {
        rgngsv->i2NumSVTrk++;
      }
    }
  }
}


//---------------------------------------------------------------------------
void ProcNmeaSentence()
{
	BOOL fgParserResult;
    BOOL fgValidPkt;

    fgValidPkt = fgNmeaCheckSum(rRawData.Data, rRawData.i2PacketSize - 1);

		if (fgValidPkt)
        {
          // Determine sentence type
          DetermineStnType();

          // Decode NMEA sentence

          if (rRawData.eType == STN_GGA)
          {
            //fgHasValidGGA = false;
            memset(&rSGPGGA, 0, sizeof(NMEA_GGA_T));
            fgParserResult = fgNmeaGPGGAParser(rRawData.Data, &rSGPGGA);
            if (fgParserResult)
            {
              ;//fgHasValidGGA = true;
              NMEA_DBG("NEMA GGA: utc=%02d:%02d:%02d", rSGPGGA.u2UtcHour, rSGPGGA.u2UtcMin, rSGPGGA.fUtcSec);
              NMEA_DBG("NEMA GGA: LON:%s", doubleFormatString(rSGPGGA.dfLongitude));
              NMEA_DBG("NEMA GGA: LAT:%s", doubleFormatString(rSGPGGA.dfLatitude));
              
            }
          }

          else if (rRawData.eType == STN_GLL)
          {
            //fgHasValidGLL = false;
            memset(&rSGPGLL, 0, sizeof(NMEA_GLL_T));
            fgParserResult = fgNmeaGPGLLParser(rRawData.Data, &rSGPGLL);
            if(fgParserResult)
            {
              ;//fgHasValidGLL = true;
            }
          }

          else if (rRawData.eType == STN_GSA)
          {
            NMEA_GSA_T rgpgsa;
            //fgHasValidGSA = false;
            memset(&rgpgsa, 0, sizeof(NMEA_GSA_T));
            fgParserResult = fgNmeaGPGSAParser(rRawData.Data, &rgpgsa);
            if (fgParserResult)
            {
              ;//fgHasValidGSA = true;
              Invalid_Pre_STN_Data(rRawData.eType, m_eLastDecodedSTN);
              GNGSA_Assign_From_GPGSA(&rSGNGSA, &rgpgsa);
            }
          }

          else if (rRawData.eType == STN_QSA)
          {
            NMEA_QSA_T rqsa;
            //fgHasValidGSA = false;
            memset(&rqsa, 0, sizeof(NMEA_QSA_T));
            fgParserResult = fgNmeaGPQSAParser(rRawData.Data, &rqsa);
            if (fgParserResult)
            {
              ;//fgHasValidGSA = true;
              Invalid_Pre_STN_Data(rRawData.eType, m_eLastDecodedSTN);
              GNGSA_Assign_From_QSA(&rSGNGSA, &rqsa);
            }
          }

          else if (rRawData.eType == STN_GSV)
          {
            //fgHasValidGSV = false;
            memset(&rsgpgsv, 0, sizeof(NMEA_GSV_T));
            fgParserResult = fgNmeaGPGSVParser(rRawData.Data, &rsgpgsv);
            if (fgParserResult)
            {
              ;//fgHasValidGSV = true;
              Invalid_Pre_GSV_Data();
              GNGSV_Assign_From_GSV(&rSGNGSV, &rsgpgsv);

              NMEA_DBG("NEMA GSV staNum=%d, SVTrk = %d MaxSnr=%d, MinSnr=%d", 
                    rSGNGSV.i2NumGNSS, rSGNGSV.i2NumSVTrk, rSGNGSV.i2MaxSNR, rSGNGSV.i2MinSNR);
            }
          }

          else if (rRawData.eType == STN_RMC)
          {
            //fgHasValidRMC = false;
            memset(&rSGPRMC, 0, sizeof(NMEA_RMC_T));
            fgParserResult = fgNmeaGPRMCParser(rRawData.Data, &rSGPRMC);
            if (fgParserResult)
            {
              ;//fgHasValidRMC = true;
            }
          }

          else if (rRawData.eType == STN_VTG)
          {
            //fgHasValidVTG = false;
            memset(&rSGPVTG, 0, sizeof(NMEA_VTG_T));
            fgParserResult = fgNmeaGPVTGParser(rRawData.Data, &rSGPVTG);
            if (fgParserResult)
            {
              ;//fgHasValidVTG = true;
            }
          }

          else
          {
            fgParserResult = false;
            rRawData.eType = STN_OTHER;
          }

          m_eLastDecodedSTN = rRawData.eType;
       }
}


// 1，初始化串口接收
void example_init()
{
	rxp_init_pcrx();
    ResetNmeaData();
}

// 2，rxp_init_pcrx()初始化之后把rxp_pcrx_nmea()作为回调函数注册给串口接收程序

// 3，解析rxp_pcrx_nmea()存下来的NMEA语句
void example_proc()
{
  short i2DataIdx = 0, m_i2PktDataSize = 0;
	// 判断rxp_init_pcrx()是否解析到可用的NMEA语句
	while (rxp_inst_avail(&rRawData.i2PacketType, &i2DataIdx, &m_i2PktDataSize))
 	{
 		// 把整条NMEA语句拷贝到rRawData.Data[]
		rxp_get_inst(i2DataIdx, m_i2PktDataSize, &rRawData.Data[0]);
                    
		/* we don't need <CR>, replace it with string ending symbol */
		rRawData.Data[m_i2PktDataSize - 1] = 0x00;  
		rRawData.i2PacketSize = m_i2PktDataSize;
		// 解析NMEA语句
		ProcNmeaSentence();
	}
}


NMEA_RMC_T* gpsDataGetRmc(void)
{
  return &rSGPRMC;
}

GNSS_GSV_T* gpsDataGetGSV(void)
{
  return &rSGNGSV;
}

GNSS_GSA_T* gpsDataGetGSA(void)
{
  return &rSGNGSA;
}

NMEA_GGA_T* gpsDataGetGGA(void)
{
  return &rSGPGGA;
}
