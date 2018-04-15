/******************************************************************************
*[File] NmeaParser.c
*[Description]
*[Copyright]
*    
******************************************************************************/
//---------------------------------------------------------------------------

//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NmeaParser.h"
#include "CommUtil.h"


//---------------------------------------------------------------------------
// Global Variable
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Static Variable
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Private Function
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

/* Note: input bytes are like $Gxxxx*AA<CR> */
GBOOL fgNmeaCheckSum(GCHAR* pData, GINT32 i4Size)
{
    GINT32 i;
    int limit;
    GUCHAR chksum = 0, chksum2 = 0;

    if (i4Size < 6)
    {
        return false;
    }

    chksum = pData[1];
    limit = i4Size - 2;
    for (i = 2; i < (limit); i++)
    {
      if (pData[i] != '*')
      {
        chksum ^= pData[i];

        // Exclude invalid NMEA characters
        if (pData[i] & 0x80)
        {
          return false;
        }
      }
      else
      {
        if (pData[i + 1] >= 'A')
        {
          //chksum2 = (pData[i+1]-'A'+10)<<4;
          chksum2 = (pData[i+1]-55)<<4;
        }
        else
        {
          chksum2 = (pData[i+1]-'0')<<4;
        }
        if (pData[i + 2] >= 'A')
        {
          //chksum2 += pData[i+2]-'A'+10;
          chksum2 += pData[i+2]-55;
        }
        else
        {
          chksum2 += pData[i+2]-'0';
        }
        break;
      }
    }

    // if not found character '*'
    if (i >= (i4Size - 2))
    {
      return (false);
    }

    if (chksum == chksum2)
    {
      return (true);
    }
    else
    {
      return (false);
    }
}

