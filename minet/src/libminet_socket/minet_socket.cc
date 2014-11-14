#include <fcntl.h>

#include "Minet.h"
#include "minet_socket.h"

#include <string>

#define UNINIT_SOCKS -1
#define KERNEL_SOCKS 1
#define MINET_SOCKS 2


#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif


using std::cerr;
using std::endl;
using std::string;

int socket_type = 0;
MinetHandle sock;

/**
 * @brief 
 *      Contains the error code of the most recent error to occur *
 *      inside a Minet socket library function call.
 *
 *	Some functions inside Minet notify the caller of an error
 *	condition * by storing an error code inside this variable.
 *
 *	Note that this variable will only contain a meaningful value
 *	if the return value of the most recent Minet function call
 *	indicated that * an error occurred, \b and the documentation
 *	for that Minet function stated that you could call
 *	minet_error() to get more information about the error.  Not
 *	all Minet functions use #minet_errno for reporting error
 *	codes; furthermore, calling additional Minet functions may
 *	change the value of #minet_errno.
 *
 * @note Within Minet, an error code of \c ENODEV ("no device error")
 * is often used to signify that Minet was not initialized
 * properly. Check that the call to minet_init() succeeded.
 *
 * @note You can set #MINET_DEBUGLEVEL in \c setup.sh to 1 or greater
 * to see more information about errors that occur while Minet is
 * operating.
 *
 * @note This variable is similar in principle to the global variable
 * \c errno in Linux.  See the manual page entry for \c errno(3)
 * ((<tt>man 3 errno</tt>)).
 *
 * @see minet_error(), minet_perror(), #MINET_DEBUGLEVEL
 *
 */
int minet_errno = EOK;


/**
 * @brief Initialize the Minet socket library.
 *
 * This function must be called before any of the Minet socket library
 * functions in minet_socket.h could be used.
 *
 * @param[in] type Specifies whether to use the kernel networking
 * stack or the Minet user stack.  
 *     - MINET_KERNEL: Use the kernel * networking stack. (This * will
 *                     bypass all Minet modules lower than \c * *
 *                     libminet_socket).
 *     - MINET_USER: Use the Minet user stack.
 *
 * @return 
 *      If the Minet socket library initialized successfully, this *
 *      function returns the networking stack being used (either *
 *      MINET_KERNEL or MINET_USER).
 *      \par
 *      If the library did not initialize * successfully, a negative
 *      value * is returned.  * To get more information about the
 *      error, you * can * use the minet_error() or minet_perror() *
 *      functions, or set * #MINET_DEBUGLEVEL to * something greater
 *      than 0 in setup.sh to see * * debugging messages while Minet
 *      is running.
 *
 * @remarks 
 *      The Minet socket library requires that the sock_module * is
 *      loaded and operational.
 *	\par
 *	The ability to easily switch between the kernel networking
 *	stack and the Minet user stack is useful when you are writing
 *	a module for Minet.  If your module is not functioning
 *	correctly when initialized using the Minet user stack
 *	(MINET_USER), try switching to the kernel stack
 *	(MINET_KERNEL). This will bypass the Minet stack entirely and
 *	will allow you to discover whether the problem occurs inside
 *	your module (which will load as part of the Minet user stack
 *	if you called this function with MINET_USER), or elsewhere.
 *	\par
 *	You can set #MINET_DEBUGLEVEL to 1 in \c setup.sh or greater
 *	to see more information about errors that occur while Minet is
 *	operating.
 *
 * @todo The meaning of the return value is a little confusing. Even
 * describing it was a little awkward. How about returning EOK or 0? 
 * (There appears to be a sort of convention in Minet that a return
 * value >= 0 signifies success, while a negative return value
 * signifies an error. Can we formalize this?)
 *
 * @todo It's a little confusing that there's a MinetInit() in Minet.h
 * and a minet_init() here. Consider renaming this function to
 * something a little more descriptive (init_minet_socket_lib()
 * perhaps, or something along those lines?).
 */
EXTERNC int minet_init(minet_socket_types type) {

    if (type == MINET_KERNEL) {
        socket_type = KERNEL_SOCKS;

    } else if (type == MINET_USER) {

        socket_type = MINET_SOCKS;
        MinetInit(MINET_SOCKLIB_MODULE);

        sock = MinetIsModuleInConfig(MINET_SOCK_MODULE) ? MinetConnect(MINET_SOCK_MODULE) : MINET_NOHANDLE;

        if ( (sock == MINET_NOHANDLE) && 
	     (MinetIsModuleInConfig(MINET_SOCK_MODULE))) {

            MinetSendToMonitor(MinetMonitoringEvent("Can't connect to sock_module"));

            DEBUGPRINTF(1, string("socklib in minet_init(): Can't connect to sock_module. Check that the sock_module loaded successfully.\n").c_str());

            socket_type = UNINIT_SOCKS;

            return socket_type;
        }

        MinetSendToMonitor(MinetMonitoringEvent("socklib up"));

    } else {
    	DEBUGPRINTF(1, string("socklib in minet_init(): Invalid argument. Must be MINET_KERNEL or MINET_USER.\n").c_str());
    	socket_type = UNINIT_SOCKS;
    }

    return (socket_type);
}

/**
 * @brief This function should be called by the application before it
 * exits.
 *
 * If minet_init() was called with MINET_USER, this will shut down the
 * Minet stack and perform cleanup.
 *
 * This function currently does nothing if minet_init() was called
 * with MINET_KERNEL.
 *
 * @todo Return value is currently -1 (because that's what
 * UNINIT_SOCKS is defined to be), which signifies error. Perhaps it
 * should return 0 or EOK.  Then we can document the return value.
 *
 * @todo Perhaps this function should call shutdown() on the socket in
 * the case of MINET_KERNEL.  Doesn't look like we ever do this
 * (although it's not a big deal, I suppose).
 *
 * (We would need the sockfd param as well, if we do this).
 * 
 * (For maximum parallelism with Berkeley sockets, we might want to
 * name this function minet_shutdown(), but on the other hand, that
 * would be rather confusing as one would * think that this would shut
 * down all of Minet, but it just closes the socket!).
 */
EXTERNC int minet_deinit() {
    if (socket_type == MINET_SOCKS) {
        MinetDeinit();
    }

    socket_type = UNINIT_SOCKS;

    return (socket_type);
}

/**
 * @brief Returns the most recent error that occurred in a Minet
 * function call.
 *
 * This function will correctly obtain the error code of the most
 * recent error based on whether Minet was initialized using the
 * kernel stack or the Minet user stack (see minet_init()).
 *
 * You may find that it is more useful to call minet_perror() instead
 * which will print a description of the error based on the error
 * code.
 *
 * @return 
 *      If minet_init() was called with MINET_KERNEL, this function *
 *      returns the error code of the most recent error that occurred
 *      in * the system.  
 *      \par
 *	If minet_init() was called with MINET_USER, this function
 *	returns the error code of the most recent error that occurred
 *	in Minet.
 *
 * @remarks 
 *      Many system-level calls as well as functions inside Minet
 *      notify the caller of an error condition * by storing an error
 *      code inside a global variable.  In Minet, this variable is
 *      called #minet_errno.  In many operating * systems, this
 *      variable is called \c errno.
 *      \par
 *	If Minet was initialized using the kernel stack, the Minet
 *	socket library will use system calls for socket programming;
 *	as such, if any errors occur, \c errno would be the correct
 *	variable to use for error reporting.  If, on the other hand,
 *	Minet was initialized using the Minet user stack, #minet_errno
 *	should be used.  This function will return the correct error
 *	code depending on how Minet was initialized when minet_init()
 *	was called.
 *	\par
 * 	This function will only return a meaningful result if the
 * 	return value of the most recent Minet function call indicated
 * 	that an error occurred, \b and the documentation for that
 * 	Minet function stated that you could call minet_error() to get
 * 	more information about the error.  Not all Minet functions use
 * 	#minet_errno for reporting error codes, so minet_error() will
 * 	not always give you meaningful information; furthermore,
 * 	calling additional Minet functions may change the value of
 * 	#minet_errno, so you should always call minet_error()
 * 	immediately after the function that failed.
 * 	\par
 * 	Within Minet, an error code of \c ENODEV ("no device error")
 * 	is often used to signify that Minet was not initialized
 * 	properly. Check that the call to minet_init() succeeded.  \par
 * 	You can set #MINET_DEBUGLEVEL in \c setup.sh to 1 or greater
 * 	to see more information about errors that occur while Minet is
 * 	operating.
 *
 * @note For more information about \c errno in Linux, see the manual
 * page entry for \c errno(3) (<tt>man 3 errno</tt>).
 *
 * @see #minet_errno, minet_perror()
 *
 * @todo The value of minet_errno is useless to the user unless we
 * provide him/her with the corresponding values (or at least define
 * all the error codes in one easy-to-find place (such as "error.h",
 * perhaps?)). Otherwise, the user just might as well call
 * minet_perror() instead.
 *
 * @todo This function does some strange things in the case of
 * UNINIT_SOCKS... it sets errno instead of minet_errno, and returns
 * -1. Maybe we should make that case equivalent to the default: case.
 * UNINIT_SOCKS will only happen if minet_init() was called with an
 * invalid argument, which would mean that Minet was not initialized
 * correctly, which is (in my understanding) what minet_errno=ENODEV
 * is supposed to represent (at least that's how I've been documenting
 * it all along...). It would make more sense to set minet_errno to
 * some representation of "invalid argument" if minet_init() is called
 * with something other than MINET_USER or MINET_KERNEL. But then the
 * UNINIT_SOCKS case ought to be equivalent to MINET_USER
 * case... otherwise, this function would wipe out the "invalid
 * argument" error code with the ENODEV error code. I'm tempted to
 * make a third "errno"-type global var just for UNINIT_SOCKS, but
 * that would uglify things even further. BLARGH!
 */
