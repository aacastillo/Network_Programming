#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXLINE 8192 /* Max text line length */
#define LISTENQ 1024 /* Second argument to listen() */

/* Destructively modify string to be upper case */
void upper_case(char *s) {
  while (*s) {
    *s = toupper(*s);
    s++;
  }
}

void echo(int connfd) {
  // Local variable declarations:
  size_t n;
  char buf[MAXLINE];

  // Keep reading lines until client closes connection:
  while ((n = recv(connfd, buf, MAXLINE, 0)) != 0) {
    printf("Echo Server received %u (%s) incoming bytes.\n", n, buf);
    printf("Echo Server is making those bytes upper case.\n");
    upper_case(buf);
    printf("Echo Server is sending %u bytes back to client (%s).\n", n, buf);
    send(connfd, buf, n, 0);
  }
}

int open_listenfd(int port) {
  int listenfd;    // the listening file descriptor
  int optval = 1;  // options for setsockopt

  struct sockaddr_in serveraddr;

  // Create a socket descriptor.
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -1;

  // Eliminates "Address already in use" error from bind.
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval,
                 sizeof(int)) < 0)
    return -1;

  /* listenfd will be an endpoint for all requests to port
     on any IP address for this host */

  // The socket API says that you need to zero out the bytes first!
  bzero((char *)&serveraddr, sizeof(serveraddr));

  // Set the AF_INET protocol family.
  serveraddr.sin_family = AF_INET;

  // Allow incoming connections from any IP address.
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

  // Indicate the port number to "listen" on.
  serveraddr.sin_port = htons((unsigned short)port);

  // Bind the server information (options and port number) to the
  // listening socket file descriptor.
  if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    return -1;

  // Make it a listening socket ready to accept connection requests.
  if (listen(listenfd, LISTENQ) < 0) return -1;

  // Return the prepared listen socket file descriptor.
  return listenfd;
}

int main(int argc, char **argv) {
  int listenfd;  // listening file descriptor
  int connfd;    // the connection file descriptor
  int port;      // the port number

  // clientaddr records the address information of the client program
  // that connects to this server.
  struct sockaddr_in clientaddr;

  // clientlen is used to record the length of the client adress. This
  // is necessary for calling the accept() function.
  socklen_t clientlen;

  // hp is used to record the client host information through DNS. We
  // will use this to lookup the host information using
  // gethostbyaddr().
  struct hostent *hp;

  // haddrp is used to remember the domain name of the host.
  char *haddrp;

  // client_port is the ephemeral port used by the client.
  unsigned short client_port;

  // First, check the command line arguments.
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(0);
  }

  // Next, convert the port number as a string to an integer.
  port = atoi(argv[1]);

  // Now, create a socket file descriptor to listen for incoming
  // connections.
  listenfd = open_listenfd(port);

  printf("Echo Server is listening on port %d.\n", port);

  // As is the case for all servers - run forever! It is only these
  // programs that you want to go into an infinte loop!
  while (1) {
    // Record the size of the clienaddr (struct sockaddr_in) structure.
    clientlen = sizeof(clientaddr);

    printf("Echo Server is accepting incoming connections on port %d.\n", port);

    // Block on accepting incoming connections. When we have an
    // incoming connection accept() will fill in the given sockaddr.
    //
    // So, why do we cast a sockaddr_in to a sockaddr:
    //
    // struct sockaddr {
    //   unsigned short    sa_family;    // address family, AF_xxx
    //   char              sa_data[14];  // 14 bytes of protocol address
    // };
    connfd = accept(listenfd, (struct sockaddr *)(&clientaddr), &clientlen);

    // determine the domain name and IP address of the client
    hp = gethostbyaddr((const char *)(&clientaddr.sin_addr.s_addr),
                       sizeof(clientaddr.sin_addr.s_addr), AF_INET);

    // Need to convert the network byte order IP address to dotted IP string.
    haddrp = inet_ntoa(clientaddr.sin_addr);

    // Convert the port number from network byte order to host byte order.
    client_port = ntohs(clientaddr.sin_port);

    // Print an information message.
    printf(
        "Echo Server received a connection to %s (%s).\n"
        "Echo Server is using port %u and client has an ephemeral port of "
        "%u.\n",
        hp->h_name, haddrp, port, client_port);

    // Service the connection.
    echo(connfd);

    printf("Echo Server is closing the connection on %s (%s).\n", hp->h_name,
           haddrp);

    // Close the connection.
    close(connfd);
  }

  exit(0);
}