//---------------------------------------------------------------------------
GBOOL fgNmeaGPGGAParser(GCHAR* str, NMEA_GGA_T* gga)
{
  GINT32 i;
//  GCHAR buffer[128];
  GCHAR SubField[NMEA_MAX_ARG_NUM][NMEA_ARG_MAX_LENGTH+1];
  GFLOAT dfVal;
  GINT32 i4NumArg;


  
  i4NumArg = i4FindSubField(str, SubField);
  if (i4NumArg != 14 && i4NumArg != 15)
  {
    return false;
  }

  if (IsFloat(SubField[0]))
  {
    gga->u2UtcHour = atoi(SubField[0])/10000;
    gga->u2UtcMin = (atoi(SubField[0])%10000)/100;
    
    gga->fUtcSec = (GUINT16)((GUINT32)commUtilAtoF(SubField[0])%100);
  }
  else
  {
    gga->u2UtcHour = 0;
    gga->u2UtcMin = 0;
    gga->fUtcSec = 0.0;
    return false;
  }


  if(IsFloat(SubField[1]))
  {
    //3149.2566
    dfVal = commUtilAtoF(SubField[1]) / 100.0; //31.492566
    gga->i2LatDeg = (GINT16)(dfVal); //31
    gga->i2LatMin = (GINT16)((dfVal - gga->i2LatDeg) * 100.0); //49
    gga->dfLatSec = (dfVal - gga->i2LatDeg - gga->i2LatMin * 0.01) * 6000.0;
    gga->dfLatitude = gga->i2LatDeg + (GDOUBLE)gga->i2LatMin / 60.0 + (GDOUBLE)gga->dfLatSec / 3600.0;

  }
  else
  {
    gga->i2LatDeg = 0;
    gga->i2LatMin = 0;
    gga->dfLatSec = 0.0;
    gga->dfLatitude = 0.0;    
  }

  gga->cNS = SubField[2][0];
  if ((gga->cNS == 'S' || gga->cNS == 's'))
  {
      if (gga->i2LatDeg != 0)
      {
        gga->i2LatDeg *= -1;
      }
      if (gga->i2LatMin != 0)
      {
        gga->i2LatMin *= -1;
      }
      if (gga->dfLatSec != 0.0)
      {
        gga->dfLatSec *= -1.0;
      }
      if (gga->dfLatitude != 0.0)
      {
        gga->dfLatitude *= -1.0;
      }
  }


  if(IsFloat(SubField[3]))
  {
    dfVal = commUtilAtoF(SubField[3]) / 100.0;
    gga->i2LonDeg = (GINT16)(dfVal);
    gga->i2LonMin = (GINT16)((dfVal - gga->i2LonDeg) * 100.0);
    gga->dfLonSec = (dfVal - gga->i2LonDeg - gga->i2LonMin * 0.01) * 6000.0;
    gga->dfLongitude = gga->i2LonDeg + gga->i2LonMin / 60.0 + gga->dfLonSec / 3600.0;
  }
  else
  {
    gga->i2LonDeg = 0;
    gga->i2LonMin = 0;
    gga->dfLonSec = 0.0;
    gga->dfLongitude = 0.0;
  }

  gga->cEW = SubField[4][0];
  if((gga->cEW == 'W' || gga->cEW == 'w'))
  {
      if(gga->i2LonDeg != 0)
      {
        gga->i2LonDeg *= -1;
      }
      if (gga->i2LonMin != 0)
      {
        gga->i2LonMin *= -1;
      }
      if (gga->dfLonSec != 0.0)
      {
        gga->dfLonSec *= -1.0;
      }
      if (gga->dfLongitude != 0.0)
      {
        gga->dfLongitude *= -1.0;
      }
  }


  if (SubField[5][0] == '\0')
  {
    gga->u2PosFixQuality = 0;
  }
  else if (IsFloat(SubField[5]))
  {
    gga->u2PosFixQuality = atoi(SubField[5]);
  }
  else
  {
    return false;
  }


  if(SubField[6][0] == '\0')
  {
    gga->u2SatUsed = 0;
  }
  else
  {
    // Satellite in used has two form: 10 or 0A
    for(i = 0; i < (GINT32)strlen(SubField[6]); i++)
    {
      if(SubField[6][i] < '0' || SubField[6][i] > '9')
      {
        break;
      }
    }

    // in 10 form
    if(i == (GINT32)strlen(SubField[6]))
    {
      gga->u2SatUsed = atoi(SubField[6]);
    }
    // in 0A form
    else
    {
      if(SubField[6][1] >= 'A' && SubField[6][1] <= 'F')
      {
        gga->u2SatUsed = SubField[6][1] - 65 + 10;
      }
    }
  }


  if(IsFloat(SubField[7]))
  {
    gga->fHdop = commUtilAtoF(SubField[7]);
  }
  else
  {
    gga->fHdop = 0.0;
  }



  if(IsFloat(SubField[8]))
  {
    if(atoi(SubField[8]) > 1000.0)
    {
      //gga->fAlt = 0.0;
      gga->fAlt = commUtilAtoF(SubField[8]);
    }
    else
    {
      gga->fAlt = commUtilAtoF(SubField[8]);
    }
  }
  else
  {
    gga->fAlt = 0.0;
  }

//  gga->AltUnit = SubField[9][0];


  if(IsFloat(SubField[10]))
  {
    gga->fGeoidal = commUtilAtoF(SubField[10]);
  }
  else
  {
    gga->fGeoidal = 0.0;
  }

//  gga->Gm = SubField[11][0];


  if(IsFloat(SubField[12]))
  {
    gga->fDgpsAge = commUtilAtoF(SubField[12]);
  }
  else
  {
    gga->fDgpsAge = 0.0;
  }


  if(IsInteger(SubField[13]))
  {
    gga->i2StationID = atoi(SubField[13]);
  }
  else
  {
    gga->i2StationID = 0.00;
  }

  return true;
}