EXTERNC int minet_error() {
    switch (socket_type) {
	case UNINIT_SOCKS:
	    errno = ENODEV;            // "No such device" error
	    return -1;
	    break;
	case KERNEL_SOCKS:
	    return errno;
	    break;
	case MINET_SOCKS:
	    return minet_errno;
	    break;
	default:
	    minet_errno = ENODEV;
	    return minet_errno;
    }
}


/**
 * @brief Prints a description for the most recent error that occurred
 * in a Minet function call.
 *
 * This function will translate the error code given by minet_error()
 * into a meaningful message and print it out to the standard error
 * stream.
 *
 * @param[in] s	
 * 	If not NULL, this function will print \c s, followed by a
 * 	colon and a space, * and then the error description to the
 * 	standard error stream.
 *	\par
 *	If NULL, this function will only print the error description
 *	to the standard error stream.
 *	\par
 *	You can optionally use this parameter to prepend the error
 *	description with a message of your own (it may be useful to
 *	print the name of the function where the error occurred, for
 *	example).
 *
 * @remarks See the documentation for minet_error() for more
 * information on error codes.
 *
 * @note This function behaves similarly to the \c perror routine in
 * Linux.  See the manual page entry for \c perror(3).
 *
 * @see #minet_errno, minet_error()
 *
 * @todo This should call minet_error(). Currently, it duplicates its
 * logic. If you change minet_error(), you might be confused (or at
 * least * slightly disappointed) to find that minet_perror() still
 * behaves the same.
 *
 * @todo The current remarks section is kind of a cop-out.
 *
 * @bug should only print ": " if s is not NULL.
 */
EXTERNC int minet_perror(const char * s) {
    switch (socket_type) {
	case UNINIT_SOCKS:
	    errno = ENODEV;            // "No such device" error
	    return (-1);
	    break;
	case KERNEL_SOCKS:
	    perror(s);
	    return (0);
	    break;
	case MINET_SOCKS:
	    cerr << s;
	    switch (minet_errno) {
		case EOK:
		    cerr << ": Ok" << endl;
		    break;
		case ENOMATCH:
		    cerr << ": No match found" << endl;
		    break;
		case EBUF_SPACE:
		    cerr << ": Buffer space error" << endl;
		    break;
		case EUNKNOWN:
		    cerr << ": Unknown error" << endl;
		    break;
		case ERESOURCE_UNAVAIL:
		    cerr << ": Resource unavailable" << endl;
		    break;
		case EINVALID_OP:
		    cerr << ": Invalid operation" << endl;
		    break;
		case ENOT_IMPLEMENTED:
		    cerr << ": Function not implemented" << endl;
		    break;
		case ENOT_SUPPORTED:
		    cerr << ": Not supported" << endl;
		    break;
		case EWOULD_BLOCK:
		    cerr << ": Would block" << endl;
		    break;
		case ECONN_FAILED:
		    cerr << ": Connection failed" << endl;
		    break;
		default:
		    cerr << ": Unknown error" << endl;
	    }
	    return (0);
	    break;
	default:
	    cerr << s << ": UNKNOWN SOCKET TYPE" << endl;
	    minet_errno = ENODEV;
	    break;
    }

    return -1;
}

/**
 * @brief Initialize an unbound socket and return its file descriptor.
 *
 * This function behaves similarly to the socket() function from
 * Berkeley Sockets API (substituting \c AF_INET for the \c domain
 * argument and \c 0 for protocol).  See the Linux manual page for \c
 * socket(2) for additional help (<tt>man 2 socket</tt>).
 *
 * minet_socket() allows you to switch easily between using the Minet
 * user stack and the kernel network stack.  If minet_init() was
 * called with MINET_KERNEL, then the kernel networking stack will be
 * used, and this function will behave identically to the socket()
 * function in the Berkeley Sockets API. If minet_init() was called
 * with MINET_USER, then the Minet user stack will be used.
 *
 * @param[in] type The type of socket to create. The following values
 * are supported:
 *          - SOCK_STREAM for TCP sockets
 *          - SOCK_DGRAM for UDP sockets
 *
 * @return  
 *     Handle to the socket if the call was successful.
 *     \par
 *     A negative return value signifies an error.  To get more
 *     information about the error, you can use the minet_error() * or
 *     minet_perror() functions.
 *     \par
 *	The following table describes the meaning of the possible *
 *	error codes that minet_error() could return if this function
 *	failed:
 *              - ENODEV (Minet user stack only) \n
 *                  Minet was not initialized properly. Check that the
 *                  call to minet_init() was successful, and that
 *                  minet_deinit() was not called at any time
 *                  afterward.
 *              - EAFNOSUPPORT \n
 *                  The implementation does not support the specified
 *                  address family.
 *              - EMFILE \n
 *                  No more file descriptors are available for this
 *                  process.
 *              - ENFILE \n
 *                  No more file descriptors are available for the
 *                  system.
 *              - EPROTONOSUPPORT \n
 *                  The protocol is not supported by the address
 *                  family, or the protocol is not supported by the
 *                  implementation.
 *              - EPROTOTYPE \n
 *                  The socket type is not supported by the protocol.
 *              - EACCES \n
 *                  The process does not have appropriate privileges.
 *              - ENOBUFS \n
 *                  Insufficient resources were available in the
 *                  system to perform the operation.
 *              - ENOMEM \n
 *                  Insufficient memory was available to fulfill the
 *                  request.
 *	\par 
 *      Additionally, you can set * #MINET_DEBUGLEVEL in \c setup.sh
 *      to 1 or * greater to see more information about errors * that
 *      occur while Minet is operating.
 *
 * \todo SOCK_ICMP for ICMP sockets is not yet supported for MINET_USER.
 * \note SOCK_STREAM and SOCK_DGRAM are defined inside the <sys/socket.h> header.
 */
