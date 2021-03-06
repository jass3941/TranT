/*
 * TNSParser.c
 *
 *  Created on: 2015. 9. 21.
 *      Author: pc
 */

/*
 ============================================================================
 Name        : TNSParser.c
 Author      :
 Version     :
 Copyright   :
 Description : TNSParser in C, Ansi-style
 ============================================================================
 */
/*
 * linkedList.c
 *
 *  Created on: 2015. 9. 16.
 *      Author: pc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TNSParser.h"


static const value_string TnsHeaderType[] = {
		{TNS_TYPE_CONNECT,   								"Connect" },
		{TNS_TYPE_ACCEPT,    								"Accept" },
		{TNS_TYPE_ACK,       								"Acknowledge" },
		{TNS_TYPE_REFUSE,    								"Refuse" },
		{TNS_TYPE_REDIRECT,  								"Redirect" },
		{TNS_TYPE_DATA,      								"Data" },
		{TNS_TYPE_NULL,      								"Null" },
		{TNS_TYPE_ABORT,     								"Abort" },
		{TNS_TYPE_RESEND,    								"Resend"},
		{TNS_TYPE_MARKER,    								"Marker"},
		{TNS_TYPE_ATTENTION, 								"Attention"},
		{TNS_TYPE_CONTROL,   								"Control"},
		{0, "UNKNOWN PROTOCOLS"}
};


static const value_string TnsBodyID[] = {
		{TNS_ID_PROTOCOL_NEGOTIATION,   					"Protocol Negotiation" },
		{TNS_ID_EXCHANGE_OF_DATA_TYPE_REPRESENTATION,    	"Exchange of Data Type representations" },
		{TNS_ID_TTI,       									"Two Task Interface" },
		{TNS_ID_OK,    										"OK server to client response" },
		{TNS_ID_EXTENDED_TTI,  								"Extended TTI" },
		{TNS_ID_EXTERNAL_PROCEDURE,      					"External Procedures" },
		{TNS_ID_SERVICE_REGISTERATION,      				"Service Registration" },
		{0, NULL}
};


static const value_string TnsBodyTTI[] = {
		{TNS_TTI_OPEN,   									"Open" },
		{TNS_TTI_QUERY,    									"Query" },
		{TNS_TTI_EXECUTE,       							"Execute" },
		{TNS_TTI_FETCH,    									"Fetch" },
		{TNS_TTI_CLOSE,  									"Close" },
		{TNS_TTI_DISCONNECT_LOGOFF,      					"Disconnect/Logoff" },
		{TNS_TTI_AUTOCOMMIT_ON,      						"Autocommit on" },
		{TNS_TTI_AUTOCOMMIT_OFF,     						"Autocommit off" },
		{TNS_TTI_COMMIT,    								"Commit"},
		{TNS_TTI_ROLLBACK,    								"Rollback"},
		{TNS_TTI_CANCEL, 									"Cancel"},
		{TNS_TTI_DESCRIBE,   								"Describe"},
		{TNS_TTI_STARTUP,   								"Startup"},
		{TNS_TTI_SHUTDOWN,   								"Shutdown"},
		{TNS_TTI_VERSION,   								"Version"},
		{TNS_TTI_K2_TRANSACTION,   							"K2 Transactions"},
		{TNS_TTI_OSQL7,   									"OSQL7"},
		{TNS_TTI_OKOD,   									"OKOD"},
		{TNS_TTI_TRANSACTION_END,   						"Transaction End"},
		{TNS_TTI_TRANSACTION_START,   						"Transaction Begin"},
		{TNS_TTI_OCCA,   									"OCCA"},
		{TNS_TTI_LOGON_PRESENT_PASSWORD,   					"Logon (present password)"},
		{TNS_TTI_LOGON_PRESENT_USERNAME,   					"Logon (present username)"},
		{TNS_TTI_LOGON_SEND_AUTH_PASSWORD,  				"Logon (present password - send AUTH_PASSWORD)"},
		{TNS_TTI_LOGON_REQUEST_AUTH_SESSIONKEY,   			"Logon (present username - request AUTH_SESSKEY)"},
		{TNS_TTI_OOTCM,   									"OOTCM"},
		{TNS_TTI_OKPFC,   									"OKPFC"},
		{TNS_TTI_QUERY2,   									"Query" },
		{0, NULL}
};


/*
 *
 * String replace Function
 *
 */

char *replaceAll(char *s, const char *olds, const char *news) {
  char *result, *sr;
  size_t i, count = 0;
  size_t oldlen = strlen(olds); if (oldlen < 1) return s;
  size_t newlen = strlen(news);


  if (newlen != oldlen) {
    for (i = 0; s[i] != '\0';) {
      if (memcmp(&s[i], olds, oldlen) == 0) count++, i += oldlen;
      else i++;
    }
  } else i = strlen(s);


  result = (char *) malloc(i + 1 + count * (newlen - oldlen));

  printf(" original %p \n", result);


  if (result == NULL)
	  return NULL;

  sr = result;

  while (*s) {
    if (memcmp(s, olds, oldlen) == 0) {
      memcpy(sr, news, newlen);
      sr += newlen;
      s  += oldlen;
    } else
    	*sr++ = *s++;
  }
  *sr = '\0';

  return result;
}


/*
 *
 * Byte Array to Integer Function
 *
 */

int bytesToInt(BYTE *byteArray, unsigned int length) {
    int value = 0;
    int j = 0;
    int i = length-1;

    for (i = length-1; i >= 0; --i) {
    	value += (byteArray[i] & 0xFF) << (8*j);
        ++j;
    }
    return value;
}



/*
 *
 * Parse bind param
 *
 */