//---------------------------------------------------------------------------
GBOOL fgNmeaGPGLLParser(GCHAR* str, NMEA_GLL_T* gll)
{
  GCHAR SubField[NMEA_MAX_ARG_NUM][NMEA_ARG_MAX_LENGTH+1];
  GDOUBLE dfVal;
//  GBOOL result;
  GINT32 i4NumArg;


  i4NumArg = i4FindSubField(str, SubField);
  if ((i4NumArg == 6) ||
      (i4NumArg == 7))
  {
    if(IsFloat(SubField[0]))
    {
      dfVal = commUtilAtoF(SubField[0]) / 100.0;
      gll->i2LatDeg = (GINT16)(dfVal);
      gll->dfLatMin = (dfVal - gll->i2LatDeg) * 100.0;
    }
    else
    {
      gll->i2LatDeg = 0;
      gll->dfLatMin = 0.0;
    }

    gll->cNS = SubField[1][0];
    if ((gll->cNS == 'S' || gll->cNS == 's'))
    {
      if(gll->i2LatDeg != 0)
      {
        gll->i2LatDeg *= -1;
      }
      if(gll->dfLatMin != 0.0)
      {
        gll->dfLatMin *= -1;
      }
    }


    if(IsFloat(SubField[2]))
    {
      dfVal = commUtilAtoF(SubField[2]) / 100.0;
      gll->i2LonDeg = (GINT16)(dfVal);
      gll->dfLonMin = (dfVal - gll->i2LonDeg) * 100.0;
    }
    else
    {
      gll->i2LonDeg = 0;
      gll->dfLonMin = 0.0;
    }

    gll->cEW = SubField[3][0];
    if((gll->cEW == 'W' || gll->cEW == 'w'))
    {
      if(gll->i2LonDeg != 0)
      {
        gll->i2LonDeg *= -1;
      }
      if(gll->dfLonMin != 0.0)
      {
        gll->dfLonMin *= -1;
      }
    }

    strcpy(gll->szUtcTime, SubField[4]);
    gll->cStatus = SubField[5][0];

    gll->cMode = SubField[6][0];
  }
  else
  {
    return false;
  }
  return true;
}

//---------------------------------------------------------------------------
GBOOL fgNmeaGPGSAParser(GCHAR* str, NMEA_GSA_T* gsa)
{
  GINT32 i;
  GINT32 Id[NMEA_MAX_NUM_SVUSED];
  GCHAR SubField[NMEA_MAX_ARG_NUM][NMEA_ARG_MAX_LENGTH+1];
  GINT32 i4NumArg;
  GINT32 IndexPDOP;
  GINT32 IndexHDOP;
  GINT32 IndexVDOP;
  GINT32 NumSv;

  if ((str[1] == 'G') && (str[2] == 'P'))
  {
    gsa->eGnssType = GNSS_TYPE_GPS;
  }
  else if (((str[1] == 'B') && (str[2] == 'D')) || ((str[1] == 'G') && (str[2] == 'B')))
  {
    gsa->eGnssType = GNSS_TYPE_BEIDOU;
  }
  else if ((str[1] == 'G') && (str[2] == 'L'))
  {
    gsa->eGnssType = GNSS_TYPE_GLONASS;
  }
  else if ((str[1] == 'G') && (str[2] == 'A'))
  {
    gsa->eGnssType = GNSS_TYPE_GALILEO;
  }      
  else if ((str[1] == 'G') && (str[2] == 'N'))
  {
  	// GNSS is for UBX
    gsa->eGnssType = GNSS_TYPE_GNSS;	// GNSS_TYPE_GPS;
  } 
  else
  {
    return false;
  }
  
  i4NumArg = i4FindSubField(str, SubField);
  if ((i4NumArg != 17) &&
      (i4NumArg != 18) &&
      (i4NumArg != 20) && // TSI
      (i4NumArg != 38)) //For TD GSA
  {
    return false;
  }

  if (i4NumArg == 38) // TD GSA contain 38 commas
  {
      NumSv = i4NumArg - 6;
      IndexPDOP = i4NumArg - 4;
      IndexHDOP = i4NumArg - 3;
      IndexVDOP = i4NumArg - 2;
  }
  else
  {
      NumSv = i4NumArg - 5;
      IndexPDOP = i4NumArg - 3;
      IndexHDOP = i4NumArg - 2;
      IndexVDOP = i4NumArg - 1;
  }

  gsa->cOpMode = SubField[0][0];

  gsa->cFixMode = SubField[1][0];

  for(i = 0; i < NumSv; i++)
  {
    if (i < NMEA_MAX_NUM_SVUSED)
    {
      if (IsInteger(SubField[2+i]))
      {
//      gsa->u2SatID[i] = atoi(SubField[2+i]);
        Id[i] = atoi(SubField[2+i]);
      }
      else
      {
//      gsa->u2SatID[i] = 0;
        Id[i] = 0;
      }
    }
  }


  if (IsFloat(SubField[IndexPDOP]))
  {
    gsa->fPDOP = commUtilAtoF(SubField[IndexPDOP]);
  }
  else
  {
    gsa->fPDOP = 0.0;
  }

  if (IsFloat(SubField[IndexHDOP]))
  {
    gsa->fHDOP = commUtilAtoF(SubField[IndexHDOP]);
  }
  else
  {
    gsa->fHDOP = 0.0;
  }

  if (IsFloat(SubField[IndexVDOP]))
  {
    gsa->fVDOP = commUtilAtoF(SubField[IndexVDOP]);
  }
  else
  {
    gsa->fVDOP = 0.0;
  }


  for (i = 0; i < NumSv; i++)
  {
    if (Id[i] != 0)
    {
      gsa->u2SatID[gsa->i2NumActive] = Id[i];
      gsa->i2NumActive++;
    }

/*
    if(gsa->u2SatID[i] == 0)
    {
      break;
    }
*/
  }

  return true;
}