EXTERNC int minet_socket(int type) {
    switch (socket_type) {
	case UNINIT_SOCKS:
	    errno = ENODEV;            // "No such device" error
	    return (-1);
	    break;
	case KERNEL_SOCKS:
	    if ((type == SOCK_STREAM) || (type == SOCK_DGRAM)) {
		return (socket (AF_INET, type, 0));
	    }
	    break;
	case MINET_SOCKS: {
	    SockLibRequestResponse slrr;
	    switch (type) {
		case SOCK_STREAM:
		    slrr = SockLibRequestResponse(mSOCKET,
						  Connection(IP_ADDRESS_ANY,
							     IP_ADDRESS_ANY,
							     TCP_PORT_NONE,
							     TCP_PORT_NONE,
							     IP_PROTO_TCP),
						  UNSPECIFIED_SOCK,
						  Buffer(),
						  0, 0);
		    break;
		case SOCK_DGRAM:
		    slrr = SockLibRequestResponse(mSOCKET,
						  Connection(IP_ADDRESS_ANY,
							     IP_ADDRESS_ANY,
							     UDP_PORT_NONE,
							     UDP_PORT_NONE,
							     IP_PROTO_UDP),
						  UNSPECIFIED_SOCK,
						  Buffer(),
						  0, 0);
		    break;
	    }

	    MinetSend(sock, slrr);
	    MinetReceive(sock, slrr);
	    minet_errno = slrr.error;

	    if (minet_errno != EOK) {
		return (-1);
	    }

	    return (slrr.sockfd);
	    break;
	}
	default:
	    minet_errno = ENODEV;
	    break;
    }

    return -1;
}


/**
 * @brief Assigns a socket a local address.
 *
 * This function behaves similarly to the bind() function from
 * Berkeley Sockets API (substituting \c sizeof(*myaddr) for the \c
 * len argument).  See the Linux manual page for \c bind(2) for
 * additional help (<tt>man 2 bind</tt>).
 *
 * minet_bind() allows you to switch easily between using the Minet
 * user stack and the kernel network stack.  If minet_init() was
 * called with MINET_KERNEL, then the kernel networking stack will be
 * used, and this function will behave identically to the bind()
 * function in the Berkeley Sockets API. If minet_init() was called
 * with MINET_USER, then the Minet user stack will be used.
 *
 * @param[in] sockfd Handle to an unbound socket that was created
 * using minet_socket().
 *
 * @param[in] myaddr Pointer to a sockaddr_in structure which may
 * contain (depending on the address family) the desired local IP
 * address, a port * number for the socket, among other possible
 * fields that will be assigned to the socket.  See the remarks
 * section for more information.
 *
 * @return  If the call was successful, this function returns 0.
 *          \par

 *          If the call was not successful, this function returns -1.
 *          To get more information about the error, you can use the
 *          minet_error() * or minet_perror() functions.  See the
 *          Linux manual page for \c bind(2) for a list of possible
 *          error codes.  Additionally, you can set *
 *          #MINET_DEBUGLEVEL in \c setup.sh to 1 or greater to see
 *          more information about errors that occur while Minet is
 *          operating.
 *	    \par
 *          If minet_init() was called with MINET_USER, an error code
 *          of \c ENODEV is used to signify that Minet * did not
 *          initialize successfully.  Check that the call to
 *          minet_init() was successful.
 *
 * @par Blocking?
 *		No
 *
 * @remarks
 *	For servers, the bind() function is typically used to assign
 *	the port number on which the server process will be listening.
 *	Clients generally do not need to call bind().

 *	\par

 *	In Linux, the \c sockaddr_in structure is defined in the
 *	<netinet/in.h> header.  It typically looks like this: \include
 *	sockaddr_in.cc
 *	\par
 *	And the \c in_addr structure typically looks like this:
 *	\include in_addr.cc
 *	\par
 *	Servers generally use \c INADDR_ANY for the \c
 *	sockaddr_in.sin_addr field.
 *	\par
 *	Note that Minet currently supports only the IPv4 address
 *	family.  Therefore, you should use \c AF_INET for \c
 *	sockaddr_in.sin_family.
 *	\par
 *	Note that byte ordering is a concern in networking code, so
 *	you should use the htons() function for storing the port
 *	number in Network Byte Order for the \c sockaddr_in.sin_port
 *	field.  See <a
 *	href="http://beej.us/guide/bgnet/output/html/multipage/htonsman.html">Section
 *	9.12</a> in <a
 *	href="http://beej.us/guide/bgnet/output/html/multipage/index.html">Beej's
 *	Guide to Network Programming</a> for more information.
 *
 * @todo How much additional work would it take to make Minet support
 * IPv6 addresses?
 */
EXTERNC int minet_bind(int sockfd, struct sockaddr_in * myaddr) {
    switch (socket_type) {
	case UNINIT_SOCKS:
	    errno = ENODEV;            // "No such device" error
	    return (-1);
	    break;
	case KERNEL_SOCKS:
	    return bind(sockfd, (sockaddr *)myaddr, sizeof(*myaddr));
	    break;
	case MINET_SOCKS: {
	    SockLibRequestResponse slrr(mBIND,
					Connection(IPAddress(ntohl((unsigned)myaddr->sin_addr.s_addr)),
						   IP_ADDRESS_ANY,
						   (unsigned short)ntohs(myaddr->sin_port),
						   PORT_NONE,
						   0),
					sockfd,
					Buffer(),
					0, 0);
	    //    cerr << slrr << endl;
	    MinetSend(sock, slrr);
	    MinetReceive(sock, slrr);
	    minet_errno = slrr.error;

	    if (minet_errno != EOK) {
		return -1;
	    }

	    return 0;
	    break;
	}
	default:
	    minet_errno = ENODEV;
	    break;
    }

    return -1;
}


/**
 * @brief Prepares a socket for receiving incoming connections.
 *
 * This function behaves similarly to the listen() function from
 * Berkeley Sockets API.  See the Linux manual page for \c listen(2)
 * for additional help (<tt>man 2 listen</tt>).
 *
 * minet_listen() allows you to switch easily between using the Minet
 * user stack and the kernel network stack.  If minet_init() was
 * called with MINET_KERNEL, then the kernel networking stack will be
 * used, and this function will behave identically to the listen()
 * function in the Berkeley Sockets API. If minet_init() was called
 * with MINET_USER, then the Minet user stack will be used.
 *
 * @param[in] sockfd - Handle to a \link minet_bind() bound\endlink
 *                     socket.
 * @param[in] backlog - An integer representing the number of pending
 *                      connections that can be queued up at any one
 *                      time. See the Linux manual page * for \c
 *                      listen(2) for more information.
 *
 * @par Blocking?
 *		No
 *
 * @return  
 *      If the call was successful, this function returns 0.
 *      \par
 *      If the call was not successful, this function returns -1.  To
 *      get more information about the error, you can use the
 *      minet_error() * or minet_perror() functions.  See the Linux
 *      manual page for \c listen(2) for a list of possible error
 *      codes.  Additionally, you can set * #MINET_DEBUGLEVEL in \c
 *      setup.sh to 1 or greater to see more information about errors
 *      that occur while Minet is operating.
 *	\par
 *	If minet_init() was called with MINET_USER, an error code of
 *	\c ENODEV is used to signify that Minet did not initialize
 *	successfully.  Check that the call to minet_init() was
 *	successful.
 */
EXTERNC int minet_listen(int sockfd, int backlog) {
    switch (socket_type) {
	case UNINIT_SOCKS:
	    errno = ENODEV;            // "No such device" error
	    return (-1);
	    break;
	case KERNEL_SOCKS:
	    return listen(sockfd, backlog);
	    break;
	case MINET_SOCKS: {
	    SockLibRequestResponse slrr(mLISTEN, Connection(), sockfd, 
					Buffer(), 0, 0);
	    
	    MinetSend(sock, slrr);
	    MinetReceive(sock, slrr);
	    minet_errno = slrr.error;
	    
	    if (minet_errno != EOK) {
		return -1;
	    }
	    
	    return 0;
	    break;
	}
	default:
	    minet_errno = ENODEV;
	    break;
    }

    return -1;
}


