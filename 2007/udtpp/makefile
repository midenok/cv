SRC_DIR=/home/v00d00/src
UDT_DIR=${SRC_DIR}/udt3-cvs/src
UDT_OBJ=${UDT_DIR}/libudt.a
CPPFLAGS=-g -I${UDT_DIR} -DDEBUG -pthread -fno-exceptions
LDFLAGS=-pthread

MODULES=udtpp app tcptransp udttransp mainclient mainserver tcpserv udtserv udtmuxer Parse

all: udtpp

udttransp.h: transp.h
	touch $@

tcptransp.h: transp.h
	touch $@

udtpp.cpp: app.h
	touch $@

tcpserv.h: classmuxer.h
	touch $@

classmuxer.h: udtmuxer.h
	touch $@

udtmuxer.h: udttransp.h
	touch $@

%.cpp: %.h
	touch $@

%.o: %.cpp
	g++ ${CPPFLAGS} -c $^

udtpp: $(addsuffix .o, ${MODULES}) ${UDT_OBJ}
	g++ ${LDFLAGS} $^ -o $@

clean:
	rm -f *.o
	rm -f udtpp
