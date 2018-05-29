// @url https://habr.com/post/346098/

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h> // for usleep
#include <stdlib.h> // for exit

#define DEBUG

#define PORTNUM    53
#define DNS_IP     "127.0.0.1"
#define DNS_PARENT "8.8.8.8"

#include "log.c"
#include "cache.c"

unsigned char buf[0xFFF];

void error(char *msg) { log_s(msg); perror(msg); exit(1); }

void parse_buf(unsigned char *buf)
{
	char *current = (char *)&buf[2*6];
	char domain[255];

	strcpy(domain, current); current = domain;

	int n = domain[0];
	while (n > 0)
	{
		current[0] = '.';
		current += n + 1;
		n = current[0];
	}
	log_s(domain);
}

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
	out_addr.sin_port   = htons(PORTNUM);
	inet_aton(DNS_PARENT, (struct in_addr *)&out_addr.sin_addr.s_addr);
	out_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (out_socket < 0) error("ERROR opening socket out");

	while (1)
	{
		memset(buf, 0, sizeof(buf));

		// receive datagram
		in_addr_len = sizeof(in_addr);
		n = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *) &in_addr, &in_addr_len);
		if (n < 0) continue;

		parse_buf(buf);

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
	serveraddr.sin_family      = AF_INET;
	inet_aton(DNS_IP, (struct in_addr *)&serveraddr.sin_addr.s_addr);
	serveraddr.sin_port        = htons(PORTNUM);
	if (bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(struct sockaddr_in)) < 0)
		error("ERROR on binding");

	loop(sockfd);
}