/**
 * @brief Allows an incoming connection to connect to the socket.
 *
 * This function behaves similarly to the accept() function from
 * Berkeley Sockets API.  See the Linux manual page for \c accept(2)
 * for additional help (<tt>man 2 accept</tt>).
 *
 * minet_accept() allows you to switch easily between using the Minet
 * user stack and the kernel network stack.  If minet_init() was
 * called with MINET_KERNEL, then the kernel networking stack will be
 * used, and this function will behave identically to the accept()
 * function in the Berkeley Sockets API. If minet_init() was called
 * with MINET_USER, then the Minet user stack will be used.
 *
 * @param[in] sockfd
 *      Handle to a socket that is currently in the \link
 *      minet_listen() listening\endlink state.  Note that upon
 *      return, this socket * goes back to the listening state and
 *      will be available to receive additional connection requests.
 *      The actual connection with the * client is made using the
 *      socket handle that is returned by this function.
 * @param[out] addr  
 *      Pointer to a sockaddr structure to receive the client's
 *      address information, or \c NULL if this information is not
 *      required.  See the remarks section for more information.
 *
 * @return  
 *      If the call was successful, this function returns a new
 *      socket descriptor on which the actual connection to the
 *      client is made.  (The parameter * sockfd is returned to the
 *      listening state and is not associated with the connection.)
 *      \par
 *      If the call was not successful, this function returns -1.  To
 *      get more information about the error, you can use the
 *      minet_error() * or minet_perror() functions.  See the Linux
 *      manual page for \c accept(2) for a list of possible error
 *      codes.  Additionally, you can set * #MINET_DEBUGLEVEL in \c
 *      setup.sh to 1 or greater to see more information about errors
 *      that occur while Minet is operating.
 *      \par
 *      If minet_init() was called with MINET_USER, an error code of
 *	\c ENODEV is used to signify that Minet did not initialize
 *	successfully.  Check that the call to minet_init() was
 *	successful.
 *
 * @par Blocking?
 *		Yes, unless the socket is in non-blocking mode.  See
 *		minet_set_blocking() and minet_set_nonblocking().
 *
 * @remarks
 * 	For servers with connection-based socket types (e.g., TCP),
 * 	the accept() function is used to accept incoming connection
 * 	requests before the server can * exchange data with the
 * 	client.  In Minet, data can be exchanged using minet_read()
 * 	and minet_write().
 *	\par
 *	Connectionless socket types (e.g., UDP) do not need to call
 *	accept().  They can begin exchanging data
 *	\par
 *	Note that this function is blocking - if there is no
 *	connection request present when accept() is called, this
 *	function will block until one arrives.  You can use
 *	minet_select() and minet_poll() to avoid unnecessary blocking
 *	behavior.
 *	\par
 *	In Linux, the \c sockaddr_in structure is defined in the
 *	<netinet/in.h> header.  It typically looks like this: \include
 *	sockaddr_in.cc
 *	\par
 *	And the \c in_addr structure typically looks like this:
 *	\include in_addr.cc
 *	\par
 *	Note that byte ordering is a concern in networking code, so
 *	you should use the ntohs() function if you intend on reading
 *	and storing the port number in \c sockaddr_in.sin_port
 *	locally.  See <a
 *	href="http://beej.us/guide/bgnet/output/html/multipage/htonsman.html">Section
 *	9.12</a> in <a
 *	href="http://beej.us/guide/bgnet/output/html/multipage/index.html">Beej's
 *	Guide to Network Programming</a> for more information.
 *	\par
 *	To convert the IP address to a string (e.g., for display), you
 *	can use the inet_ntop() function from <arpa/inet.h>.  See <a
 *	href="http://beej.us/guide/bgnet/output/html/multipage/inet_ntopman.html">Section
 *	9.14</a> in <a
 *	href="http://beej.us/guide/bgnet/output/html/multipage/index.html">Beej's
 *	Guide to Network Programming</a> for more information.
 *
 * @todo Does Minet *actually* support non-blocking sockets for
 * MINET_USER at the moment?  After searching around in the code, it
 * seems that mSET_NONBLOCKING * currently doesn't actually do
 * anything.  Maybe it doesn't need to?  (Test it!)
 *
 */
EXTERNC int minet_accept(int sockfd, struct sockaddr_in * addr) {
    socklen_t length = sizeof(*addr);

    switch (socket_type) {
	case UNINIT_SOCKS:
	    errno = ENODEV;            // "No such device" error
	    return -1;
	    break;
	case KERNEL_SOCKS:
	    return accept(sockfd, (sockaddr *)addr, &length);
	    break;
	case MINET_SOCKS: {
	    SockLibRequestResponse slrr(mACCEPT,
					Connection(IP_ADDRESS_ANY,
						   IPAddress((unsigned)addr->sin_addr.s_addr),
						   TCP_PORT_NONE,
						   (unsigned short)addr->sin_port,
						   0),
					sockfd,
					Buffer(),
					0, 0);
	    MinetSend(sock, slrr);
	    MinetReceive(sock, slrr);
	    minet_errno = slrr.error;
	    
	    if (minet_errno != EOK) {
		return -1;
	    }
	    
	    return (slrr.sockfd);
	    break;
	}
	default:
	    minet_errno = ENODEV;
	    break;
    }

    return -1;
}


/**
 * @brief Establishes a connection to a specified address.
 *
 * This function behaves similarly to the connect() function from
 * Berkeley Sockets API (substituting \c sizeof(*myaddr) for the \c
 * len argument). See the Linux manual page for \c connect(2) for
 * additional help (<tt>man 2 connect</tt>).
 *
 * minet_connect() allows you to switch easily between using the Minet
 * user stack and the kernel network stack.  If minet_init() was
 * called with MINET_KERNEL, then the kernel networking stack will be
 * used, and this function will behave identically to the connect()
 * function in the Berkeley Sockets API. If minet_init() was called
 * with MINET_USER, then the Minet user stack will be used.
 *
 * @param[in] sockfd   
 *      Handle to an unbound socket created using minet_socket().
 * @param[in] addr  
 *      Pointer to a sockaddr_in structure containing the destination
 *      address.  See the remarks section for more information.
 *
 * @return  
 *      If the call was successful, this function returns 0.
 *      \par
 *      If the call was not successful, this function returns -1.  To
 *      get more information about the error, you can use the
 *      minet_error() * or minet_perror() functions.  See the Linux
 *      manual page for \c connect(2) for a list of possible error
 *      codes.  Additionally, you can set * #MINET_DEBUGLEVEL in \c
 *      setup.sh to 1 or greater to see more information about errors
 *      that occur while Minet is operating.
 *	\par
 *	If minet_init() was called with MINET_USER, an error code of
 *	\c ENODEV is used to signify that Minet did not initialize
 *	successfully.  Check that the call to minet_init() was
 *	successful.
 *
 * @par Blocking?
 *	Yes, unless the socket is in non-blocking mode.  See
 *	minet_set_blocking() and minet_set_nonblocking().
 *
 * @remarks
 *	connect() is typically used by connection-oriented clients
 *	(e.g., TCP) to establish a connection with a server before
 *	they can exchange data.  In the case of Minet, data exchange
 *	is done using minet_read() and minet_write() for
 *	connection-oriented sockets, and minet_sendto() and
 *	minet_recvfrom() for connectionless sockets.
 *	\par
 *	You can use \c getaddrinfo() to convert a human-readable
 *	hostname or IP address into its binary representation for use
 *	with this function.  See the Linux manual page for \c
 *	getaddrinfo(3) (<tt>man 3 getaddrinfo</tt>), or <a
 *	href="http://beej.us/guide/bgnet/output/html/multipage/getaddrinfoman.html">Section
 *	9.5</a> in <a
 *	href="http://beej.us/guide/bgnet/output/html/multipage/index.html">Beej's
 *	Guide to Network Programming</a> for more information.
 *	\par
 *	In Linux, the \c sockaddr_in structure is defined in the
 *	<netinet/in.h> header.  It typically looks like this: \include
 *	sockaddr_in.cc
 *	\par
 *	And the \c in_addr structure typically looks like this:
 *	\include in_addr.cc
 *	\par
 *	Note that Minet currently supports only the IPv4 address
 *	family.  Therefore, you should use \c AF_INET for \c
 *	sockaddr_in.sin_family.
 *	\par
 *	Note that byte ordering is a concern in networking code, so
 *	you should use the htons() function for storing the port
 *	number in Network Byte Order for the \c sockaddr_in.sin_port
 *	field.  See <a
 *	href="http://beej.us/guide/bgnet/output/html/multipage/htonsman.html">Section
 *	9.12</a> in <a
 *	href="http://beej.us/guide/bgnet/output/html/multipage/index.html">Beej's
 *	Guide to Network Programming</a> for more information.

 *
 * @todo Should I be mentioning that in here (as well as the docs for
 * some other functions in this file) that, for MINET_USER, * this
 * will actually construct a SockLibRequestResponse message (of type
 * mCONNECT) and send it to the specified destination?  * And that the
 * remote peer will then respond with another (STATUS?) message? 
 * Certainly, when students are working on the http client/server
 * project, they wouldn't care, but * if someone is working on a lower
 * level module (or if you're a Minet developer), this might suddenly
 * be a concern and should * probably be mentioned.
 *
 * @todo Some additional documentation is needed here and elsewhere
 * (e.g., minet_accept()) for non-blocking sockets. Also, the
 * "Blocking?" sections in other places may need * to be adjusted in a
 * similar manner as in here ("unless the socket is in non-blocking
 * mode...").
 *
 * @todo Connectionless sockets sometimes use connect() to set the
 * default destination address for subsequent send() and recv()
 * calls. It doesn't look like Minet has corresponding * minet_send()
 * or minet_recv() functions, although it has minet_write() and
 * minet_read() - will this functionality (default destination
 * address) be accomplished using these?  * From looking at
 * udp_client.cc and udp_server.cc, it looks like yes, but test it
 * anyway (using sendto() vs write(), and recvfrom() vs read()).  * We
 * might want to write a minet_send() and minet_recv() anyways. Why? 
 * 1.) If we're going to port this to Windows at any time, we would
 * prefer to use send() and * recv() instead of write() and read()
 * since sockets aren't treated like files on Windows.  2.) just to
 * achieve maximum parallelism with Berkeley sockets, we should
 * probably implement * these minet_send() and minet_recv()
 * functions. (Although note that there may be some confusion with the
 * similarly-named MinetSend() and MinetReceive(), which can be
 * clarified if necessary.)
 */
