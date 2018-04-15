/******************************************************************************
*[File] NmeaParser.h
*[Description]
*[Copyright]
*    
******************************************************************************/

//---------------------------------------------------------------------------
#ifndef __NMEAPSRSER_H__
#define __NMEAPSRSER_H__


#ifdef __cplusplus
 extern "C" {
#endif

#include "TypeDef.h"


//---------------------------------------------------------------------------
#define MPS_TO_KNOTS 	1.9438400f // m/s to knots
#define KNOTS_TO_MPS 	0.5144456f // knots to m/s

//---------------------------------------------------------------------------
#define NMEA_STD_LENGTH 	82
#define NMEA_EX_LENGTH 		256  // In order to support YLS proprietary sentence
#define NMEA_MAX_GPS_ID 	32
#define NMEA_MAX_GNSS_ID 	254 // instead of 255, to remove warning
#define NMEA_MAX_ARG_NUM 	72
#define NMEA_ARG_MAX_LENGTH 16


//---------------------------------------------------------------------------
#define YLS_DBG_FLAG_SIZE  	32
#define YLS_DBG_I1_SIZE     8
#define YLS_DBG_R4_SIZE     6
#define YLS_DB2_U2_SIZE     8
#define YLS_DB2_R4_SIZE     8
#define NUM_PSEUDO_CHNL 	32
#define NMEA_MAX_NUM_SVVIEW 32
//#define NMEA_MAX_NUM_SVUSED 12 // GPS only
#define NMEA_MAX_NUM_SVUSED 32 // GPS + GLONASS, two GNGSA sentences

//---------------------------------------------------------------------------
typedef enum{
  GNSS_TYPE_GPS 	= 0,
  GNSS_TYPE_QZSS 	= 1,
  GNSS_TYPE_GALILEO = 2,
  GNSS_TYPE_GLONASS = 3,
  GNSS_TYPE_BEIDOU 	= 4,
  GNSS_TYPE_SBAS 	= 5,
  GNSS_TYPE_GNSS 	= 10
} GNSS_ID_TYPE_T;

typedef enum{
  STN_GGA = 0,
  STN_GLL = 1,
  STN_GSA = 2,
  STN_GSV = 3,
  STN_RMC = 4,
  STN_VTG = 5,
  STN_QSA = 6,  
  STN_OTHER = 255,
} NMEA_STN_T;
//---------------------------------------------------------------------------


typedef struct {            //GPGGA -- Global Positioning System Fix Data
  GUINT16 u2UtcHour;
  GUINT16 u2UtcMin;
  GUINT16 fUtcSec;
  GDOUBLE dfLatitude;
  GINT16 i2LatDeg;
  GINT16 i2LatMin;
  GDOUBLE dfLatSec;
  GCHAR cNS;
  GDOUBLE dfLongitude;
  GINT16 i2LonDeg;
  GINT16 i2LonMin;
  GDOUBLE dfLonSec;
  GCHAR cEW;
  GUINT16 u2PosFixQuality;
  GUINT16 u2SatUsed;
  GFLOAT fHdop;
  GFLOAT fAlt;
  GFLOAT fGeoidal;
  GFLOAT fDgpsAge;
  GINT16 i2StationID;
} NMEA_GGA_T;


typedef struct {            //GPGLL -- Geographic Position - Latitude/Longitude
  GINT16 i2LatDeg;
  GDOUBLE dfLatMin;
  GCHAR cNS;
  GINT32 i2LonDeg;
  GDOUBLE dfLonMin;
  GCHAR cEW;
  GCHAR szUtcTime[16];
  GCHAR cStatus;
  GCHAR cMode;
} NMEA_GLL_T;


typedef struct {            //GPGSA -- GNSS DOP and Active Satellites
  GNSS_ID_TYPE_T eGnssType; // 0 - GPS, 1 - QZSS, 2 - GALILEO, 3 - GLONASS,4 - BEIDOU
  GCHAR cOpMode;
  GCHAR cFixMode;
  GINT16 i2NumActive;
  GUINT16 u2SatID[NMEA_MAX_NUM_SVUSED];
  GFLOAT fPDOP;
  GFLOAT fHDOP;
  GFLOAT fVDOP;
} NMEA_GSA_T;


typedef struct {            //GP/QZ QSA -- GNSS DOP and Active Satellites
  GNSS_ID_TYPE_T eGnssType; // 0 - GPS, 1 - QZSS
  GCHAR cGnssMode;  // 'S' = Single GNSS solution; 'M' = Multi-GNSS solution
  GCHAR cOpMode;
  GCHAR cFixMode;
  GINT16 i2NumActive;
  GUINT16 u2SatID[NMEA_MAX_NUM_SVUSED];
  GFLOAT fPDOP;
  GFLOAT fHDOP;
  GFLOAT fVDOP;
} NMEA_QSA_T;

typedef struct {            //GNSS GSA -- GNSS DOP and Active Satellites
#if 1	
  GINT16 i2NumGNSS;
  GUINT16 u2ActiveGNSS[4];
#endif
  GCHAR cOpMode;
  GCHAR cFixMode;
  GINT16 i2NumActive;
  GUINT16 u2SatID[NMEA_MAX_NUM_SVUSED];
  GNSS_ID_TYPE_T eGnssType[NMEA_MAX_NUM_SVUSED];
  GFLOAT fPDOP;
  GFLOAT fHDOP;
  GFLOAT fVDOP;
} GNSS_GSA_T;


typedef struct{
  GUINT16 u2SatID;
  GINT16 i2Elv;
  GINT16 i2Azi;
  GINT16 i2SNR;
  GNSS_ID_TYPE_T eGnssType;
} GSV_SV_STATUS_T;

typedef struct {            //GPGSV -- GNSS Satellites in View
  GNSS_ID_TYPE_T eGnssType; // 0 - GPS, 1 - QZSS, 2 - GALILEO, 3 - GLONASS
  GINT16 i2NumMsg;
  GINT16 i2MsgSqn;
  GINT16 i2SatView;
  GINT16 i2MaxSNR;
  GINT16 i2MinSNR;
  GINT16 i2NumSVTrk;
  GINT16 i2SatNum;
  GSV_SV_STATUS_T rSv[4];
} NMEA_GSV_T;

typedef struct {            //GNSS GSV -- GNSS Satellites in View
#if 1	
  GINT16 i2NumGNSS;
  GNSS_ID_TYPE_T eGnssType[4];
  GINT16 i2GnssStaView[4];
#endif
  GINT16 i2MaxSNR;
  GINT16 i2MinSNR;
  GINT16 i2NumSVTrk;
  GINT16 i2SatNum;
  GSV_SV_STATUS_T rSv[NMEA_MAX_NUM_SVVIEW];
} GNSS_GSV_T;

typedef struct {            //GPRMC -- Recommended Minimum Specific GNSS Data
  GUINT16 u2UtcHour;
  GUINT16 u2UtcMin;
  GFLOAT fUtcSec;
  GCHAR cStatus;
  GINT16 i2LatDeg;
  GDOUBLE dfLatMin;
  GCHAR cNS;
  GINT32 i2LonDeg;
  GDOUBLE dfLonMin;
  GCHAR cEW;
  GFLOAT fSpeed; // Speed over ground [knots]
  GFLOAT fCourse;
  GUINT16 u2UtcYear;
  GUINT16 u2UtcMonth;
  GUINT16 u2UtcDay;
  GFLOAT fMagVar;
  GCHAR cMagEW;
  GCHAR cmode;
} NMEA_RMC_T;


typedef struct{
  GFLOAT fTrueHeading;
  GFLOAT fMagHeading;
  GFLOAT fHSpeedKnot;
  GFLOAT fHSpeedKm;
  GCHAR cMode;
} NMEA_VTG_T;


//---------------------------------------------------------------------------
// Public Function
//---------------------------------------------------------------------------
  GBOOL fgNmeaCheckSum(GCHAR* pData, GINT32 i4Size);
  GBOOL fgNmeaGPGGAParser(GCHAR* str, NMEA_GGA_T* gga);
  GBOOL fgNmeaGPGLLParser(GCHAR* str, NMEA_GLL_T* gll);
  GBOOL fgNmeaGPGSAParser(GCHAR* str, NMEA_GSA_T* gsa);
  GBOOL fgNmeaGPQSAParser(GCHAR* str, NMEA_QSA_T* qsa);
  GBOOL fgNmeaGPGSVParser(GCHAR* str, NMEA_GSV_T* gsv);
  GBOOL fgNmeaGPRMCParser(GCHAR* str, NMEA_RMC_T* rmc);
  GBOOL fgNmeaGPVTGParser(GCHAR* str, NMEA_VTG_T* vtg);
  GINT32 i4FindSubField(GCHAR* str, GCHAR sub[][NMEA_ARG_MAX_LENGTH+1]);
  GINT32 i4SplitNmeaFields(GCHAR* str, GCHAR* psub[NMEA_MAX_ARG_NUM]);    
#ifdef __cplusplus
 }
#endif

#endif
