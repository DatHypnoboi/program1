#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_LINE 100000

int lookup_and_connect(const char* host, const char* service);
int sendall(int s, char* buf, int* len);

int main(int argc, char* argv[]) {
	const char* host = "www.ecst.csuchico.edu";
	const char* port = "80";
	char buf[MAX_LINE] = "GET /~kkredo/file.html HTTP/1.0\r\n\r\n";
	char* msg;
	char* bp = buf;
	int s;
	int len;
	int tagCount = 0;
	int chunkSize;
	int size;
	// int i;
	char* rest;
	size_t offset;
	int debug = 0;

	if (argc == 2) {
		chunkSize = atoi(argv[1]);
		msg = malloc(chunkSize);
	} else if (argc == 3 && !strcmp(argv[2], "-d")) {
		chunkSize = atoi(argv[1]);
		msg = malloc(chunkSize);
		debug = 1;
	} else {
		fprintf(stderr, "usage: %s chunkSize\n", argv[0]);
		printf("|%s|\n", argv[2]);
		exit(1);
	}

	if ((s = lookup_and_connect(host, port)) < 0) {
		exit(1);
	}

	len = strlen(bp);
	if (sendall(s, buf, &len) == -1) {
		perror("sendall");
		printf("We only sent %d bytes because of the error!\n", len);
	}

	for (len = 0; (size = recv(s, msg, chunkSize, 0)); len += size) {
		if (size == -1) {
			perror("recv");
			printf("s: %d\n", s);
			close(s);
			return 0;
		}
		if (size < chunkSize)
			msg[size] = '\0';
		// if (strstr(buf, "<h1>") != NULL) {
		// 	tagCount++;
		// }
		// for (i = 0; i < size - 3; i++) {
		// 	if (buf[i] == '<' && buf[i + 1] == 'h' && buf[i + 2] == '1' && buf[i
		// + 3] == '>') 		tagCount++;
		// 	// printf("%d: |%s|\n\n", tagCount, tmp);
		// 	//
		// printf("------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
		// }
		rest = msg;
		offset = 0;
		if (debug)
			printf("|%s|\n", msg);
		while ((rest = strstr(msg + offset, "<h1>")) != NULL) {
			if (debug)
				printf("\norig:|%s|\n", msg + offset);
			offset = rest + 4 - msg;
			if (debug)
				printf("rest:|%s|\n\n", msg + offset);
			tagCount++;
		}
		// printf("\nend------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
	}

	printf("Byte count = %d\nTag count = %d\n", len, tagCount);

	close(s);
	return 0;
}

int lookup_and_connect(const char* host, const char* service) {
	struct addrinfo hints;
	struct addrinfo *rp, *result;
	int s;

	/* Translate host name into peer's IP address */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
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