//---------------------------------------------------------------------------
GBOOL fgNmeaGPQSAParser(GCHAR* str, NMEA_QSA_T* qsa)
{
  GINT32 i, count;
  GINT32 Id[NMEA_MAX_NUM_SVUSED];
  GCHAR SubField[NMEA_MAX_ARG_NUM][NMEA_ARG_MAX_LENGTH+1];
  GINT32 i4NumArg;
  GINT32 IndexPDOP;
  GINT32 IndexHDOP;
  GINT32 IndexVDOP;
  GINT32 NumSv;


  if ((str[1] == 'G') && (str[2] == 'P'))
  {
    qsa->eGnssType = GNSS_TYPE_GPS;
  }
  else if ((str[1] == 'Q') && (str[2] == 'Z'))
  {
    qsa->eGnssType = GNSS_TYPE_QZSS;
  }
  else
  {
    return false;
  }

  i4NumArg = i4FindSubField(str, SubField);
  if (i4NumArg != 18)
  {
    return false;
  }

  NumSv = i4NumArg - 6;
  IndexPDOP = i4NumArg - 3;
  IndexHDOP = i4NumArg - 2;
  IndexVDOP = i4NumArg - 1;


  qsa->cGnssMode = SubField[0][0];
  qsa->cOpMode = SubField[1][0];
  qsa->cFixMode = SubField[2][0];

  for (i = 0; i < NumSv; i++)
  {
    if (i < NMEA_MAX_NUM_SVUSED)
    {
      if (IsInteger(SubField[3+i]))
      {
        Id[i] = atoi(SubField[3+i]);

        if (qsa->eGnssType == GNSS_TYPE_QZSS)
        {
          if ((Id[i] >= 1) && (Id[i] <= 5))
          {
            Id[i] += 192;
          }
          else if ((Id[i] >= 55) && (Id[i] <= 59))
          {
            Id[i] += 128;
          }
          else
          {
            return false;
          }
        }
      }
      else
      {
        Id[i] = 0;
      }
    }
  }


  if (IsFloat(SubField[IndexPDOP]))
  {
    qsa->fPDOP = commUtilAtoF(SubField[IndexPDOP]);
  }
  else
  {
    qsa->fPDOP = 0.0;
  }

  if (IsFloat(SubField[IndexHDOP]))
  {
    qsa->fHDOP = commUtilAtoF(SubField[IndexHDOP]);
  }
  else
  {
    qsa->fHDOP = 0.0;
  }

  if (IsFloat(SubField[IndexVDOP]))
  {
    qsa->fVDOP = commUtilAtoF(SubField[IndexVDOP]);
  }
  else
  {
    qsa->fVDOP = 0.0;
  }


  count = 0;
  for (i = 0; i < NMEA_MAX_NUM_SVUSED; i++)
  {
    if (Id[i] != 0)
    {
      qsa->u2SatID[count] = Id[i];
      count++;
    }
  }
  qsa->i2NumActive = count;

  return true;
}


