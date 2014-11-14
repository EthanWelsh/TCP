#ifndef _attachdebugger
#define _attachdebugger

#include <cstdio>
#include <string>
#include <iostream>
#include <iomanip>

#define DEFAULT_DEBUG_PRINT_LEVEL 0 // overridable with MINET_DEBUGLEVEL env
#define DEFAULT_DEBUG_PRINT_FILE  stderr

#define STRINGIZE(T) #T


/**
 * @brief A value between 0 and 10 that specifies how much debugging information Minet should display while it is operating.
 *
 * Debug levels range from 0 (no debug output) to 10 (highly verbose output).  This value
 * can be changed by modifying \c setup.sh, or by calling DEBUGSETLEVEL().  This value is currently global across all of Minet.
 *
 * @remarks
 *		There is currently no convention regarding what each debug level represents.  In general, a debug level of 1 will only display
 *		serious errors that will prevent Minet from operating; therefore, you may find it helpful to set MINET_DEBUGLEVEL to 1 by
 *		default.  You may find higher debug levels to be obtrusive rather than helpful.
 *		\par
 *		If you prefer, you may also include debugging messages in your own modules using MINET_DEBUGLEVEL by using DEBUGPRINTF() or #debug.
 *
 * @see DEBUGSETLEVEL(), DEBUGPRINTF(), #debug
 *
 * @todo Maybe ship setup.sh with MINET_DEBUGLEVEL set to 1 by default to justify what I just said above.
 *
 * @todo The student would also like to know about BreakHere() and AttachDebuggerHere(), and might also appreciate a link
 *		to a guide on Minet debugging strategies.
 */
extern int MINET_DEBUGLEVEL;


void BreakHere();
void AttachDebuggerHere(char * execname = 0);

void DEBUGSETLEVEL(int level);
void DEBUGSETFILE(FILE * file);

/**
 * @brief Print a debug message that appears only if #MINET_DEBUGLEVEL is greater than or equal to the \c level parameter.
 *
 * \include DEBUGPRINTF.cc
 *
 * @param[in] level  The minimum debug level (#MINET_DEBUGLEVEL) required for the message to get displayed.
 * @param[in] fmt  Format string (similar to printf).
 * @param[in] ...  Variable-length argument list.
 *
 * @see #debug
 */
void DEBUGPRINTF(int level, const char *fmt, ...);

/**
 * C++ analog of DEBUGPRINTF() which allows objects to be printed on the stream.  Use the global #debug variable, or make a new instance of this class.
 *
 * Allows statements such as the following to work:
 * @code
 // Will print if MINET_DEBUGLEVEL is at least 3
 debug(3) << "The Connection object is: " << conn << endl;
 * @endcode
 *
 * Requires global variable #MINET_DEBUGLEVEL to be defined.
 *
 * @see #debug
 */
class DebugStream {
    //! The stream on which debug messages will be printed (e.g., std::cout or std::cerr)
    std::ostream& stream;
    
    //! Debug level for the current statement (e.g., debug(3) has level=3)
    short level;
    
  public:
    DebugStream(std::ostream &_stream, short _debug_level = 1)
	: stream(_stream), level(_debug_level) 
    {}
    
    template <class T>
    inline DebugStream& operator<<(const T &val) {
	if (MINET_DEBUGLEVEL >= level) {
	    stream << val;
	}
	return *this;
    }
    
    inline DebugStream& operator<<(std::ostream& (*val)(std::ostream&)) {
        if (MINET_DEBUGLEVEL >= level) {
            stream << val;
	}
        return *this;
    }
    
    inline DebugStream& operator()(short new_level) {
	level = new_level;
	return *this;
    }
};

/**
 * C++ analog of DEBUGPRINTF() which allows objects to be printed on the stream.
 *
 * Allows for debugging statements such as the following:
 * @code
 // Will print if MINET_DEBUGLEVEL is at least 3
 debug(3) << "The Connection object is: " << conn << endl;
 * @endcode
 *
 * Requires global variable #MINET_DEBUGLEVEL to be defined.
 *
 * @note This is a global instance of the DebugStream class. You may create a
 *		separate instance of that class and use that instead if you prefer.
 */
extern DebugStream debug;

#endif
