ifeq ($(RTE_SDK),)
$(error "Please define RTE_SDK environment variable")
endif

ifeq ($(RTE_ANS),)
$(error "Please define RTE_ANS environment variable")
endif

ifeq ($(RTE_TARGET),)
$(erroyoutur "Please define RTE_TARGET environment variable")
endif

ifeq ($(FF_PATH),)
	FF_PATH=${TOPDIR}
endif

ifeq ($(FF_DPDK),)
	FF_DPDK=${TOPDIR}/dpdk/x86_64-native-linuxapp-gcc
endif


CC = g++
RM = rm -f

LINUX = 0
ANS = 1
FSTACK = 2

CPPFLAGS_LINUX += -O3 -std=c++11 -g

LDLIBS_LINUX += -lpthread

CPPFLAGS_ANS += -O3 -std=c++11 -g \
          -I$(RTE_ANS)/librte_ans/include \
          -I$(RTE_ANS)/librte_anssock/include \

LDLIBS_ANS += $(RTE_ANS)/librte_anssock/librte_anssock.a \
			  -L$(RTE_SDK)/$(RTE_TARGET)/lib \
              -Wl,--whole-archive -Wl,-lrte_mbuf -Wl,-lrte_mempool_ring -Wl,-lrte_mempool -Wl,-lrte_ring -Wl,-lrte_eal -Wl,--no-whole-archive -Wl,-export-dynamic \
              -lrt -pthread -ldl -lnuma -lstdc++

CPPFLAGS_FSTACK += -O3 -std=c++11 -g -I$(FF_PATH)/lib \

LDLIBS_FSTACK += -L${FF_PATH}/lib -Wl,--whole-archive,-lfstack,--no-whole-archive \
                 -L${FF_DPDK}/lib -Wl,--whole-archive,-ldpdk,--no-whole-archive \
                 -Wl,--no-whole-archive -lrt -lm -ldl -lcrypto -pthread -lnuma

#OBJS = main_epoll.o main_client.o
#TARGET = main_epoll main_client
all:
	$(CC) -D API=$(LINUX) main_epoll.cpp  $(CPPFLAGS_LINUX) -o server_linux $(LDLIBS_LINUX)
	$(CC) -D API=$(LINUX) main_client.cpp  $(CPPFLAGS_LINUX) -o client_linux $(LDLIBS_LINUX)

	$(CC) -D API=$(ANS) main_epoll.cpp  $(CPPFLAGS_ANS) -o server_ans $(LDLIBS_ANS)
	$(CC) -D API=$(ANS) main_client.cpp $(CPPFLAGS_ANS) -o client_ans $(LDLIBS_ANS)

	$(CC) -D API=$(FSTACK) main_epoll.cpp $(CPPFLAGS_FSTACK) -o server_fstack $(LDLIBS_FSTACK)
	$(CC) -D API=$(FSTACK) main_client.cpp  $(CPPFLAGS_FSTACK) -o client_fstack $(LDLIBS_FSTACK)

clean:
	-$(RM) *.o *fstack *ans *linux