EXTERNC int minet_connect(int sockfd, struct sockaddr_in * addr) {
    switch (socket_type) {
	case UNINIT_SOCKS:
	    errno = ENODEV;            // "No such device" error
	    return (-1);
	    break;
	case KERNEL_SOCKS:
	    return (connect (sockfd, (sockaddr *) addr, sizeof(*addr)));
	    break;
	case MINET_SOCKS: {
	    SockLibRequestResponse slrr(mCONNECT,
					Connection(IP_ADDRESS_ANY,
						   IPAddress(ntohl((unsigned)addr->sin_addr.s_addr)),
						   TCP_PORT_NONE,
						   ntohs((unsigned short)addr->sin_port),
						   0),
					sockfd,
					Buffer(),
					0, 0);
	    
	    debug(3) << "socklib: Connecting to: " << slrr.connection.dest << std::endl;
	    
	    MinetSend(sock, slrr);
	    MinetReceive(sock, slrr);
	    minet_errno = slrr.error;

	    if (minet_errno != EOK) {
		debug(1) << "socklib: minet_connect() failed." << std::endl;
		return -1;
	    }

	    return 0;
	    break;
	}
	default:
	    minet_errno = ENODEV;
	    break;
    }

    return -1;
}


/**
 * @brief Attempt to read data from a socket.
 *
 * This function behaves similarly to the read() function from the
 * POSIX API.  See the Linux manual page for \c read(2) for additional
 * help (<tt>man 2 read</tt>).
 *
 * minet_read() allows you to switch easily between using the Minet
 * user stack and the kernel network stack for reading from a socket.
 * If minet_init() was called with MINET_KERNEL, then the kernel
 * networking stack will be used, and this function will behave
 * identically to the Unix read() function from the POSIX API.  If
 * minet_init() was called with MINET_USER, then the Minet user stack
 * will be used, and the read operation will be performed by
 * interacting with the Minet socket module.
 *
 * @param[in] fd    
 *      Handle to a connected socket.
 * @param[out] buf 
 *      Pointer to a buffer that will receive the incoming data.
 * @param[in] len  
 *      Maximum number of bytes to read from the socket.
 *
 * @par Blocking?
 *	Yes, unless the socket is in non-blocking mode.  See
 *	minet_set_blocking() and minet_set_nonblocking().
 *
 * @return  
 *      If the call was successful, this function returns the number
 *      of bytes read from the socket. Zero indicates that the sender
 *      has closed * the connection (this is the regular way to check
 *      for an orderly shutdown on a socket).
 *
 *      \par
 *
 *      If the call was not successful, this function returns -1.  To
 *      get more information about the error, you can use the
 *      minet_error() * or minet_perror() functions.  See the Linux
 *      manual page for \c read(2) for a list of possible error codes.
 *      Additionally, you can set * #MINET_DEBUGLEVEL in \c setup.sh
 *      to 1 or greater to see more information about errors that
 *      occur while Minet is operating.
 *
 *	\par
 *
 *	If minet_init() was called with MINET_USER, an error code of
 *	\c ENODEV is used to signify that Minet did not initialize
 *	successfully.  Check that the call to minet_init() was
 *	successful.
 *
 * @remarks
 *	For connection-oriented sockets (such as TCP), the socket must
 *	be connected before any data can be received.  For clients,
 *	this typically means that minet_connect() must have been
 *	called beforehand.  For servers, this typically means that
 *	minet_accept() must have been called.
 *
 *	\par
 *
 *	For connectionless sockets (such as UDP), the socket must be
 *	\link minet_bind() bound\endlink.
 *
 *	\par
 *
 *	Note that this function is blocking - if there is no data when
 *	minet_read() is called, this function will block until data
 *	arrives.  You can use minet_select() and minet_poll() to avoid
 *	unnecessary blocking behavior.
 *
 *	\par
 *
 *	Note that not all of the data will necessarily be read in a
 *	single call.  It may be necessary to put the read function
 *	inside a loop.
 */
EXTERNC int minet_read(int fd, char * buf, int len) {
    switch (socket_type) {
	case UNINIT_SOCKS:
	    errno = ENODEV;            // "No such device" error
	    return -1;
	    break;
	case KERNEL_SOCKS:
	    return read(fd, buf, len);
	    break;
	case MINET_SOCKS: {
	    SockLibRequestResponse slrr(mREAD, Connection(), fd,
					Buffer(buf, len), 0, 0);
	    MinetSend(sock, slrr);
	    MinetReceive(sock, slrr);
	    minet_errno = slrr.error;

	    if (minet_errno != EOK) {
		return -1;
	    }

	    Buffer b;
	    b = slrr.data;
	    b.GetData(buf, b.GetSize(), 0);

	    return (b.GetSize());

	    break;
	}
	default:
	    minet_errno = ENODEV;
	    break;
    }

    return -1;
}


/**
 * @brief Attempt to write data to a socket.
 *
 * This function behaves similarly to the write() function from the
 * POSIX API.  See the Linux manual page for \c write(2) for
 * additional help (<tt>man 2 write</tt>).
 *
 * minet_write() allows you to switch easily between using the Minet
 * user stack and the kernel network stack for writing to a socket.
 * If minet_init() was called with MINET_KERNEL, then the kernel
 * networking stack will be used, and this function will behave
 * identically to the Unix write() function from the POSIX API.  If
 * minet_init() was called with MINET_USER, then the Minet user stack
 * will be used, and the write operation will be performed by
 * interacting with the Minet socket module.
 *
 * @param[in] fd    Handle to a connected socket.
 * @param[in] buf   Pointer to a buffer that containing the data to be transmitted.
 * @param[in] len   Maximum number of bytes to be written to the socket.
 *
 * @par Blocking?

 *		Yes, unless the socket is in non-blocking mode.  See
 *		minet_set_blocking() and minet_set_nonblocking().
 *
 * @return  
 *      If the call was successful, this function returns the actual
 *      number of bytes sent. Note that this could be less than the
 *      number of bytes requested in the \c len parameter.
 *
 *      \par
 *
 *      If the call was not successful, this function returns -1.  To
 *      get more information about the error, you can use the
 *      minet_error() * or minet_perror() functions.  See the Linux
 *      manual page for \c write(2) for a list of possible error
 *      codes.  Additionally, you can set * #MINET_DEBUGLEVEL in \c
 *      setup.sh to 1 or greater to see more information about errors
 *      that occur while Minet is operating.
 * 
 *	\par
 * 
 *	If minet_init() was called with MINET_USER, an error code of
 *	\c ENODEV is used to signify that Minet did not initialize
 *	successfully.  Check that the call to minet_init() was
 *	successful.
 *
 * @remarks
 *	For connection-oriented sockets (such as TCP), the socket must
 *	be connected before any data can be sent.  For clients, this
 *	typically means that minet_connect() must have been called
 *	beforehand.  For servers, this typically means that
 *	minet_accept() must have been called.
 *
 *	\par
 *
 *	For connectionless sockets (such as UDP), the socket must be
 *	\link minet_bind() bound\endlink.
 *
 *	\par
 *
 *	Note that this function is blocking - if the write operation
 *	cannot be completed immediately, this function will block
 *	until the operation has completed.  You can use minet_select()
 *	to determine when a socket is ready for writing.
 *
 *	\par
 *
 *	Note that not all of the data will necessarily be written in a
 *	single call.  It may be necessary to put the write function
 *	inside a loop.
 */