char * bindParam(const BYTE param[1024]) {
	char* result = NULL;
	BYTE parameter[1024];

	char paramKind[3];
	char paramLength[3];


	result = "abc";
//	strcpy( result, "abc");
    int i = 0;

    memset(parameter,'0', 1024*sizeof(BYTE));
   	memcpy( parameter, param, strlen((char*)param)*sizeof(BYTE));

	while( i < strlen((char*)param)) {
		sprintf(paramLength,"%02x",*(param+i));
		sprintf(paramKind,"%02x",*(param+i+1));

		// digit
		if(paramKind[0] == 'c' ) {
			i = strlen((char*)param);
		}
		// string
		else {
			printf("string\n");

		}

		i += atoi(paramLength) + 1;

		break;

	}

	return result;

}



/*
 *
 * Packet Parsing according to packet type
 *
 *
 */

ORATNS* parse( BYTE *srcPacketData) {

	int packetType  = -1;
	int packetLength = -1;
	int packetID = -1;
	int packetTTI = -1;
	char *bindParamStr;

	/*
	 * memory allocation, TNS Header, TNS Body
	 */

	ORATNSHEADER *ptrOraHeader  = (ORATNSHEADER *) malloc( sizeof(ORATNSHEADER ));
	ORATNSBODY *ptrOraBody = (ORATNSBODY *) malloc(sizeof(ORATNSBODY ));
	ORATNS *ptrOraTNS = (ORATNS *)malloc(sizeof(ORATNS));

	/*
	 * Initialize header and body
	 */
	memset(ptrOraHeader, 0, sizeof( ORATNSHEADER));
	memset(ptrOraBody, 0, sizeof( ORATNSBODY));
	memset(ptrOraTNS, 0, sizeof( ORATNS));

	/*
	 * Copy Source to Target
	 *
	 */
	memcpy( ptrOraHeader->hdrPacketLength,  &srcPacketData[0], 2*sizeof(BYTE));
	memcpy( ptrOraHeader->hdrType,  &srcPacketData[4], 1*sizeof(BYTE));


	/*
	 * TNS Packet Type
	 */
	packetType = bytesToInt(ptrOraHeader->hdrType, 1);
	packetLength = bytesToInt(ptrOraHeader->hdrPacketLength, 2);

	switch( packetType ) {
		/*
		 * Connect ( complete )
		 */
		case TNS_TYPE_CONNECT :
			ptrOraTNS->hType = (BYTE*)TnsHeaderType[packetType-1].strptr;
			ptrOraTNS->hBodyID = (BYTE*)"NULL";
			ptrOraTNS->hTTI = (BYTE*)"NULL";
			memcpy( ptrOraTNS->hContents,  &srcPacketData[58], (packetLength - 60)*sizeof(BYTE));
			break;
		/*
		 * Accept
		 */
		case TNS_TYPE_ACCEPT :
			ptrOraTNS->hType = (BYTE*)TnsHeaderType[packetType-1].strptr;
			ptrOraTNS->hBodyID = (BYTE*)"NULL";
			ptrOraTNS->hTTI = (BYTE*)"NULL";
			memcpy( ptrOraTNS->hContents,  &srcPacketData[58], (packetLength - 60)*sizeof(BYTE));
			break;
		/*
		 * ACK
		 */
		case TNS_TYPE_ACK:
			ptrOraTNS->hType = (BYTE*)TnsHeaderType[packetType-1].strptr;
			ptrOraTNS->hBodyID = (BYTE*)"NULL";
			ptrOraTNS->hTTI = (BYTE*)"NULL";
			memcpy( ptrOraTNS->hContents,  &srcPacketData[58], (packetLength - 60)*sizeof(BYTE));
			break;
		/*
		 * REFUSE
		 */
		case TNS_TYPE_REFUSE :
			ptrOraTNS->hType = (BYTE*)TnsHeaderType[packetType-1].strptr;
			ptrOraTNS->hBodyID = (BYTE*)"NULL";
			ptrOraTNS->hTTI = (BYTE*)"NULL";
			memcpy( ptrOraTNS->hContents,  &srcPacketData[58], (packetLength - 60)*sizeof(BYTE));
			break;
		/*
		 * REDIRECT
		 */
		case TNS_TYPE_REDIRECT :
			ptrOraTNS->hType = (BYTE*)TnsHeaderType[packetType-1].strptr;
			ptrOraTNS->hBodyID = (BYTE*)"NULL";
			ptrOraTNS->hTTI = (BYTE*)"NULL";
			memcpy( ptrOraTNS->hContents,  &srcPacketData[58], (packetLength - 60)*sizeof(BYTE));
			break;

		/*
		 * TNS_TYPE_DATA ( 04 )
		 */
		case TNS_TYPE_DATA :
			ptrOraTNS->hType = (BYTE*)TnsHeaderType[packetType-1].strptr;
			memcpy( ptrOraBody->bodyDataflag,  &srcPacketData[8], 2*sizeof(BYTE));
			memcpy( ptrOraBody->bodyId,  	   &srcPacketData[10], 1*sizeof(BYTE));
			memcpy( ptrOraBody->bodyTTI,  	   &srcPacketData[11], 1*sizeof(BYTE));
			memcpy( ptrOraBody->bodyData,      &srcPacketData[12], (packetLength - 12)*sizeof(BYTE) );

			packetID = bytesToInt(ptrOraBody->bodyId, 1);
			packetTTI =  bytesToInt(ptrOraBody->bodyTTI, 1);

			switch( packetID ) {
				case TNS_ID_PROTOCOL_NEGOTIATION :
					ptrOraTNS->hBodyID = (BYTE*)TnsBodyID[0].strptr;
					ptrOraTNS->hTTI = (BYTE*)"NULL";
					break;

				// NOT Available
				case TNS_ID_EXCHANGE_OF_DATA_TYPE_REPRESENTATION :
					ptrOraTNS->hBodyID = (BYTE*)TnsBodyID[1].strptr;
					ptrOraTNS->hTTI = (BYTE*)"NULL";
					memcpy(ptrOraTNS->hContents, &srcPacketData[12], (packetLength - 12)*sizeof(BYTE));
					break;

				case TNS_ID_TTI :
					ptrOraTNS->hBodyID = (BYTE*)TnsBodyID[2].strptr;
					switch( packetTTI ) {

						// NOT USED
						case TNS_TTI_OPEN:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[0].strptr;
							break;

						// ( NOT USED )
						case TNS_TTI_QUERY:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[1].strptr;
							break;

						// BIND PARAMETER
						case TNS_TTI_EXECUTE:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[2].strptr;
							memcpy(ptrOraTNS->hContents, &srcPacketData[19] , (packetLength - 19)*sizeof(BYTE));
							bindParamStr = bindParam(ptrOraTNS->hContents);

							printf("bind Param : %s\n", bindParamStr);
							printf("bind Param : %s\n", bindParamStr);
							break;

						// Fetch ( incomplete )
						case TNS_TTI_FETCH:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[3].strptr;
							break;

						case TNS_TTI_CLOSE:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[4].strptr;
							break;

						case TNS_TTI_DISCONNECT_LOGOFF:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[5].strptr;
							break;
						case TNS_TTI_AUTOCOMMIT_ON:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[6].strptr;
							break;

						case TNS_TTI_AUTOCOMMIT_OFF:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[7].strptr;
							break;

						case TNS_TTI_COMMIT:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[8].strptr;
							break;

						case TNS_TTI_ROLLBACK:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[9].strptr;
							break;

						case TNS_TTI_CANCEL:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[10].strptr;
							break;
						// Describe ( complete )
						case TNS_TTI_DESCRIBE:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[11].strptr;
							memcpy(ptrOraTNS->hContents, ptrOraBody->bodyData , (packetLength - 12)*sizeof(BYTE));
							break;

						case TNS_TTI_STARTUP:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[12].strptr;
							break;

						case TNS_TTI_SHUTDOWN:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[13].strptr;
							break;

						case TNS_TTI_VERSION:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[14].strptr;
							break;

						case TNS_TTI_K2_TRANSACTION:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[15].strptr;
							break;

						case TNS_TTI_OSQL7:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[16].strptr;
							break;

						case TNS_TTI_OKOD:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[17].strptr;
							break;

						case TNS_TTI_TRANSACTION_END:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[18].strptr;
							break;

						case TNS_TTI_TRANSACTION_START:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[19].strptr;
							break;

						case TNS_TTI_OCCA:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[20].strptr;
							break;

						case TNS_TTI_LOGON_PRESENT_PASSWORD:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[21].strptr;
							break;

						case TNS_TTI_LOGON_PRESENT_USERNAME:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[22].strptr;
							break;

						case TNS_TTI_LOGON_SEND_AUTH_PASSWORD:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[23].strptr;
							break;

						case TNS_TTI_LOGON_REQUEST_AUTH_SESSIONKEY:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[24].strptr;
							break;

						case TNS_TTI_OOTCM:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[25].strptr;
							break;

						case TNS_TTI_OKPFC:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[26].strptr;
							break;
						// Query ( Complete )
						case TNS_TTI_QUERY2:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[27].strptr;
							memcpy(ptrOraTNS->hContents, &srcPacketData[37] , (packetLength - (37))*sizeof(BYTE));
							char * pStr = replaceAll((char*)ptrOraTNS->hContents, "@","" );

							printf("Pointer Addr  : %p\n", pStr);
							printf("Pointer Value : %s\n", pStr);

							strcpy((char*)ptrOraTNS->hContents,  pStr);
							free(pStr);
							break;

						// ERROR
						default:
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[28].strptr;
							break;
					}

					//memcpy(ptrOraTNS->hContents, ptrOraBody->bodyData , (packetLength - 12)*sizeof(BYTE));
					break;

				// ID : 06 , SUB FUNCTION : 02 ( Response complete )
				case TNS_ID_TTI_RES :
					switch( packetTTI ) {
						case TNS_TTI_OPEN:
							ptrOraTNS->hBodyID = (BYTE*)TnsBodyID[2].strptr;
							ptrOraTNS->hTTI = (BYTE*)TnsBodyTTI[0].strptr;
							memcpy(ptrOraTNS->hContents, &srcPacketData[23], (packetLength - 23)*sizeof(BYTE) );
							break;
					}
					break;


				case TNS_ID_OK :
					ptrOraTNS->hBodyID = (BYTE*)TnsBodyID[3].strptr;
					ptrOraTNS->hTTI = (BYTE*)"NULL";
					memcpy(ptrOraTNS->hContents, &srcPacketData[204], (packetLength - 204)*sizeof(BYTE) );
//					strcpy((char*)ptrOraTNS->hContents,  replaceAll((char*)ptrOraTNS->hContents, "\"","," ));
					break;

				case TNS_ID_EXTENDED_TTI :
					ptrOraTNS->hBodyID = (BYTE*)TnsBodyID[4].strptr;
					ptrOraTNS->hTTI = (BYTE*)"NULL";
					memcpy(ptrOraTNS->hContents, ptrOraBody->bodyData , (packetLength - 12)*sizeof(BYTE));
					break;

				case TNS_ID_EXTERNAL_PROCEDURE :
					ptrOraTNS->hBodyID = (BYTE*)TnsBodyID[5].strptr;
					ptrOraTNS->hTTI = (BYTE*)"NULL";
					memcpy(ptrOraTNS->hContents, ptrOraBody->bodyData, (packetLength - 12)*sizeof(BYTE) );
					break;

				case TNS_ID_SERVICE_REGISTERATION :
					ptrOraTNS->hBodyID = (BYTE*)TnsBodyID[6].strptr;
					ptrOraTNS->hTTI = (BYTE*)"NULL";
					memcpy(ptrOraTNS->hContents, ptrOraBody->bodyData, (packetLength - 12)*sizeof(BYTE) );
					break;

				default:
					ptrOraTNS->hBodyID = (BYTE*)TnsBodyID[7].strptr;
					ptrOraTNS->hTTI = (BYTE*)"NULL";
					memcpy(ptrOraTNS->hContents, ptrOraBody->bodyData, (packetLength - 12)*sizeof(BYTE) );
					break;

			}
			break;

		/*
		 * TNS_TYPE_NULL
		 */
		case TNS_TYPE_NULL :
			ptrOraTNS->hType = (BYTE*)TnsHeaderType[packetType-1].strptr;
			ptrOraTNS->hBodyID = (BYTE*) "NULL";
			ptrOraTNS->hTTI = (BYTE*) "NULL";
			memcpy( ptrOraTNS->hContents,  &srcPacketData[58], (packetLength - 60)*sizeof(BYTE));
			break;

		/*
		 * TNS_TYPE_ABORT
		 */
		case TNS_TYPE_ABORT :
			ptrOraTNS->hType = (BYTE*)TnsHeaderType[packetType-1].strptr;
			ptrOraTNS->hBodyID = (BYTE*) "NULL";
			ptrOraTNS->hTTI = (BYTE*) "NULL";
			memcpy( ptrOraTNS->hContents,  &srcPacketData[58], (packetLength - 60)*sizeof(BYTE));
			break;

		/*
		 * TNS_TYPE_RESEND
		 */
		case TNS_TYPE_RESEND :
			ptrOraTNS->hType = (BYTE*)TnsHeaderType[packetType-1].strptr;
			ptrOraTNS->hBodyID = (BYTE*) "NULL";
			ptrOraTNS->hTTI = (BYTE*) "NULL";
			memcpy( ptrOraTNS->hContents,  &srcPacketData[58], (packetLength - 60)*sizeof(BYTE));
			break;

		/*
		 * TNS_TYPE_MARKER
		 */
		case TNS_TYPE_MARKER :
			ptrOraTNS->hType = (BYTE*)TnsHeaderType[packetType-1].strptr;
			ptrOraTNS->hBodyID = (BYTE*) "NULL";
			ptrOraTNS->hTTI = (BYTE*) "NULL";
			memcpy( ptrOraTNS->hContents,  &srcPacketData[58], (packetLength - 60)*sizeof(BYTE));
			break;

		/*
		 * TNS_TYPE_ATTENTION
		 */
		case TNS_TYPE_ATTENTION :
			ptrOraTNS->hType = (BYTE*)TnsHeaderType[packetType-1].strptr;
			ptrOraTNS->hBodyID = (BYTE*) "NULL";
			ptrOraTNS->hTTI = (BYTE*) "NULL";
			memcpy( ptrOraTNS->hContents,  &srcPacketData[58], (packetLength - 60)*sizeof(BYTE));
			break;

		/*
		 * TNS_TYPE_CONTROL
		 */
		case TNS_TYPE_CONTROL :
			ptrOraTNS->hType = (BYTE*)TnsHeaderType[packetType-1].strptr;
			ptrOraTNS->hBodyID = (BYTE*) "NULL";
			ptrOraTNS->hTTI = (BYTE*) "NULL";
			memcpy( ptrOraTNS->hContents,  &srcPacketData[58], (packetLength - 60)*sizeof(BYTE));
			break;

		default :
			ptrOraTNS->hType = (BYTE*)"Unknown Packet Type";
			ptrOraTNS->hBodyID = (BYTE*) "NULL";
			ptrOraTNS->hTTI = (BYTE*) "NULL";
			memcpy( ptrOraTNS->hContents, (BYTE*)"ERROR!", sizeof(5));
			break;
	}

	free( ptrOraHeader);
	free( ptrOraBody );
	return ptrOraTNS;
}




