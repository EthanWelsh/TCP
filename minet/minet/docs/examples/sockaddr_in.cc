struct sockaddr_in {
	short            sin_family;   // Address family (AF_INET, AF_INET6, etc.)
	unsigned short   sin_port;     // Port number. For example: htons(3490)
	struct in_addr   sin_addr;     // See below.
};
