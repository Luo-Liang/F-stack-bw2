TOPDIR=..

ifeq ($(FF_PATH),)
	FF_PATH=${TOPDIR}
endif

ifeq ($(FF_DPDK),)
	FF_DPDK=${TOPDIR}/dpdk/x86_64-native-linuxapp-gcc
endif

LIBS+= -L${FF_PATH}/lib -Wl,--whole-archive,-lfstack,--no-whole-archive
LIBS+= -L${FF_DPDK}/lib -Wl,--whole-archive,-ldpdk,--no-whole-archive
LIBS+= -Wl,--no-whole-archive -lrt -lm -ldl -lcrypto -pthread -lnuma
LDFLAGS += -lstdc++
TARGET="helloworld"
all:
	g++ -O3 -std=c++11  -gdwarf-2  -I../lib -o ${TARGET} main.cpp ${LIBS} ${LDFLAGS}
	g++ -O3 -std=c++11  -gdwarf-2  -I../lib -o ${TARGET}_epoll main_epoll.cpp ${LIBS} ${LDFLAGS}
	g++ -O3 -std=c++11  -gdwarf-2  -I../lib -o ${TARGET} main_client.cpp ${LIBS} ${LDFLAGS}

.PHONY: clean
clean:
	rm -f *.o ${TARGET} ${TARGET}_epoll ${TARGET}_client
