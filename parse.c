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

typedef struct __attribute__((__packed__))
{
	uint16_t uid;
	uint QR     : 1; // QR=1 answer, QR=0 response
	uint Opcode : 4; // query type
	uint AA     : 1; // is authority answer
	uint TC     : 1; // is trunctated
	uint RD     : 1; // is recursion
	uint RA     : 1; // is recursion available
	uint Z      : 3; // reserve
	uint RCODE  : 4; // request state
	uint16_t QRCOUNT;
	uint16_t ANCOUNT;
	uint16_t NSCOUNT;
	uint16_t ARCOUNT;
} TQuery;

void parse_buf(TQuery *buf)
{
	char domain[255];
	char *current = domain;

	strcpy(domain, (char*)(buf+1));

	int n = current[0];
	while (n > 0)
	{
		current[0] = '.';
		current += n + 1;
		n = current[0];
	}
	log_s(domain + 1); // COMMENT: skip first dot
}
