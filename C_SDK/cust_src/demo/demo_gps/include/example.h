


typedef struct {        //NMEA Format structure
	GINT16 i2PacketType;  // 1: NMEA,  2: DEBUG, 3: HBD, 4: BIN
	GINT16 i2PacketSize;
	NMEA_STN_T eType;
	GCHAR Data[NMEA_EX_LENGTH];
} NMEA_STN_DATA_T;

