libminet-dir := src/libminet
libminet_socket-dir := src/libminet_socket
app-dir := src/apps
module-dir := src/modules
lowlevel-dir := src/lowlevel

include $(libminet-dir)/Makefile.in
include $(libminet_socket-dir)/Makefile.in
include $(app-dir)/Makefile.in
include $(module-dir)/Makefile.in
include $(lowlevel-dir)/Makefile.in

CXX=g++
CC=gcc
AR=ar
RANLAB=ranlib


include-dirs = -I/usr/include/pcap -I$(libminet-dir) -I$(libminet_socket-dir)

libraries = -lnet -lpcap lib/libminet_socket.a lib/libminet.a

CXXFLAGS = -g -ggdb -gstabs+ -Wall -std=c++0x -fPIC


build = \
	@if [ -z "$V" ]; then \
		echo '    [$1]	$@'; \
		$2; \
	else \
		echo '$2'; \
		$2; \
	fi



CXX_COMPILE = \
	$(call build,CXX,$(CXX) \
		$(CXXFLAGS) \
		$(include-dirs) \
		-c \
		$< \
		-o $@ \
	)


libminet-objs      := $(patsubst %, $(libminet-dir)/%, $(libminet-objs) )
libminet_socket-objs      := $(patsubst %, $(libminet_socket-dir)/%, $(libminet_socket-objs) )

vpath %.o $(app-dir)
vpath %.o $(module-dir)
vpath %.o $(lowlevel-dir)

apps          := $(patsubst %.o, %, $(app-objs)) \
		 $(patsubst %.o, %, $(module-objs)) \
		 $(patsubst %.o, %, $(lowlevel-objs))

app-objs      := $(patsubst %, $(app-dir)/%, $(app-objs))
module-objs      := $(patsubst %, $(module-dir)/%, $(module-objs))
lowlevel-objs      := $(patsubst %, $(lowlevel-dir)/%, $(lowlevel-objs))

all-objs := $(libminet-objs) $(libminet_socket-objs) $(app-objs) $(module-objs) $(lowlevel-objs)

% :  %.o
	@mkdir -p bin
	$(call build,INSTALL,$(CXX) $< $(libraries) -o  bin/$@ ) 


%.o: %.cc
	$(CXX_COMPILE)


all: libminet libminet_socket $(app-objs) $(module-objs) $(lowlevel-objs) $(apps)


lib/libminet.a: $(libminet-objs)
	@mkdir -p lib
	$(call build,AR,$(AR) rcs $@ $^)

lib/libminet_socket.a: $(libminet_socket-objs)
	@mkdir -p lib
	$(call build,AR,$(AR) rcs $@ $^)

libminet: lib/libminet.a

libminet_socket: lib/libminet_socket.a



clean: 
	rm -f $(libminet_socket-objs) $(libminet-objs) $(app-objs) $(module-objs) $(lowlevel-objs)
	rm -f lib/* bin/*

#
#depend:
#	$(CXX) $(CXXFLAGS) $(include-dirs)  -MM $(all-objs:.o=.cc) > .dependencies
#
#include .dependencies
#