//---------------------------------------------------------------------------
GBOOL fgNmeaGPGSVParser(GCHAR* str, NMEA_GSV_T* gsv)
{
  GINT32 i;
//  GINT32 index;
//  GINT32 i4Id[4];
//  GINT32 i4Elv[4], i4Azi[4], i4Snr[4];
  GCHAR SubField[NMEA_MAX_ARG_NUM][NMEA_ARG_MAX_LENGTH+1];
  GINT32 i4NumArg;


  if ((str[1] == 'G') && (str[2] == 'P'))
  {
    gsv->eGnssType = GNSS_TYPE_GPS;
  }
  else if ((str[1] == 'Q') && (str[2] == 'Z'))
  {
    gsv->eGnssType = GNSS_TYPE_QZSS;
  }
  else if ((str[1] == 'G') && (str[2] == 'L'))
  {
    gsv->eGnssType = GNSS_TYPE_GLONASS;
  }
  else if (((str[1] == 'B') && (str[2] == 'D')) || ((str[1] == 'G') && (str[2] == 'B')))
  {
    gsv->eGnssType = GNSS_TYPE_BEIDOU;
  }
  else if ((str[1] == 'G') && (str[2] == 'A'))
  {
    gsv->eGnssType = GNSS_TYPE_GALILEO;
  }   
  else
  {
    return false;
  }

  i4NumArg = i4FindSubField(str, SubField);

  // some vendor's GSV has an additional comma at end.
  if ((i4NumArg == 19) || (i4NumArg == 20))
  {
    gsv->i2SatNum = 4;    // 4 sv
  }
  else if ((i4NumArg == 15) || (i4NumArg == 16))
  {
    gsv->i2SatNum = 3;    // 3 sv
  }
  else if ((i4NumArg == 11) || (i4NumArg == 12))
  {
    gsv->i2SatNum = 2;    // 2 sv
  }
  else if ((i4NumArg == 7) || (i4NumArg == 8))
  {
    gsv->i2SatNum = 1;    // 1 sv
  }
  else if ((i4NumArg == 3) || (i4NumArg == 4))
  {
    gsv->i2SatNum = 0;    // 0 sv
  }
  else
  {
    return false;
  }


  if (IsInteger(SubField[0]) &&
      IsInteger(SubField[1]) &&
      IsInteger(SubField[2]))
  {
    gsv->i2NumMsg = atoi(SubField[0]);
    gsv->i2MsgSqn = atoi(SubField[1]);
    gsv->i2SatView = atoi(SubField[2]);

    // Check reasonbility
    if ((gsv->i2NumMsg < 1) ||
        (gsv->i2NumMsg > 9) ||
        (gsv->i2MsgSqn > gsv->i2NumMsg) ||
        (gsv->i2SatView > (gsv->i2NumMsg * 4)) ||
        (gsv->i2SatView > NMEA_MAX_NUM_SVVIEW))
    {
      return false;
    }
  }
  else
  {
    return false;
  }

  for (i = 0; i < gsv->i2SatNum; i++)
  {
    if (IsInteger(SubField[3 + 4 * i]))
    {
      gsv->rSv[i].u2SatID = atoi(SubField[3 + 4 * i]);
      if (gsv->eGnssType == GNSS_TYPE_QZSS)
      {
          if ((gsv->rSv[i].u2SatID >= 1) && (gsv->rSv[i].u2SatID <= 5))
          {
            gsv->rSv[i].u2SatID += 192;
          }
          else if ((gsv->rSv[i].u2SatID >= 55) && (gsv->rSv[i].u2SatID <= 59))
          {
            gsv->rSv[i].u2SatID += 128;
          }
          else
          {
            return false;
          }
      }
    }
    else
    {
      gsv->rSv[i].u2SatID = 0;
    }

    if (IsInteger(SubField[4 + 4 * i]))
    {
      gsv->rSv[i].i2Elv = atoi(SubField[4 + 4 * i]);
    }
    else
    {
      gsv->rSv[i].i2Elv = 0;
    }


    if (IsInteger(SubField[5 + 4 * i]))
    {
      gsv->rSv[i].i2Azi = atoi(SubField[5 + 4 * i]);
    }
    else
    {
      gsv->rSv[i].i2Azi = 0;
    }


    if (IsFloat(SubField[6 + 4 * i]))
    {
      gsv->rSv[i].i2SNR = commUtilAtoF(SubField[6 + 4 * i]);
    }
    else
    {
      gsv->rSv[i].i2SNR = 0;
    }

	if( gsv->eGnssType == GNSS_TYPE_GPS )
	{
		if( gsv->rSv[i].u2SatID>=33 && gsv->rSv[i].u2SatID<=64 )		// SBAS
		  	gsv->rSv[i].eGnssType = GNSS_TYPE_SBAS;
		else if( gsv->rSv[i].u2SatID>=193 && gsv->rSv[i].u2SatID<=197 )	// QZSS
		  	gsv->rSv[i].eGnssType = GNSS_TYPE_QZSS;
		else															// GPS
			gsv->rSv[i].eGnssType = gsv->eGnssType;
	}
	else																// others
		gsv->rSv[i].eGnssType = gsv->eGnssType;
  }
  
/*
  for (i = 0; i < gsv->i2SatNum; i++)
  {
    index = (gsv->i2MsgSqn - 1) * 4 + i;
    if (index < NMEA_MAX_NUM_SVVIEW)
    {
      gsv->rSv[index].u2SatID = i4Id[i];
      gsv->rSv[index].i2Elv = i4Elv[i];
      gsv->rSv[index].i2Azi = i4Azi[i];
      gsv->rSv[index].i2SNR = i4Snr[i];
    }
  }
*/

  // Update number of SV in tracking, MAX SNR, MIN SNR

//  if (gsv->i2NumMsg == gsv->i2MsgSqn)
  {
    gsv->i2NumSVTrk = 0;
    for (i = 0; i < gsv->i2SatNum; i++)
    {
//      if (i < NMEA_MAX_NUM_SVVIEW)
      {
        if (gsv->rSv[i].i2SNR > 0)
        {
          gsv->i2NumSVTrk++;
        }
      }
    }

    gsv->i2MaxSNR = 0;
    gsv->i2MinSNR = 0;
    if (gsv->i2NumSVTrk > 0)
    {
      GINT16 i2MaxSNR = 0;
      GINT16 i2MinSNR = 100;
      for (i = 0; i < gsv->i2SatNum; i++)
      {
        //if (i < NMEA_MAX_NUM_SVVIEW)
        {
          if (gsv->rSv[i].i2SNR > 0)
          {
            if (gsv->rSv[i].i2SNR > i2MaxSNR)
            {
              i2MaxSNR = gsv->rSv[i].i2SNR;
            }
            if (gsv->rSv[i].i2SNR < i2MinSNR)
            {
              i2MinSNR = gsv->rSv[i].i2SNR;
            }
          }
        }
      }
      gsv->i2MaxSNR = i2MaxSNR;
      gsv->i2MinSNR = i2MinSNR;
    }
  }


  return true;
}