EXTERNC int minet_write(int fd, char * buf, int len)
{
    switch (socket_type) {
	case UNINIT_SOCKS:
	    errno = ENODEV;            // "No such device" error
	    return (-1);
	    break;
	case KERNEL_SOCKS:
	    return write(fd, buf, len);
	    break;
	case MINET_SOCKS: {
	    SockLibRequestResponse slrr(mWRITE, Connection(), fd,
					Buffer(buf, len), 0, 0);
	    MinetSend(sock, slrr);
	    MinetReceive(sock, slrr);
	    minet_errno = slrr.error;
	    
	    if (minet_errno != EOK) {
		return (-1);
	    }
	    
	    return (slrr.bytes);
	    break;
	}
	default:
	    minet_errno = ENODEV;
	    break;
    }

    return -1;
}


/**
 * @brief Receive data from a socket and store the source address.
 *
 * This function behaves similarly to the recvfrom() function from
 * <sys/socket.h>.  See the Linux manual page for \c recvfrom(2) for
 * additional help (<tt>man 2 recvfrom</tt>).
 *
 * minet_recvfrom() allows you to switch easily between using the
 * Minet user stack and the kernel network stack for reading from a
 * socket.  If minet_init() was called with MINET_KERNEL, then the
 * kernel networking stack will be used, and this function will behave
 * identically to the recvfrom() function from <sys/socket.h>.  If
 * minet_init() was called with MINET_USER, then the Minet user stack
 * will be used, and the read operation will be performed by
 * interacting with the Minet socket module.
 *
 * @param[in] fd    Handle to a socket (which may or may not be connected).
 * @param[out] buf  Pointer to a buffer that will receive the incoming data.
 * @param[in] len   Maximum number of bytes to read from the socket.
 * @param[out] from 
 *      Pointer to a sockaddr structure to receive the source address
 *      information, or \c NULL if this information is not required.
 *      See the remarks section for more information.
 *
 * @par Blocking?
 *	Yes, unless the socket is in non-blocking mode.  See *
 *	minet_set_blocking() and minet_set_nonblocking().
 *
 * @return  

 *      If the call was successful, this function returns the number
 *      of * bytes read from the socket. Zero indicates that the
 *      sender has * closed * the connection (this is the regular way
 *      to check for * an orderly shutdown on a socket).
 *
 *      \par
 *
 *      If the call was not successful, this function returns -1.  To
 *      get more information about the error, you can use the
 *      minet_error() * or minet_perror() functions.  See the Linux
 *      manual page for \c recvfrom(2) for a list of possible error
 *      codes.  Additionally, you can set * #MINET_DEBUGLEVEL in \c
 *      setup.sh to 1 or greater to see more information about errors
 *      that occur while Minet is operating.
 *
 *	\par
 *
 *	If minet_init() was called with MINET_USER, an error code of
 *	\c ENODEV is used to signify that Minet did not initialize
 *	successfully.  Check that the call to minet_init() was
 *	successful.
 *
 * @remarks
 *
 *	recvfrom() is typically used by connectionless sockets (such
 *	as UDP).
 *
 *	\par
 *
 *	The socket must be bound before minet_recvfrom() can be used.
 *	For servers, this is accomplished explicitly using
 *	minet_bind(). For clients, this is generally accomplished
 *	implicitly via a prior call to minet_sendto().
 *
 *	\par
 *
 *	Note that this function is blocking - if there is no data when
 *	minet_read() is called, this function will block until data
 *	arrives.  You can use minet_select() and minet_poll() to avoid
 *	unnecessary blocking behavior.
 *
 *	\par
 *
 *	Note that not all of the data will necessarily be read in a
 *	single call.  It may be necessary to place the function call
 *	inside a loop.
 *
 *	\par
 *
 *	In Linux, the \c sockaddr_in structure is defined in the
 *	<netinet/in.h> header.  It typically looks like this: \include
 *	sockaddr_in.cc
 *
 *	\par
 *
 *	And the \c in_addr structure typically looks like this:
 *	\include in_addr.cc
 *
 *	\par
 *
 *	Note that byte ordering is a concern in networking code, so
 *	you should use the ntohs() function if you intend on reading
 *	and storing the port number in \c sockaddr_in.sin_port
 *	locally.  See <a
 *	href="http://beej.us/guide/bgnet/output/html/multipage/htonsman.html">Section
 *	9.12</a> in <a
 *	href="http://beej.us/guide/bgnet/output/html/multipage/index.html">Beej's
 *	Guide to Network Programming</a> for more information.
 *
 *	\par
 *
 *	To convert the IP address to a string (e.g., for display), you
 *	can use the inet_ntop() function from <arpa/inet.h>.  See <a
 *	href="http://beej.us/guide/bgnet/output/html/multipage/inet_ntopman.html">Section
 *	9.14</a> in <a
 *	href="http://beej.us/guide/bgnet/output/html/multipage/index.html">Beej's
 *	Guide to Network Programming</a> for more information.
 *
 * @bug Will crash if you pass NULL for the "from" param.
 *
 * @todo Technically, it is possible to use recvfrom() with TCP, but
 * the code currently assumes it will only be used with UDP when the
 * SockLibRequestResponse object is being constructed.
 */
EXTERNC int minet_recvfrom (int fd, char * buf, int len,
                            struct sockaddr_in * from) {

    socklen_t length = sizeof(*from);

    switch (socket_type) {
	case UNINIT_SOCKS:
	    errno = ENODEV;            // "No such device" error
	    return -1;
	    break;
	case KERNEL_SOCKS:
	    return recvfrom(fd, buf, len, 0, (sockaddr *)from, &length);
	    break;
	case MINET_SOCKS: {
	    SockLibRequestResponse slrr(mRECVFROM,
					Connection(IP_ADDRESS_ANY,
						   IPAddress((unsigned)from->sin_addr.s_addr),
						   UDP_PORT_NONE,
						   (unsigned short)from->sin_port,
						   0),
					fd,
					Buffer(buf, len),
					0, 0);
	    MinetSend(sock, slrr);
	    MinetReceive(sock, slrr);
	    minet_errno = slrr.error;

	    if (minet_errno != EOK) {
		return -1;
	    }

	    return 0;
	    break;
	}
	default:
	    minet_errno = ENODEV;
	    break;
    }

    return -1;
}


/**
 * @brief Send data to a specific destination.
 *
 *
 * ====== YOU'RE HERE ======
 *
 *
 */
