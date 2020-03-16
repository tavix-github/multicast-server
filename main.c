
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <getopt.h>

static int mode = 0; // 0: message mode, 1: loopback mode
static char multicast_ip[32] = "239.0.0.10"; // 239.0.0.0~239.255.255.255
static char interface_ip[32] = "0.0.0.0"; // interface ip of to joined multicast group
static short localport = 9812; // local port to listened
static size_t max_frame = 2048;

static const char* short_options = "bm:i:p:l:h"; // Short options
static const struct option long_options[] = { // Long options
    {"loopback", 	no_argument, 	    NULL, 'b'},
    {"multicast", 	required_argument, 	NULL, 'm'},
    {"interface", 	required_argument, 	NULL, 'i'},
    {"port", 	    required_argument,	NULL, 'p'},
    {"maxlen", 	    required_argument, 	NULL, 'l'},
    {"help", 	    no_argument, 		NULL, 'h'},
	{ 0, 		    0, 					0, 		0}
};

static const char *help = "\nUsage: multicast-server [option] [argument]\n\n"
		"\t-b, --loopback\tloopback mode\n"
        "\t-m, --multicast\tmulticast ip, 239.0.0.0~239.255.255.255, default: 239.0.0.10\n"
        "\t-i, --interface\tinterface ip to joined multicast group, default: 0.0.0.0\n"
		"\t-p, --port\tlocal udp port, default: 9812\n"
		"\t-l, --maxlen\tmax udp frame length, default: 2048\n"
		//"\t-r, --rsize\tsocket receive buff size, default: 128KB\n"
		"\t-h, --help\tdisplay this help and exit\n";

int main(int argc, char *argv[])
{

    int retval = 0;

    /* Command line parameter analysis */
	/* ***********************************************************
	*		***	    Command line parameter analysis       ***	 * 
	* ***********************************************************/
	int c;
	while((c = getopt_long (argc, argv, short_options, long_options, NULL)) != -1)
	{
		switch (c)
		{
			case 'b': // --loopback
				mode = 1; // loopback mode
				break;
			case 'm': // --multicast
				strncpy(multicast_ip, optarg, strlen(optarg) +1);
				break;
            case 'i': // --interface
				strncpy(interface_ip, optarg, strlen(optarg) +1);
				break;
			case 'p': // --port
				localport = strtoul(optarg, NULL, 0);
				break;
			case 'l': // --maxlen
				max_frame = strtoul(optarg, NULL, 0);
				break;
			case 'h': // --help
				printf("%s\n", help); // Display help
				return 0;
			case '?':
				return 0;
		};
	};

    /* crate socket for udp */
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("socket failed");
        return EXIT_FAILURE;
    }

    /* Join multicast group */
    struct ip_mreq multicast;
    multicast.imr_multiaddr.s_addr = inet_addr(multicast_ip);
    multicast.imr_interface.s_addr = inet_addr(interface_ip);
    retval = setsockopt(sock_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &multicast, sizeof(struct ip_mreq));
    if (retval != 0) {
        perror("failed to join multicast group");
        close(sock_fd);
        return EXIT_FAILURE;
    }

    /* bind local interface and port */
    struct sockaddr_in local_addr ; /* local ip addr and port to listened */
    local_addr.sin_family = AF_INET ; /* IPv4 */
    local_addr.sin_port = htons(localport) ; /* local port */
    local_addr.sin_addr.s_addr = INADDR_ANY ; /* local computer ip */
    retval = bind(sock_fd,(struct sockaddr *)&local_addr,sizeof(local_addr));
    if (retval != 0) {
        perror("failed to bind local inteface");
        close(sock_fd);
        return EXIT_FAILURE;
    }

    /* request buffer */
    char *buf = malloc(max_frame);
    if (buf == NULL) {
        perror("failed to request buffer");
        close(sock_fd);
        return EXIT_FAILURE;
    }

    ssize_t rbytes = 0, sbytes = 0;
    static struct sockaddr_in from; /* remote ip addr and port */
    socklen_t fromlen = sizeof(from);
    while (1) {
        rbytes = recvfrom(sock_fd, buf, max_frame, 0, (struct sockaddr *)&from, &fromlen);
        if (rbytes > 0) {
            if (mode == 0) {
                buf[rbytes] = '\0';
                printf("receive message: %s\n", buf);
            }
            else if (mode == 1) {
                sbytes = sendto(sock_fd, buf, rbytes, 0, (struct sockaddr *)&from, fromlen);
                if (sbytes != rbytes) {
				    printf("Data loss occurs!\n");
			    }
            }
        } 
    }

    free(buf);
    close(sock_fd);

    return 0;
}