//---------------------------------------------------------------------------
GBOOL fgNmeaGPRMCParser(GCHAR* str, NMEA_RMC_T* rmc)
{
  GCHAR SubField[NMEA_MAX_ARG_NUM][NMEA_ARG_MAX_LENGTH+1];
  GDOUBLE dfVal;
  GINT32 i4NumArg;


  i4NumArg = i4FindSubField(str, SubField);


  if ((i4NumArg == 12) ||
      (i4NumArg == 11) ||
      (i4NumArg == 13)) // TSI
  {
    if (IsFloat(SubField[0]))
    {
      rmc->u2UtcHour = atoi(SubField[0])/10000;
      rmc->u2UtcMin = (atoi(SubField[0])%10000)/100;
      rmc->fUtcSec = (GUINT32)commUtilAtoF(SubField[0])%100;
    }
    else
    {
      rmc->u2UtcHour = 0;
      rmc->u2UtcMin = 0;
      rmc->fUtcSec = 0.0;
      return false;
    }

    rmc->cStatus = SubField[1][0];

    if(IsFloat(SubField[2]))
    {
      dfVal = commUtilAtoF(SubField[2]) / 100.0;
      rmc->i2LatDeg = (GINT32)(dfVal);
      rmc->dfLatMin = (dfVal - rmc->i2LatDeg) * 100.0;
    }
    else
    {
      rmc->i2LatDeg = 0;
      rmc->dfLatMin = 0.0;
    }

    rmc->cNS = SubField[3][0];
    if ((rmc->cNS == 'S' || rmc->cNS == 's'))
    {
      if(rmc->i2LatDeg != 0)
      {
        rmc->i2LatDeg *= -1;
      }
      if(rmc->dfLatMin != 0.0)
      {
        rmc->dfLatMin *= -1;
      }
    }


    if(IsFloat(SubField[4]))
    {
      dfVal = commUtilAtoF(SubField[4]) / 100.0;
      rmc->i2LonDeg = (GINT16)(dfVal);
      rmc->dfLonMin = (dfVal - rmc->i2LonDeg) * 100.0;
    }
    else
    {
      rmc->i2LonDeg = 0;
      rmc->dfLonMin = 0.0;
    }

    rmc->cEW = SubField[5][0];
    if((rmc->cEW == 'W' || rmc->cEW == 'w'))
    {
      if(rmc->i2LonDeg != 0)
      {
        rmc->i2LonDeg *= -1;
      }
      if(rmc->dfLonMin != 0.0)
      {
        rmc->dfLonMin *= -1;
      }
    }


    if(IsFloat(SubField[6]))
    {
      rmc->fSpeed = commUtilAtoF(SubField[6]);
    }
    else
    {
      rmc->fSpeed = 0.0;
    }

    if(IsFloat(SubField[7]))
    {
      rmc->fCourse = commUtilAtoF(SubField[7]);
    }
    else
    {
      rmc->fCourse = 0.0;
    }

    if (IsFloat(SubField[8]))
    {
      rmc->u2UtcDay = atoi(SubField[8])/10000;
      rmc->u2UtcMonth = (atoi(SubField[8]) - rmc->u2UtcDay * 10000)/100;
      rmc->u2UtcYear = atoi(SubField[8])%100;
      if (rmc->u2UtcYear <= 50)
      {
        rmc->u2UtcYear += 2000;
      }
      else
      {
        rmc->u2UtcYear += 1900;
      }
    }
    else
    {
      rmc->u2UtcDay = 0;
      rmc->u2UtcYear = 0;
      rmc->u2UtcMonth = 0;
      return false;
    }

    if(IsFloat(SubField[9]))
    {
      rmc->fMagVar = commUtilAtoF(SubField[9]);
    }
    else
    {
      rmc->fMagVar = 0.0;
    }
    rmc->cMagEW = SubField[10][0];
    rmc->cmode = SubField[11][0];
    }
  else
  {
    return false;
  }

  return true;
}

