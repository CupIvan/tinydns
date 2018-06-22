#include <arpa/inet.h> // for uint16_t

#define OPCODE_STANDART     0
#define OPCODE_REVERSE      1
#define OPCODE_SERVER_STATE 2

#define RCODE_OK               0
#define RCODE_UNKNOW_QFORM     1
#define RCODE_NAMESERVER_ERROR 2
#define RCODE_NONAME           3
#define RCODE_UNKNOW_QTYPE     4
#define RCODE_ACCESS_DENIED    5

typedef struct
{
	char*    QNAME;
	uint16_t QTYPE;
	uint16_t QCLASS;
} TQuestion;

typedef struct __attribute__((__packed__))
{
	char*    NAME;
	uint16_t TYPE;
	uint16_t CLASS;
	uint16_t TTL;
	uint16_t RDLENGTH;
	uint16_t RDATA;
} TResource;

/*
                                1  1  1  1  1  1
  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                      ID                       |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                    QDCOUNT                    |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                    ANCOUNT                    |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                    NSCOUNT                    |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                    ARCOUNT                    |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*/
typedef struct __attribute__((__packed__))
{
	uint16_t uid;

	// COMMENT: this bit fields was rearrange becouse of big-endian
	uint16_t RD     : 1; // is recursion
	uint16_t TC     : 1; // is trunctated
	uint16_t AA     : 1; // is authority answer
	uint16_t Opcode : 4; // query type
	uint16_t QR     : 1; // QR=0 question, QR=1 answer

	uint16_t RCODE  : 4; // request state
	uint16_t Z      : 3; // reserve
	uint16_t RA     : 1; // is recursion available

	uint16_t QRCOUNT;
	uint16_t ANCOUNT;
	uint16_t NSCOUNT;
	uint16_t ARCOUNT;
} THeader;

void parse_buf(THeader *buf);
