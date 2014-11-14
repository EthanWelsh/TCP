#include <iomanip>
#include "constate.h"

Time::Time(const Time &rhs) : timeval((timeval&)rhs)
{}

Time::Time(const timeval &rhs) : timeval(rhs)
{}

Time::Time(const double time)
{
    tv_sec = (int)time;
    tv_usec = (int)((time - ((double)((int)time))) * 1e6);
}

Time::Time(const unsigned sec, const unsigned usec)
{
    tv_sec = sec;
    tv_usec = usec;
}

Time::Time()
{
    SetToCurrentTime();
}

void Time::SetToCurrentTime()
{
    gettimeofday((struct timeval *)this, 0);
}

Time & Time::operator=(const Time &rhs)
{
    tv_sec = rhs.tv_sec;
    tv_usec = rhs.tv_usec;

    return *this;
}

Time & Time::operator=(const double &rhs)
{
    return operator = (Time(rhs));
}

Time::operator double() const
{
    return ((double)(tv_sec)) + ((double)(tv_usec) / 1e6);
}

bool Time::operator<(const Time &rhs) const
{
    if (tv_sec < rhs.tv_sec) {
	return true;
    } else if (tv_sec > rhs.tv_sec) {
	return false;
    } else {
	return (tv_usec < rhs.tv_usec);
    }    
}

bool Time::operator>(const Time &rhs) const
{
    return !(operator <(rhs));
}

bool Time::operator==(const Time &rhs) const
{
    return ((tv_sec == rhs.tv_sec) && (tv_usec == rhs.tv_usec));
}

std::ostream & Time::Print(std::ostream &os) const
{
    os << "Time"
       << "( sec="    << tv_sec
       << ", usec="   << tv_usec
       << ", double=" << ((double)(*this))
       << ")";

    return os;
}
