const char *USAGE = "ipk24-chat -t PROTOCOL -s HOSTNAME [-p PORT] [-d TIMEOUT] [-r MAX_UDP_RETR] [-h]\n"
                    "\t-t PROTOCOL\tTransport protocol used for connection (tcp/udp)\n"
                    "\t-s HOSTNAME\tServer IP or hostname\n"
                    "\t-p PORT\tServer port\n"
                    "\t-d TIMEOUT\tUDP confirmation timeout\n"
                    "\t-r MAX_UDP_RETR\tMaximum number of UDP retransmissions\n"
                    "\t-h\tDisplay this message\n";

enum Protocol
{
  NONE,
  UDP,
  TCP
};