int main(int argc, char **argv){
	ORATNS *ptrOraTNS = NULL;

	/*
	 *  Packet type :Connect
	 */


	BYTE packetSource[] = {
			0x00,0xe1,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x36,0x01,0x2c,0x00,0x00,0x08,0x00,
			0x7f,0xff,0xa3,0x0a,0x00,0x00,0x01,0x00,0x00,0xa7,0x00,0x3a,0x00,0x00,0x02,0x00,
			0x41,0x41,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x70,0x00,0x00,
			0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x28,0x44,0x45,0x53,0x43,0x52,
			0x49,0x50,0x54,0x49,0x4f,0x4e,0x3d,0x28,0x41,0x44,0x44,0x52,0x45,0x53,0x53,0x3d,
			0x28,0x50,0x52,0x4f,0x54,0x4f,0x43,0x4f,0x4c,0x3d,0x54,0x43,0x50,0x29,0x28,0x48,
			0x4f,0x53,0x54,0x3d,0x31,0x31,0x30,0x2e,0x33,0x2e,0x31,0x2e,0x39,0x31,0x29,0x28,
			0x50,0x4f,0x52,0x54,0x3d,0x31,0x35,0x32,0x33,0x29,0x29,0x28,0x43,0x4f,0x4e,0x4e,
			0x45,0x43,0x54,0x5f,0x44,0x41,0x54,0x41,0x3d,0x28,0x53,0x45,0x52,0x56,0x49,0x43,
			0x45,0x5f,0x4e,0x41,0x4d,0x45,0x3d,0x6c,0x74,0x69,0x73,0x29,0x28,0x43,0x49,0x44,
			0x3d,0x28,0x50,0x52,0x4f,0x47,0x52,0x41,0x4d,0x3d,0x43,0x3a,0x5c,0x73,0x75,0x6e,
			0x63,0x68,0x65,0x6f,0x6e,0x5c,0x4a,0x43,0x74,0x61,0x78,0x50,0x43,0x32,0x2e,0x45,
			0x58,0x45,0x29,0x28,0x48,0x4f,0x53,0x54,0x3d,0xbf,0xb5,0xc4,0xa1,0xc2,0xf7,0xb7,
			0xae,0x29,0x28,0x55,0x53,0x45,0x52,0x3d,0x4d,0x79,0x63,0x6f,0x6d,0x29,0x29,0x29,
			0x29

	};


	/*
	 *  Packet type :DATA
	 *  Exchange of Data Type representations
	 */


	BYTE packetSource1[] = {
			 0x03,0xc4,0x00,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x02,0x4e,0x03,0x4e,0x03,0x03
			,0x02,0x06,0x01,0x02,0x02,0x01,0x80,0x00,0x00,0x00,0x3c,0x3c,0x3c,0x80,0x00,0x00
			,0x00,0x01,0x01,0x01,0x00,0x02,0x02,0x0a,0x00,0x08,0x08,0x01,0x00,0x0c,0x0c,0x0a
			,0x00,0x17,0x17,0x01,0x00,0x18,0x18,0x01,0x00,0x19,0x19,0x18,0x19,0x01,0x00,0x1a
			,0x1a,0x19,0x1a,0x01,0x00,0x1b,0x1b,0x0a,0x1b,0x01,0x00,0x1c,0x1c,0x16,0x1c,0x01
			,0x00,0x1d,0x1d,0x17,0x1d,0x01,0x00,0x1e,0x1e,0x17,0x1e,0x01,0x00,0x1f,0x1f,0x19
			,0x1f,0x01,0x00,0x20,0x20,0x0a,0x20,0x01,0x00,0x21,0x21,0x0a,0x21,0x01,0x00,0x0a
			,0x0a,0x01,0x00,0x0b,0x0b,0x01,0x00,0x22,0x22,0x01,0x00,0x23,0x23,0x01,0x23,0x01
			,0x00,0x24,0x24,0x01,0x00,0x25,0x25,0x01,0x00,0x26,0x26,0x01,0x00,0x28,0x28,0x01
			,0x00,0x29,0x29,0x01,0x00,0x2a,0x2a,0x01,0x00,0x2b,0x2b,0x01,0x00,0x2c,0x2c,0x01
			,0x00,0x2d,0x2d,0x01,0x00,0x2e,0x2e,0x01,0x00,0x2f,0x2f,0x01,0x00,0x30,0x30,0x01
			,0x00,0x31,0x31,0x01,0x00,0x32,0x32,0x01,0x00,0x33,0x33,0x01,0x00,0x34,0x34,0x01
			,0x00,0x35,0x35,0x01,0x00,0x36,0x36,0x01,0x00,0x37,0x37,0x01,0x00,0x38,0x38,0x01
			,0x00,0x39,0x39,0x01,0x00,0x3b,0x3b,0x01,0x00,0x3c,0x3c,0x01,0x00,0x3d,0x3d,0x01
			,0x00,0x3e,0x3e,0x01,0x00,0x3f,0x3f,0x01,0x00,0x40,0x40,0x01,0x00,0x41,0x41,0x01
			,0x00,0x42,0x42,0x01,0x00,0x43,0x43,0x01,0x00,0x47,0x47,0x01,0x00,0x48,0x48,0x01
			,0x00,0x49,0x49,0x01,0x00,0x4b,0x4b,0x01,0x00,0x4d,0x4d,0x01,0x00,0x4e,0x4e,0x01
			,0x00,0x4f,0x4f,0x01,0x00,0x50,0x50,0x01,0x00,0x51,0x51,0x01,0x00,0x52,0x52,0x01
			,0x00,0x53,0x53,0x01,0x00,0x54,0x54,0x01,0x00,0x55,0x55,0x01,0x00,0x56,0x56,0x01
			,0x00,0x57,0x57,0x01,0x57,0x01,0x00,0x59,0x59,0x01,0x00,0x5a,0x5a,0x01,0x00,0x5c
			,0x5c,0x01,0x00,0x5d,0x5d,0x01,0x00,0x62,0x62,0x01,0x00,0x63,0x63,0x01,0x00,0x67
			,0x67,0x01,0x00,0x6b,0x6b,0x01,0x00,0x75,0x75,0x01,0x00,0x78,0x78,0x01,0x00,0x7c
			,0x7c,0x01,0x42,0x01,0x00,0x7d,0x7d,0x01,0x00,0x7e,0x7e,0x01,0x00,0x7f,0x7f,0x01
			,0x00,0x80,0x80,0x01,0x00,0x81,0x81,0x01,0x00,0x82,0x82,0x01,0x00,0x83,0x83,0x01
			,0x00,0x84,0x84,0x01,0x00,0x85,0x85,0x01,0x00,0x86,0x86,0x01,0x00,0x87,0x87,0x01
			,0x00,0x89,0x89,0x01,0x00,0x8a,0x8a,0x01,0x00,0x8b,0x8b,0x01,0x00,0x8c,0x8c,0x01
			,0x00,0x8d,0x8d,0x01,0x00,0x8e,0x8e,0x01,0x00,0x8f,0x8f,0x01,0x00,0x90,0x90,0x01
			,0x00,0x91,0x91,0x01,0x00,0x94,0x94,0x01,0x25,0x01,0x00,0x95,0x95,0x01,0x00,0x96
			,0x96,0x01,0x00,0x97,0x97,0x01,0x00,0x9d,0x9d,0x01,0x00,0x9e,0x9e,0x01,0x00,0x9f
			,0x9f,0x01,0x00,0xa0,0xa0,0x01,0x00,0xa1,0xa1,0x01,0x00,0xa2,0xa2,0x01,0x00,0xa3
			,0xa3,0x01,0x00,0xa4,0xa4,0x01,0x00,0xa5,0xa5,0x01,0x00,0xa6,0xa6,0x01,0x00,0xa7
			,0xa7,0x01,0x00,0xa8,0xa8,0x01,0x00,0xa9,0xa9,0x01,0x00,0xaa,0xaa,0x01,0x00,0xab
			,0xab,0x01,0x00,0xad,0xad,0x01,0x00,0xae,0xae,0x01,0x00,0xaf,0xaf,0x01,0x00,0xb0
			,0xb0,0x01,0x00,0xb1,0xb1,0x01,0x00,0xc1,0xc1,0x01,0x00,0xc2,0xc2,0x01,0x25,0x01
			,0x00,0xc6,0xc6,0x01,0x00,0xc7,0xc7,0x01,0x00,0xc8,0xc8,0x01,0x00,0xc9,0xc9,0x01
			,0x00,0xca,0xca,0x01,0x9f,0x01,0x00,0xcb,0xcb,0x01,0xa0,0x01,0x00,0xcc,0xcc,0x01
			,0xa2,0x01,0x00,0xcd,0xcd,0x01,0xa3,0x01,0x00,0xce,0xce,0x01,0xb1,0x01,0x00,0xcf
			,0xcf,0x01,0x22,0x01,0x00,0xd2,0xd2,0x01,0x00,0xd3,0xd3,0x01,0xab,0x01,0x00,0xd6
			,0xd6,0x01,0x00,0xd7,0xd7,0x01,0x00,0xd8,0xd8,0x01,0x00,0xd9,0xd9,0x01,0x00,0xda
			,0xda,0x01,0x00,0xdb,0xdb,0x01,0x00,0xdc,0xdc,0x01,0x00,0xdd,0xdd,0x01,0x00,0xde
			,0xde,0x01,0x00,0xdf,0xdf,0x01,0x00,0xe0,0xe0,0x01,0x00,0xe1,0xe1,0x01,0x00,0xe2
			,0xe2,0x01,0x00,0xe3,0xe3,0x01,0x6b,0x01,0x00,0xe4,0xe4,0x01,0x00,0xe5,0xe5,0x01
			,0x00,0xe6,0xe6,0x01,0x00,0xea,0xea,0x01,0x00,0x03,0x02,0x0a,0x00,0x04,0x02,0x0a
			,0x00,0x05,0x01,0x01,0x00,0x06,0x02,0x0a,0x00,0x07,0x02,0x0a,0x00,0x09,0x01,0x01
			,0x00,0x0d,0x00,0x0e,0x00,0x0f,0x17,0x01,0x00,0x10,0x00,0x11,0x00,0x12,0x00,0x13
			,0x00,0x14,0x00,0x15,0x00,0x16,0x00,0x27,0x78,0x01,0x5d,0x01,0x26,0x01,0x00,0x3a
			,0x6d,0x01,0x00,0x44,0x02,0x0a,0x00,0x45,0x00,0x46,0x00,0x4a,0x00,0x4c,0x00,0x58
			,0x00,0x5b,0x02,0x0a,0x00,0x5e,0x01,0x01,0x00,0x5f,0x17,0x01,0x00,0x60,0x60,0x01
			,0x00,0x61,0x60,0x01,0x00,0x64,0x00,0x65,0x00,0x66,0x66,0x01,0x00,0x68,0x00,0x69
			,0x00,0x6a,0x6a,0x01,0x00,0x6c,0x6d,0x01,0x00,0x6d,0x6d,0x01,0x00,0x6e,0x6f,0x01
			,0x00,0x6f,0x6f,0x01,0x00,0x70,0x70,0x01,0x00,0x71,0x71,0x01,0x00,0x72,0x72,0x01
			,0x00,0x73,0x73,0x01,0x00,0x74,0x66,0x01,0x00,0x76,0x00,0x77,0x00,0x79,0x6d,0x00
			,0x00,0x7a,0x6d,0x00,0x00,0x7b,0x6d,0x00,0x00,0x88,0x00,0x92,0x92,0x01,0x00,0x93
			,0x93,0x01,0x00,0x98,0x02,0x0a,0x00,0x99,0x02,0x0a,0x00,0x9a,0x02,0x0a,0x00,0x9b
			,0x01,0x01,0x00,0x9c,0x0c,0x0a,0x00,0xac,0x02,0x0a,0x00,0xb2,0xb2,0x01,0x00,0xb3
			,0xb3,0x01,0x00,0xb4,0xb4,0x01,0x00,0xb5,0xb5,0x01,0x00,0xb6,0xb6,0x01,0x00,0xb7
			,0xb7,0x01,0x00,0xb8,0x0c,0x0a,0x00,0xb9,0xb2,0x01,0x00,0xba,0xb3,0x01,0x00,0xbb
			,0xb4,0x01,0x00,0xbc,0xb5,0x01,0x00,0xbd,0xb6,0x01,0x00,0xbe,0xb7,0x01,0x00,0xbf
			,0x00,0xc0,0x00,0xc3,0x70,0x01,0x00,0xc4,0x71,0x01,0x00,0xc5,0x72,0x01,0x00,0xd0
			,0xd0,0x01,0x00,0xd1,0x00,0xd4,0x00,0xd5,0x00,0xe7,0xe7,0x01,0x00,0xe8,0xe7,0x01
			,0x00,0xe9,0x00,0x00
	};

	/*
	 *  Packet : DATA > TTI > Query
	 *
	 */

	BYTE packetSource2[] = {
			 0x00,0xf5,0x00,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x03,0x47,0x0e,0x02,0x80,0x09
			,0x01,0x03,0x01,0x02,0x01,0x58,0x00,0x00,0x01,0x01,0x07,0x01,0x01,0x02,0x00,0x00
			,0x00,0x01,0x01,0x02,0xfe,0x40,0x53,0x65,0x6c,0x65,0x63,0x74,0x20,0xc0,0xda,0xc4
			,0xa1,0xb4,0xdc,0xc3,0xbc,0xc4,0xda,0xb5,0xe5,0x20,0x2c,0x20,0xbc,0xbc,0xb8,0xf1
			,0xc4,0xda,0xb5,0xe5,0x20,0x2c,0x20,0xb3,0xb3,0xbc,0xbc,0xb9,0xf8,0xc8,0xa3,0x20
			,0x2c,0x20,0xb3,0xb3,0xbc,0xbc,0xc0,0xda,0xb9,0xf8,0xc8,0xa3,0x20,0x2c,0x20,0xb3
			,0xb3,0xbc,0xbc,0xc0,0xda,0xb8,0x40,0xed,0x20,0x2c,0x20,0xb3,0xb3,0xbc,0xbc,0xc0
			,0xda,0xc1,0xd6,0xbc,0xd2,0x20,0x2c,0x20,0xc2,0xf7,0xb7,0xae,0xb9,0xf8,0xc8,0xa3
			,0x20,0x2c,0x20,0xc3,0xbc,0xb3,0xb3,0xbe,0xd7,0x20,0x2c,0x20,0xbc,0xd2,0xc0,0xaf
			,0xc0,0xda,0xb9,0xf8,0xc8,0xa3,0x20,0x2c,0x20,0xb0,0xe1,0xbc,0xd5,0xbe,0xd7,0x20
			,0x46,0x72,0x6f,0x6d,0x20,0x76,0x5f,0x2c,0x64,0x70,0x68,0x6f,0x30,0x30,0x32,0x20
			,0x77,0x68,0x65,0x72,0x65,0x20,0xc3,0xbc,0xb3,0xb3,0xbe,0xd7,0x20,0x3e,0x3d,0x20
			,0x3a,0x31,0x20,0x61,0x6e,0x64,0x20,0xc3,0xbc,0xb3,0xb3,0xbe,0xd7,0x20,0x3c,0x3d
			,0x20,0x3a,0x32,0x20,0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x01,0x00
			,0x00,0x01,0x16,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x01,0x00,0x00,0x01,0x16,0x00
			,0x00,0x00,0x00,0x00,0x00
	};
	/*
	 *  Packet type : DATA > TTI > Describe
	 *
	 */

	BYTE packetSource3[] = {
			 0x00,0x1b,0x00,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x03,0x2b,0x0f,0x01,0x03,0x01
			,0x01,0x01,0x01,0x01,0x20,0x01,0x01,0x02,0x07,0x80,0x01
	};

	/*
	 *  Response
	 *  Packet type : DATA > OK server to client response
	 *
	 */
	BYTE packetSource4[] = {
		 0x01,0x2d,0x00,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x08,0x01,0x0a,0x01,0x0a,0x60
		,0x80,0x00,0x00,0x01,0x05,0x00,0x00,0x00,0x00,0x02,0x03,0x4e,0x01,0x00,0x0c,0x00
		,0x00,0x00,0x60,0x80,0x00,0x00,0x01,0x06,0x00,0x00,0x00,0x00,0x02,0x03,0x4e,0x01
		,0x01,0x08,0x00,0x00,0x00,0x60,0x80,0x00,0x00,0x01,0x1a,0x00,0x00,0x00,0x00,0x02
		,0x03,0x4e,0x01,0x01,0x08,0x00,0x00,0x00,0x60,0x80,0x00,0x00,0x01,0x0d,0x00,0x00
		,0x00,0x00,0x02,0x03,0x4e,0x01,0x01,0x0a,0x00,0x00,0x00,0x01,0x80,0x00,0x00,0x01
		,0x50,0x00,0x00,0x00,0x00,0x02,0x03,0x4e,0x01,0x01,0x08,0x00,0x00,0x00,0x01,0x80
		,0x00,0x00,0x01,0xc8,0x00,0x00,0x00,0x00,0x02,0x03,0x4e,0x01,0x01,0x0a,0x00,0x00
		,0x00,0x60,0x80,0x00,0x00,0x01,0x0c,0x00,0x00,0x00,0x00,0x02,0x03,0x4e,0x01,0x01
		,0x08,0x00,0x00,0x00,0x02,0x00,0x0d,0x00,0x01,0x16,0x00,0x00,0x00,0x00,0x00,0x00
		,0x00,0x06,0x00,0x00,0x00,0x60,0x80,0x00,0x00,0x01,0x0d,0x00,0x00,0x00,0x00,0x02
		,0x03,0x4e,0x01,0x01,0x0a,0x00,0x00,0x00,0x02,0x00,0x00,0x81,0x01,0x16,0x00,0x00
		,0x00,0x00,0x00,0x00,0x01,0x06,0x00,0x00,0x00,0x01,0x60,0x60,0xc0,0xda,0xc4,0xa1
		,0xb4,0xdc,0xc3,0xbc,0xc4,0xda,0xb5,0xe5,0x22,0xbc,0xbc,0xb8,0xf1,0xc4,0xda,0xb5
		,0xe5,0x22,0xb3,0xb3,0xbc,0xbc,0xb9,0xf8,0xc8,0xa3,0x22,0xb3,0xb3,0xbc,0xbc,0xc0
		,0xda,0xb9,0xf8,0xc8,0xa3,0x22,0xb3,0xb3,0xbc,0xbc,0xc0,0xda,0xb8,0xed,0x22,0xb3
		,0xb3,0xbc,0xbc,0xc0,0xda,0xc1,0xd6,0xbc,0xd2,0x22,0xc2,0xf7,0xb7,0xae,0xb9,0xf8
		,0xc8,0xa3,0x22,0xc3,0xbc,0xb3,0xb3,0xbe,0xd7,0x22,0xbc,0xd2,0xc0,0xaf,0xc0,0xda
		,0xb9,0xf8,0xc8,0xa3,0x22,0xb0,0xe1,0xbc,0xd5,0xbe,0xd7,0x22,0x09
	};

	/*
	 *  bind parameter
	 *  Packet type : DATA > Two Task Interface > Execute
	 *
	 */
	BYTE packetSource5[] = {
		 0x00,0x1d,0x00,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x03,0x04,0x10,0x01,0x03,0x01
		,0x01,0x00,0x07,0x02,0xc1,0x02,0x06,0xc5,0x0a,0x64,0x64,0x64,0x64
	};


	/*
	 *  Result Set
	 *  Packet type : DATA > Two Task Interface > Query2 ( resultset )
	 *
	 */
	BYTE packetSource6[] = {
	 0x00,0xf1,0x00,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x06,0x02,0x01,0x0a,0x00,0x01
	,0x01,0x00,0x00,0x00,0x07,0x05,0x34,0x36,0x31,0x35,0x30,0x00,0x06,0x31,0x31,0x31
	,0x30,0x30,0x30,0x00,0x1a,0x31,0x31,0x31,0x30,0x30,0x30,0x2d,0x31,0x39,0x39,0x37
	,0x31,0x30,0x2d,0x31,0x2d,0x36,0x36,0x30,0x2d,0x30,0x37,0x37,0x34,0x34,0x31,0x00
	,0x0d,0x31,0x34,0x30,0x31,0x31,0x31,0x30,0x30,0x30,0x35,0x32,0x31,0x32,0x00,0x0c
	,0xc8,0xab,0xc0,0xcd,0xc1,0xd6,0xc5,0xc3,0xb0,0xc7,0xbc,0xb3,0x00,0x35,0xb0,0xe6
	,0xb1,0xe2,0xb5,0xb5,0x20,0xba,0xce,0xc3,0xb5,0xbd,0xc3,0xbf,0xc0,0xc1,0xa4,0xb1
	,0xb8,0x20,0xbb,0xef,0xc0,0xdb,0xb7,0xce,0x34,0x39,0x36,0xb9,0xf8,0xb1,0xe6,0x20
	,0x35,0x36,0x2c,0x20,0xbd,0xc9,0xbb,0xf3,0xc8,0xaf,0xb4,0xec,0x20,0x28,0xc0,0xdb
	,0xb5,0xbf,0x29,0x00,0x00,0x02,0x05,0x7d,0x04,0xc3,0x1c,0x1c,0x0b,0x00,0x00,0x02
	,0x05,0x7d,0x01,0x80,0x00,0x04,0x01,0x02,0x02,0x05,0x7d,0x01,0x09,0x02,0x05,0x7d
	,0x01,0x03,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	,0x12,0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x28,0x4f,0x52,0x41,0x2d,0x30,0x31,0x34
	,0x30,0x35,0x3a,0x20,0x66,0x65,0x74,0x63,0x68,0x65,0x64,0x20,0x63,0x6f,0x6c,0x75
	,0x6d,0x6e,0x20,0x76,0x61,0x6c,0x75,0x65,0x20,0x69,0x73,0x20,0x4e,0x55,0x4c,0x4c
	,0x0a
	};



	int i;

	for( i = 0; i < 1000000; i++) {
	ptrOraTNS = parse( packetSource);


	printf( "--------------------Packet type :Connect-------------------------\n");
	printf( "Packet Type : %s\n", ptrOraTNS->hType);
	printf( "Packet ID : %s\n", ptrOraTNS->hBodyID);
	printf( "Packet TTI Function call : %s\n", ptrOraTNS->hTTI );
	printf( "Packet Description : %s\n", ptrOraTNS->hContents);
	free(ptrOraTNS);



	printf( "----------------------Exchange of Data Type representations-----------------------\n");
	ptrOraTNS = parse( packetSource1 );
	printf( "Packet Type : %s\n", ptrOraTNS->hType);
	printf( "Packet ID : %s\n", ptrOraTNS->hBodyID);
	printf( "Packet TTI Function call : %s\n", ptrOraTNS->hTTI );
	printf( "Packet Description : %s\n", ptrOraTNS->hContents);
	free(ptrOraTNS);




	ptrOraTNS = parse( packetSource2 );
	printf( "----------------------Query ( SQL ��  )-----------------------\n");
	printf( "Packet Type : %s\n", ptrOraTNS->hType);
	printf( "Packet ID : %s\n", ptrOraTNS->hBodyID);
	printf( "Packet TTI Function call : %s\n", ptrOraTNS->hTTI );
	printf( "Packet Description : %s\n", ptrOraTNS->hContents);
	free(ptrOraTNS);


	ptrOraTNS = parse( packetSource3 );
	printf( "----------------------Desc ( SQL ��  )-----------------------\n");
	printf( "Packet Type : %s\n", ptrOraTNS->hType);
	printf( "Packet ID : %s\n", ptrOraTNS->hBodyID);
	printf( "Packet TTI Function call : %s\n", ptrOraTNS->hTTI );
	printf( "Packet Description : %s\n", ptrOraTNS->hContents);
	free(ptrOraTNS);


	ptrOraTNS = parse( packetSource4 );
	printf( "----------------------Response -----------------------\n");
	printf( "Packet Type : %s\n", ptrOraTNS->hType);
	printf( "Packet ID : %s\n", ptrOraTNS->hBodyID);
	printf( "Packet TTI Function call : %s\n", ptrOraTNS->hTTI );
	printf( "Packet Description : %s\n", ptrOraTNS->hContents);
	free(ptrOraTNS);


	ptrOraTNS = parse( packetSource5 );
	printf( "----------------------Bind param -----------------------\n");
	printf( "Packet Type : %s\n", ptrOraTNS->hType);
	printf( "Packet ID : %s\n", ptrOraTNS->hBodyID);
	printf( "Packet TTI Function call : %s\n", ptrOraTNS->hTTI );
	printf( "Packet Description : %s\n", ptrOraTNS->hContents);
	free(ptrOraTNS);


	printf( "----------------------Response ( Resultset  )-----------------------\n");
	ptrOraTNS = parse( packetSource6 );
	printf( "Packet Type : %s\n", ptrOraTNS->hType);
	printf( "Packet ID : %s\n", ptrOraTNS->hBodyID);
	printf( "Packet TTI Function call : %s\n", ptrOraTNS->hTTI );
	printf( "Packet Description : %s\n", ptrOraTNS->hContents);

	free(ptrOraTNS);
	}

	return 0;
}