//---------------------------------------------------------------------------
GBOOL fgNmeaGPVTGParser(GCHAR* str, NMEA_VTG_T* vtg)
{
  GCHAR SubField[NMEA_MAX_ARG_NUM][NMEA_ARG_MAX_LENGTH+1];
  GINT32 i4NumArg;


  i4NumArg = i4FindSubField(str, SubField);


  if ((i4NumArg == 8) ||
      (i4NumArg == 9))
  {
    if(IsFloat(SubField[0]))
    {
      vtg->fTrueHeading = commUtilAtoF(SubField[0]);
    }
    else
    {
      vtg->fTrueHeading = 0.0;
    }


    if(IsFloat(SubField[2]))
    {
      vtg->fMagHeading = commUtilAtoF(SubField[2]);
    }
    else
    {
      vtg->fMagHeading = 0.0;
    }

    if(IsFloat(SubField[4]))
    {
      vtg->fHSpeedKnot = commUtilAtoF(SubField[4]);
    }
    else
    {
      vtg->fHSpeedKnot = 0.0;
    }

    if(IsFloat(SubField[6]))
    {
      vtg->fHSpeedKm = commUtilAtoF(SubField[6]);
    }
    else
    {
      vtg->fHSpeedKm = 0.0;
    }

    vtg->cMode = SubField[8][0];
  }
  else
  {
    return false;
  }

  return true;
}

