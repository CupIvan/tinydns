// @url RFC 1035 https://tools.ietf.org/html/rfc1035

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h> // for usleep
#include <stdlib.h> // for exit
#include <unistd.h> // for fork

#define DNS_PORT 53

#include "log.c"
#include "parse.c"
#include "cache.c"
#include "config.c"
#include "help.c"

char version[] = "0.2";

unsigned char buf[0xFFF];

void error(char *msg) { log_s(msg); perror(msg); exit(1); }

void loop(int sockfd)
{
	 int16_t  i, n;
	uint16_t  id;
	uint16_t *ans = NULL;

	int                in_addr_len;
	struct sockaddr_in in_addr;

	int                out_socket;
	int                out_addr_len;
	struct sockaddr_in out_addr;

	memset((char *) &out_addr, 0, sizeof(out_addr));
	out_addr.sin_family = AF_INET;
	out_addr.sin_port   = htons(DNS_PORT);
	inet_aton(config.dns, (struct in_addr *)&out_addr.sin_addr.s_addr);
	out_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (out_socket < 0) error("ERROR opening socket out");

	while (1)
	{
		memset(buf, 0, sizeof(buf));

		// receive datagram
		in_addr_len = sizeof(in_addr);
		n = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *) &in_addr, &in_addr_len);
		if (n < 0) continue;

		// clear Additional section, becouse of EDNS: OPTION-CODE=000A add random bytes to the end of the question
		// EDNS: https://tools.ietf.org/html/rfc2671
		THeader* ptr = (THeader*)buf;
		if (ptr->ARCOUNT > 0)
		{
			ptr->ARCOUNT = 0;
			i = sizeof(THeader);
			while (buf[i] && i < n) i += buf[i] + 1;
			n = i + 1 + 4; // COMMENT: don't forget end zero and last 2 words
		}
		// also clear Z: it's strange, but dig util set it in 0x02
		ptr->Z = 0;

		parse_buf((THeader*)buf);

		id = *((uint16_t*)buf);

		log_b("Q-->", buf, n);

		if (ans = (uint16_t *)cache_search(buf, &n))
		{
			ans[0] = id;
			log_b("<--C", ans, n);
		}
		else
			cache_question(buf, n);

		// resend to parent
		if (!ans)
		{
			out_addr_len = sizeof(out_addr);
			n = sendto(out_socket, buf, n, 0, (struct sockaddr *) &out_addr,  out_addr_len);
			if (n < 0) { log_s("ERROR in sendto");  }

			int ck = 0;
			while (++ck < 10)
			{
				usleep(ck * 1000);
				n = recvfrom(out_socket, buf, sizeof(buf), MSG_DONTWAIT, (struct sockaddr *) &out_addr, &out_addr_len);
				if (n < 0) continue;

				cache_answer(buf, n);

				log_b("<--P", buf, n);
				if (id != *((uint16_t*)buf)) continue;

				ans = (uint16_t*)buf;
				break;
			}
			if (!ck) log_s("<--P no answer");
		}

		// send answer back
		if (ans)
		{
			n = sendto(sockfd, ans, n, 0, (struct sockaddr *) &in_addr, in_addr_len);
			if (n < 0) log_s("ERROR in sendto back");
		}
	}
}

int main(int argc, char **argv)
{
	int sockfd, n;
	struct sockaddr_in serveraddr; /* server's addr */

	if (argv[1] && 0 == strcmp(argv[1], "--version"))
	{
		printf("tinydns v%s\n", version);
		exit(0);
	}

	if (argv[1] && 0 == strcmp(argv[1], "--help"))
	{
		help();
		exit(0);
	}

	if (argv[1] && 0 == strcmp(argv[1], "-d"))
	{
		pid_t pid = fork();
		if (pid < 0)
		{
			if (pid < 0) error("Can't create daemon!");
			exit(1);
		}
		if (pid > 0) exit(0); // exit from current process
		g_debug = 0;
	}

	config_load();

	// create socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) error("ERROR opening socket");

	/* setsockopt: Handy debugging trick that lets
	* us rerun the server immediately after we kill it;
	* otherwise we have to wait about 20 secs.
	* Eliminates "ERROR on binding: Address already in use" error.
	*/
	int optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(optval));

	// bind
	memset((char *) &serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port   = htons(DNS_PORT);
	inet_aton(config.server_ip, (struct in_addr *)&serveraddr.sin_addr.s_addr);
	if (bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(struct sockaddr_in)) < 0)
		error("ERROR on binding");

	loop(sockfd);
}
