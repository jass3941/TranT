/*
 * TNSParser.h
 *
 *  Created on: 2015. 9. 21.
 *      Author: pc
 */

#ifndef TNSPARSER_H_
#define TNSPARSER_H_

/*
 * TNSParser.h
 *
 */

/* Packet Header Types */
#define 	TNS_TYPE_CONNECT 										1
#define 	TNS_TYPE_ACCEPT 										2
#define 	TNS_TYPE_ACK 											3
#define 	TNS_TYPE_REFUSE 										4
#define 	TNS_TYPE_REDIRECT 										5
#define 	TNS_TYPE_DATA 											6
#define 	TNS_TYPE_NULL 											7
#define 	TNS_TYPE_ABORT 											9
#define 	TNS_TYPE_RESEND 										11
#define 	TNS_TYPE_MARKER 										12
#define 	TNS_TYPE_ATTENTION 										13
#define 	TNS_TYPE_CONTROL 										14


/* Packet Body ID */
#define 	TNS_ID_PROTOCOL_NEGOTIATION								0x01
#define 	TNS_ID_EXCHANGE_OF_DATA_TYPE_REPRESENTATION				0x02
#define 	TNS_ID_TTI						 						0x03
// RESPONSE SUB TYPE = 02 Parsing Required
#define 	TNS_ID_TTI_RES						 					0x06
#define 	TNS_ID_OK							 					0x08
#define 	TNS_ID_EXTENDED_TTI						 				0x11
#define 	TNS_ID_EXTERNAL_PROCEDURE								0x20
#define 	TNS_ID_SERVICE_REGISTERATION				 			0x44


/* Packet Body TTI */
#define 	TNS_TTI_OPEN											0x02
#define 	TNS_TTI_QUERY											0x03
#define 	TNS_TTI_EXECUTE						 					0x04
#define 	TNS_TTI_FETCH						 					0x05
#define 	TNS_TTI_CLOSE						 					0x08
#define 	TNS_TTI_DISCONNECT_LOGOFF			 					0x09
#define 	TNS_TTI_AUTOCOMMIT_ON				 					0x0C
#define 	TNS_TTI_AUTOCOMMIT_OFF				 					0x0D
#define 	TNS_TTI_COMMIT						 					0x0E
#define 	TNS_TTI_ROLLBACK					 					0x0F
#define 	TNS_TTI_CANCEL						 					0x14
#define 	TNS_TTI_DESCRIBE					 					0x2B
#define 	TNS_TTI_STARTUP						 					0x30
#define 	TNS_TTI_SHUTDOWN					 					0x31
#define 	TNS_TTI_VERSION											0x3B
#define 	TNS_TTI_K2_TRANSACTION									0x43
#define 	TNS_TTI_OSQL7											0x4A
#define 	TNS_TTI_OKOD											0x5C
#define 	TNS_TTI_TRANSACTION_END									0x67
#define 	TNS_TTI_TRANSACTION_START								0x68
#define 	TNS_TTI_OCCA											0x69
#define 	TNS_TTI_LOGON_PRESENT_PASSWORD							0x51
#define 	TNS_TTI_LOGON_PRESENT_USERNAME							0x52
#define 	TNS_TTI_LOGON_SEND_AUTH_PASSWORD						0x73
#define 	TNS_TTI_LOGON_REQUEST_AUTH_SESSIONKEY					0x76
#define 	TNS_TTI_OOTCM											0x7F
#define 	TNS_TTI_OKPFC											0x8B
#define 	TNS_TTI_QUERY2											0x47

typedef struct _value_string {
    int		     value;
    const char *strptr;
} value_string;

typedef unsigned char BYTE;

typedef struct _TNS {
	BYTE *hType;
	BYTE *hBodyID;
	BYTE *hTTI;
	BYTE hContents[4096];
} ORATNS,*pORATNS;


typedef struct _oratnsheader {
	BYTE hdrPacketLength[2];
	BYTE hdrPacketChecksum[2];
	BYTE hdrType[1];
	BYTE hdrReserved[1];
	BYTE hdrHeaderCheckSum[2];
} ORATNSHEADER, *pORATNSHEADER;


typedef struct _oratnsbody {
	BYTE bodyDataflag[2];
	BYTE bodyId[1];
	BYTE bodyTTI[1];
	BYTE bodyData[4096];
} ORATNSBODY, *pORATNSBODY;


typedef struct _oratnspacket {
	ORATNSHEADER tnsHeader;
	ORATNSBODY   tnsBody;
}ORATNSPACKET, *pORATNSPACKET;


#endif /* TNSPARSER_H_ */