EXTERNC int minet_sendto(int fd, char * buf, int len,
                          struct sockaddr_in * to) {
    switch (socket_type) {
	case UNINIT_SOCKS:
	    errno = ENODEV;            // "No such device" error
	    return (-1);
	    break;
	case KERNEL_SOCKS:
	    return sendto(fd, buf, len, 0, (sockaddr *)to, sizeof(*to));
	    break;
	case MINET_SOCKS: {
	    SockLibRequestResponse slrr(mSENDTO,
					Connection(IP_ADDRESS_ANY,
						   IPAddress(ntohl((unsigned)to->sin_addr.s_addr)),
						   UDP_PORT_NONE,
						   ntohs((unsigned short)to->sin_port),
						   0),
					fd,
					Buffer(buf, len),
					0, 0);
	    MinetSend(sock, slrr);
	    MinetReceive(sock, slrr);
	    minet_errno = slrr.error;

	    if (minet_errno != EOK) {
		return -1;
	    }
	    
	    return 0;
	    break;
	}
	default:
	    minet_errno=ENODEV;
	    break;
    }

    return -1;
}


EXTERNC int minet_close(int sockfd) {
    switch (socket_type) {
	case UNINIT_SOCKS:
	    errno = ENODEV;            // "No such device" error
	    return -1;
	    break;
	case KERNEL_SOCKS:
	    return close(sockfd);
	    break;
	case MINET_SOCKS: {
	    SockLibRequestResponse slrr(mCLOSE, Connection(), sockfd,
					Buffer(), 0, 0);
	    MinetSend(sock, slrr);
	    MinetReceive(sock, slrr);
	    minet_errno = slrr.error;

	    if (minet_errno != EOK) {
		return -1;
	    }

	    return 0;
	    break;
	}
	default:
	    minet_errno = ENODEV;
	    break;
    }

    return -1;
}

#define SELECTPOLL_BROKEN 1

/*
  This is PAD:  I don't understand what this thing is trying to do
  Why is it re-opening the minet fifos?
*/
EXTERNC int minet_select (int minet_maxfd,
                          fd_set * minet_read_fds,
                          fd_set * minet_write_fds,
                          fd_set * minet_except_fds,
                          struct timeval * timeout) {
    switch (socket_type) {
	case UNINIT_SOCKS:
	    errno = ENODEV;            // "No such device" error
	    return (-1);
	    break;
	case KERNEL_SOCKS:
	    return (select (minet_maxfd, minet_read_fds, minet_write_fds,
			    minet_except_fds, timeout));
	    break;
	case MINET_SOCKS:
#if SELECTPOLL_BROKEN
	    return ENOT_IMPLEMENTED;
#else
	    fd_set minet_read_fifos, minet_write_fifos, minet_except_fifos;

	    FD_ZERO(&minet_read_fifos);
	    FD_ZERO(&minet_write_fifos);
	    FD_ZERO(&minet_except_fifos);

	    for (int fd = 0; fd < (minet_maxfd + 1); fd++) {
#if 0
		if (FD_ISSET(fd, minet_read_fds)) {
		    fd = open(sock2app_fifo_name, O_RDONLY);
		    FD_SET(fd, &minet_read_fifos);
		} else if (FD_ISSET(fd, minet_write_fds)) {
		    fd = open(app2sock_fifo_name, O_WRONLY);
		    FD_SET(fd, &minet_write_fifos);
		} else if (FD_ISSET(fd, minet_except_fds)) {
		    fd = open(app2sock_fifo_name, O_WRONLY);
		    FD_SET(fd, &minet_except_fifos);
		}
#else
		if (FD_ISSET(fd, minet_read_fds)) {
		    FD_SET(fd, &minet_read_fifos);
		} else if (FD_ISSET(fd, minet_write_fds)) {
		    FD_SET(fd, &minet_write_fifos);
		} else if (FD_ISSET(fd, minet_except_fds)) {
		    FD_SET(fd, &minet_except_fifos);
		}
#endif
	    }
	    
        // eh? - PAD
        //int numfds = select(minet_maxfd, &minet_read_fifos, &minet_write_fifos,
        //&minet_except_fifos, timeout);

        SockLibRequestResponse slrr;

        if (numfds) {
            slrr = SockLibRequestResponse(mSELECT,
                                          Connection(),
                                          UNSPECIFIED_SOCK,
                                          Buffer(),
                                          0,
                                          ESET_SELECT,
                                          minet_read_fifos,
                                          minet_write_fifos,
                                          minet_except_fifos);
	    
            MinetSend(sock, slrr);
            MinetReceive(sock, slrr);
        } else {
            slrr = SockLibRequestResponse(mSELECT,
                                          Connection(),
                                          UNSPECIFIED_SOCK,
                                          Buffer(),
                                          0,
                                          ECLEAR_SELECT,
                                          minet_read_fifos,
                                          minet_write_fifos,
                                          minet_except_fifos);
	    
            MinetSend(sock, slrr);
            MinetReceive(sock, slrr);
        }


        int ctr = 0;

        for (int j = 0; j < (minet_maxfd + 1); j++) {
            if (FD_ISSET(j, &minet_read_fifos) ||
		FD_ISSET(j, &minet_write_fifos) ||
		FD_ISSET(j, &minet_except_fifos)) {
		ctr++;
	    }
        }

        return (ctr);
#endif
        break;
    default:
        minet_errno = ENODEV;
        break;
    }

    return -1;
}


EXTERNC int minet_select_ex (int minet_maxfd,
                             fd_set * minet_read_fds,
                             fd_set * minet_write_fds,
                             fd_set * minet_except_fds,
                             int unix_maxfd,
                             fd_set * unix_read_fds,
                             fd_set * unix_write_fds,
                             fd_set * unix_except_fds,
                             struct timeval * timeout) {
    switch (socket_type) {
	case UNINIT_SOCKS:
	    errno = ENODEV;            // "No such device" error
	    return -1;
	    break;
	case KERNEL_SOCKS:
#if SELECTPOLL_BROKEN
	    return ENOT_IMPLEMENTED;
#else
	    // NO, this is WRONG
	    return (select(minet_maxfd, minet_read_fds, minet_write_fds,
			   minet_except_fds, timeout) ||
		    select(unix_maxfd, unix_read_fds, unix_write_fds,
			   unix_except_fds, timeout));
#endif
	    break;
	case MINET_SOCKS:
#if SELECTPOLL_BROKEN
	    return ENOT_IMPLEMENTED;
#else
	    fd_set minet_read_fifos, minet_write_fifos, minet_except_fifos;

	    FD_ZERO(&minet_read_fifos);
	    FD_ZERO(&minet_write_fifos);
	    FD_ZERO(&minet_except_fifos);

	    for (int fd = 0; fd < (minet_maxfd + 1); fd++) {
#if 0
		if (FD_ISSET(fd, minet_read_fds)) {
		    fd = open(sock2app_fifo_name, O_RDONLY);
		    FD_SET(fd, &minet_read_fifos);
		} else if (FD_ISSET(fd, minet_write_fds)) {
		    fd = open(app2sock_fifo_name, O_WRONLY);
		    FD_SET(fd, &minet_write_fifos);
		} else if (FD_ISSET(fd, minet_except_fds)) {
		    fd = open(app2sock_fifo_name, O_WRONLY);
		    FD_SET(fd, &minet_except_fifos);
		};
#else
		if (FD_ISSET(fd, minet_read_fds)) {
		    FD_SET(fd, &minet_read_fifos);
		} else if (FD_ISSET(fd, minet_write_fds)) {
		    FD_SET(fd, &minet_write_fifos);
		} else if (FD_ISSET(fd, minet_except_fds)) {
		    FD_SET(fd, &minet_except_fifos);
		};
#endif
	    }

	    //int numfds = select(minet_maxfd, &minet_read_fifos, &minet_write_fifos,
	    //&minet_except_fifos, timeout);
	    SockLibRequestResponse slrr;
	    if (numfds) { 
		slrr = SockLibRequestResponse(mSELECT,
					      Connection(),
					      UNSPECIFIED_SOCK,
					      Buffer(),
					      0,
					      ESET_SELECT,
					      minet_read_fifos,
					      minet_write_fifos,
					      minet_except_fifos);
		
		MinetSend(sock,slrr);
		MinetReceive(sock,slrr);
	    } else {
		slrr = SockLibRequestResponse(mSELECT,
					      Connection(),
					      UNSPECIFIED_SOCK,
					      Buffer(),
					      0,
					      ECLEAR_SELECT,
					      minet_read_fifos,
					      minet_write_fifos,
					      minet_except_fifos);
		
		MinetSend(sock,slrr);
		MinetReceive(sock,slrr);
	    }
	    
	    int ctr = 0;
	    
	    for (int j = 0; j < (minet_maxfd + 1); j++) {
		if (FD_ISSET(j, &minet_read_fifos) ||
		    FD_ISSET(j, &minet_write_fifos) ||
		    FD_ISSET(j, &minet_except_fifos)) {
		    ctr++;
		}
	    }
	    return ctr;
	    
	    // NO NO NO NO
	    //    return (ctr ||
	    //    select(unix_maxfd, unix_read_fds, unix_write_fds,
	    //	   unix_except_fds, timeout));
#endif
	    break;
	default:
	    minet_errno = ENODEV;
	    break;
    }

    return -1;
}


