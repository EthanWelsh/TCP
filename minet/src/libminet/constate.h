#ifndef _constate
#define _constate

#include "sockint.h"
#include <deque>
#include <sys/time.h>
#include <unistd.h>

#include <iostream>
#include <typeinfo>

// connection state mapping

// To be interchangeably used with timevals
// Note this is NOT virtual
struct Time : public timeval {
    Time(const Time &rhs);
    Time(const timeval &rhs);
    Time(const double time);
    Time(const unsigned sec, const unsigned usec);
    Time(); // gets the current time

    Time & operator=(const Time &rhs);
    Time & operator=(const double &rhs);

    void SetToCurrentTime();
    
    operator double() const;
    
    bool operator<(const Time &rhs) const;
    bool operator>(const Time &rhs) const;
    bool operator==(const Time &rhs) const;

    std::ostream & Print(std::ostream &os) const;
    
    friend std::ostream &operator<<(std::ostream &os, const Time& L) {
	return L.Print(os);
    }
};


template <class STATE>
struct ConnectionToStateMapping {
    Connection connection;
    Time       timeout;
    STATE      state;
    bool       bTmrActive;
    
    ConnectionToStateMapping(const ConnectionToStateMapping<STATE> &rhs) :
	connection(rhs.connection), timeout(rhs.timeout), state(rhs.state), bTmrActive(rhs.bTmrActive) 
    {}
    
    ConnectionToStateMapping(const Connection &c, const Time &t, const STATE &s, const bool &b) :
	connection(c), timeout(t), state(s), bTmrActive(b) 
    {}
    
    ConnectionToStateMapping() : 
	connection(), timeout(), state(), bTmrActive() 
    {}
    
    ConnectionToStateMapping<STATE> & operator=(const ConnectionToStateMapping<STATE> &rhs)  {
	connection = rhs.connection; 
	timeout    = rhs.timeout; 
	state      = rhs.state;
	bTmrActive = rhs.bTmrActive; 

	return *this;
    }
    
    bool MatchesSource(const Connection &rhs) const {
	return connection.MatchesSource(rhs);
    }
    
    bool MatchesDest(const Connection &rhs) const {
	return connection.MatchesDest(rhs);
    }
    
    bool MatchesProtocol(const Connection &rhs) const {
	return connection.MatchesProtocol(rhs);
    }
    
    bool Matches(const Connection &rhs) const {
	return connection.Matches(rhs);
    }
    
    std::ostream & Print(std::ostream &os) const {
	os << "ConnectionToStateMapping"
	   << "( connection="  << connection
	   << ", timeout="     << timeout
	   << ", state="       << state
	   << ", bTmrActive="  << bTmrActive
	   << ")";

	return os;
    }
    
    friend std::ostream &operator<<(std::ostream &os, const ConnectionToStateMapping<STATE> &L) {
	return L.Print(os);
    }
};



template <class STATE>
class ConnectionList : public std::deque<ConnectionToStateMapping<STATE> > {
 public:
    ConnectionList(const ConnectionList &rhs) : std::deque<ConnectionToStateMapping<STATE> >(rhs) 
    {}
    
    ConnectionList() 
    {}
	
    typedef typename ConnectionList<STATE>::iterator iterator;
    typedef typename ConnectionList<STATE>::const_iterator const_iterator;
    
    typename ConnectionList<STATE>::iterator FindEarliest() {
	typename ConnectionList<STATE>::iterator ptr = this->end();
	typename ConnectionList<STATE>::iterator i = this->begin();
	
	// No connections in list
	if (this->empty()) {
	    return this->end();
	}

	// 1 connection in list
	if (this->size() == 1) {
	    if ((*i).bTmrActive == true) {
		return this->begin();
	    } else {
		return this->end();
 	    }
	}
	
	// More than one connection in list
	Time min=(*i).timeout;
	for (; i != this->end(); ++i) {

	    if (((*i).bTmrActive == true) && ((*i).timeout <= min)) {
		min = (*i).timeout;
		ptr = i;
	    }
	}

	return ptr;
    }
    
    typename ConnectionList<STATE>::iterator FindMatching(const Connection &rhs) {
	for (typename ConnectionList<STATE>::iterator i = this->begin(); i != this->end(); ++i) {
	    if ((*i).Matches(rhs)) {
		return i;
	    }
	}

	return this->end();
    }

    typename ConnectionList<STATE>::iterator FindMatchingSource(const Connection &rhs) {
	for (typename ConnectionList<STATE>::iterator i = this->begin(); i != this->end(); ++i) {
	    if ((*i).MatchesSource(rhs)) {
		return i;
	    }
	}
	return this->end();
    }

    typename ConnectionList<STATE>::iterator FindMatchingDest(const Connection &rhs) {
	for (typename ConnectionList<STATE>::iterator i = this->begin(); i != this->end(); ++i) {
	    if ((*i).MatchesDest(rhs)) {
		return i;
	    }
	}
	return this->end();
    }

    typename ConnectionList<STATE>::iterator FindMatchingProtocol(const Connection &rhs) {
	for (typename ConnectionList<STATE>::iterator i = this->begin(); i != this->end(); ++i) {
	    if ((*i).MatchesProtocol(rhs)) {
		return i;
	    }
	}
	return this->end();
    }
    
    std::ostream & Print(std::ostream &os) const {
	os << "ConnectionList(";

	for (const_iterator i = this->begin(); i != this->end(); ++i) {
	    os << (*i);
	}

	os << ")";

	return os;
    }
    
    friend std::ostream &operator<<(std::ostream &os, const ConnectionList& L) {
	return L.Print(os);
    }
};



#endif