//---------------------------------------------------------------------------
GINT32 i4FindSubField(GCHAR* str, GCHAR sub[][NMEA_ARG_MAX_LENGTH+1])
{
  GINT32 i,j,k;
  GINT32 i4StrLength;
  GINT32 cmms_idx[NMEA_MAX_ARG_NUM+1];
  GINT32 i4NumArg = 0;
  int i4MaxField;


  i4MaxField = NMEA_MAX_ARG_NUM+1;
  for (i = 0; i < i4MaxField; i++)
  {
    cmms_idx[i] = -1;
  }

  i4StrLength = strlen(str);
  if (i4StrLength > NMEA_EX_LENGTH)
  {
    return (-1);
  }

  /* find all the commas position */
  j = 0;
  for (i = 0; i < i4StrLength; i++)
  {
    if (str[i] == ',')
    {
      cmms_idx[j++] = i;
    }

    if (j >= NMEA_MAX_ARG_NUM)
    {
      break;
    }
  }

  i4NumArg = j;


  /* find the star symbol position */
  for (i = 0; i < i4StrLength; i++)
  {
    if (str[i] == '*')
    {
      cmms_idx[j] = i;
    }
  }

  /* if no star found, something error */
  if (cmms_idx[j] == -1)
  {
    return (-1);
  }


  for (i = 0; i < i4NumArg; i++)
  {
    j = 0;
    for (k = cmms_idx[i] + 1; k < cmms_idx[i+1]; k++)
    {
      sub[i][j++] = str[k];
      if (j >= NMEA_ARG_MAX_LENGTH)
      {
        break;
      }
    }
    sub[i][j] = '\0';
  }

  
  return i4NumArg;
}

//---------------------------------------------------------------------------
GINT32 i4SplitNmeaFields(GCHAR* str, GCHAR* psub[NMEA_MAX_ARG_NUM])
{
  GINT32 i,j;
  GINT32 i4StrLength;
  GINT32 i4NumArg = 0;


  i4StrLength = strlen(str);
  if (i4StrLength > NMEA_EX_LENGTH)
  {
    return (-1);
  }

  for (i = 0; i < NMEA_MAX_ARG_NUM; i++)
  {
    psub[i] = NULL;
  }


  /* find all the commas position */
  j = 0;
  for (i = 0; i < i4StrLength; i++)
  {
    if (str[i] == ',')
    {
      str[i] = 0x00;
      psub[j++] = &str[i+1];
    }
    else if (str[i] == '*')
    {
      str[i] = 0x00;
      break;
    }

    if (j >= NMEA_MAX_ARG_NUM)
    {
      break;
    }
  }

  i4NumArg = j;

  
  return i4NumArg;
}



