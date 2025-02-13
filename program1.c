/* This code is an updated version of the sample code from "Computer Networks: A
 * Systems Approach," 5th Edition by Larry L. Peterson and Bruce S. Davis. Some
 * code comes from man pages, mostly getaddrinfo(3). */
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_PORT "5432"
#define MAX_LINE 256

/*
 * Lookup a host IP address and connect to it using service. Arguments match the
 * first two arguments to getaddrinfo(3).
 *
 * Returns a connected socket descriptor or -1 on error. Caller is responsible
 * for closing the returned socket.
 */
int lookup_and_connect(const char* host, const char* service);
int sendall(int s, char* buf, int* len);

int main() {
	const char* host = "www.ecst.csuchico.edu";
	const char* port = "80";
	char buf[MAX_LINE] = "GET /~kkredo/file.html HTTP/1.0\r\n\r\n";
	int s;
	int len;
	char msg[INT_MAX];
	int i;

	/* Lookup IP and connect to server */
	if ((s = lookup_and_connect(host, SERVER_PORT)) < 0) {
		exit(1);
	}

	len = strlen(buf);
	if (sendall(s, buf, &len) == -1) {
		perror("sendall");
		printf("We only sent %d bytes because of the error!\n", len);
	}

	for (len = 0; recv(s, buf, 1, 0); len++) {
		msg[len] = buf[0];
	}

	for (i = 0; i < len; i++) {
		printf(msg[i]);
	}
	printf("\n");

	close(s);

	return 0;
}

int lookup_and_connect(const char* host, const char* service) {
	struct addrinfo hints;
	struct addrinfo *rp, *result;
	int s;

	/* Translate host name into peer's IP address */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	if ((s = getaddrinfo(host, service, &hints, &result)) != 0) {
		fprintf(stderr, "stream-talk-client: getaddrinfo: %s\n",
				gai_strerror(s));
		return -1;
	}

	/* Iterate through the address list and try to connect */
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		if ((s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) ==
			-1) {
			continue;
		}

		if (connect(s, rp->ai_addr, rp->ai_addrlen) != -1) {
			break;
		}

		close(s);
	}
	if (rp == NULL) {
		perror("stream-talk-client: connect");
		return -1;
	}
	freeaddrinfo(result);

	return s;
}

int sendall(int s, char* buf, int* len) {
	int total = 0;		   // how many bytes we've sent
	int bytesleft = *len;  // how many we have left to send
	int n;

	while (total < *len) {
		n = send(s, buf + total, bytesleft, 0);
		if (n == -1) {
			break;
		}
		total += n;
		bytesleft -= n;
	}

	*len = total;  // return number actually sent here

	return n == -1 ? -1 : 0;  // return -1 on failure, 0 on success
}