EXTERNC int minet_poll(struct pollfd * minet_fds,
		       int num_minet_fds,
		       int timeout) {
    switch (socket_type) {
	case UNINIT_SOCKS:
	    errno = ENODEV;            // "No such device" error
	    return -1;
	    break;
	case KERNEL_SOCKS:
	    return poll(minet_fds, num_minet_fds, timeout);
	    break;
	case MINET_SOCKS:
#if SELECTPOLL_BROKEN
	    return ENOT_IMPLEMENTED;
#else
	    struct pollfd * minet_fifos = (struct pollfd*)malloc(num_minet_fds * sizeof(struct pollfd));

	    for (int i = 0; i < num_minet_fds; i++) {
		minet_fifos[i].fd = open(sock2app_fifo_name, O_RDWR);
		minet_fifos[i].events = minet_fds[i].events;
		minet_fifos[i].revents = minet_fds[i].revents;
	    }

	    int revnts = poll(minet_fifos, num_minet_fds, timeout);

	    if (revnts) {
		SockLibRequestResponse slrr = SockLibRequestResponse(mPOLL, Connection(), UNSPECIFIED_SOCK,
								     Buffer(), 0, 0,
								     num_minet_fds, *minet_fifos);
		slrr.Serialize(tosock);
		slrr.Unserialize(fromsock);
	    } else {
		int ctr = 0;

		for (int j = 0; j < num_minet_fds; j++) {	
		    if (minet_fds[j].revents) {
			ctr++;
		    }
		}

		return (ctr);
	    }

	    return revnts;
#endif
	    break;
	default:
	    minet_errno = ENODEV;
	    break;
    }

    return -1;
}


EXTERNC int minet_poll_ex(struct pollfd * minet_fds,
			  int num_minet_fds,
			  struct pollfd * unix_fds,
			  int num_unix_fds,
			  int timeout) {
    switch (socket_type) {
	case UNINIT_SOCKS:
	    errno = ENODEV;            // "No such device" error
	    return -1;
	    break;
	case KERNEL_SOCKS:
	    return poll(minet_fds, num_minet_fds, timeout);
	    break;
	case MINET_SOCKS:
#if SELECTPOLL_BROKEN
	    return ENOT_IMPLEMENTED;
#else

	    struct pollfd * minet_fifos = (struct pollfd *)malloc(num_minet_fds * sizeof(struct pollfd));

	    for (int i = 0; i < num_minet_fds; i++) {
		minet_fifos[i].fd = open(sock2app_fifo_name, O_RDWR);
		minet_fifos[i].events = minet_fds[i].events;
		minet_fifos[i].revents = minet_fds[i].revents;
	    }

	    int revnts = poll(minet_fifos, num_minet_fds, timeout);

	    if (revnts) {
		SockLibRequestResponse slrr = SockLibRequestResponse(mPOLL, Connection(), UNSPECIFIED_SOCK,
								     Buffer(), 0, 0,
								     num_minet_fds, *minet_fifos);
		slrr.Serialize(tosock);
		slrr.Unserialize(fromsock);
	    } else {
		int ctr = 0;

		for (int j = 0; j < num_minet_fds; j++) {
		    if (minet_fds[j].revents) {
			ctr++;
		    }
		}

		return (ctr || poll(unix_fds, num_unix_fds, timeout));
	    }

	    return (revnts || poll(unix_fds, num_unix_fds, timeout));
#endif
	    break;
	default:
	    minet_errno = ENODEV;
	    break;
    }

    return -1;
}

/**
 * @todo 
 *       Does Minet *actually* support non-blocking sockets for *
 *       MINET_USER at the moment?  After searching around in the
 *       code, it * seems that mSET_NONBLOCKING * currently doesn't
 *       actually do * anything.  Maybe it doesn't need to?  (Test
 *       it!)
 */
EXTERNC int minet_set_nonblocking(int sockfd) {
    switch (socket_type) {
    case UNINIT_SOCKS:
        errno = ENODEV;            // "No such device" error
        return -1;
        break;
    case KERNEL_SOCKS: {
        int val = 1;
        int x = ioctl(sockfd, FIONBIO, &val);

        if (x) {
            return -1;
	}

        return 0;
	break;
    }
    case MINET_SOCKS: {
        SockLibRequestResponse slrr(mSET_NONBLOCKING, Connection(), sockfd,
                                    Buffer(), 0, 0);

        MinetSend(sock, slrr);
        MinetReceive(sock, slrr);
        minet_errno = slrr.error;

        if (minet_errno != EOK) {
            return -1;
	}

        return 0;
	break;
    }
    default:
        minet_errno = ENODEV;
        break;
    }

    return -1;
}


EXTERNC int minet_set_blocking(int sockfd) {
    switch (socket_type) {
	case UNINIT_SOCKS:
	    errno = ENODEV;            // "No such device" error
	    return (-1);
	    break;

	case KERNEL_SOCKS: {
	    int val = 0;
	    int x = ioctl(sockfd, FIONBIO, &val);

	    if (x) {
		return (-1);
	    }

	    return 0;
	    break;
	}
	case MINET_SOCKS: {
	    SockLibRequestResponse slrr(mSET_BLOCKING, Connection(), sockfd,
					Buffer(), 0, 0);
	    MinetSend(sock, slrr);
	    MinetReceive(sock, slrr);
	    minet_errno = slrr.error;

	    if (minet_errno != EOK) {
		return -1;
	    }

	    return 0;
	    break;
	}
	default:
	    minet_errno = ENODEV;
	    break;
    }

    return -1;
}


EXTERNC int minet_can_write_now(int sockfd) {
    switch (socket_type) {

	case UNINIT_SOCKS:
	    errno = ENODEV;            // "No such device" error
	    return -1;
	    break;

	case KERNEL_SOCKS:
	    fd_set write_fds;
	    FD_ZERO(&write_fds);
	    FD_SET(sockfd, &write_fds);
	    return select(sockfd+1, NULL, &write_fds, NULL, 0);
	    break;

	case MINET_SOCKS: {
	    SockLibRequestResponse slrr(mCAN_WRITE_NOW, Connection(), sockfd,
					Buffer(), 0, 0);
	    MinetSend(sock, slrr);
	    MinetReceive(sock, slrr);
	    minet_errno = slrr.error;

	    if (minet_errno != EOK) {
		return -1;
	    }

	    return 0;
	    break;
	}
	default:
	    minet_errno = ENODEV;
	    break;
    }

    return -1;
}


EXTERNC int minet_can_read_now(int sockfd) {
    switch (socket_type) {
	case UNINIT_SOCKS:
	    errno = ENODEV;            // "No such device" error
	    return -1;
	    break;

	case KERNEL_SOCKS:
	    fd_set read_fds;
	    FD_ZERO(&read_fds);
	    FD_SET(sockfd, &read_fds);
	    return select(sockfd + 1, &read_fds, NULL, NULL, 0);
	    break;

	case MINET_SOCKS: {
	    SockLibRequestResponse slrr(mCAN_READ_NOW, Connection(), sockfd,
					Buffer(), 0, 0);

	    MinetSend(sock, slrr);
	    MinetReceive(sock, slrr);
	    minet_errno = slrr.error;

	    if (minet_errno != EOK) {
		return -1;
	    }

	    return 0;
	    break;

	}
	default:
	    minet_errno = ENODEV;
	    break;
    }

    return -1;
}
