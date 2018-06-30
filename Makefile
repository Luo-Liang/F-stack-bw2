ifeq ($(RTE_SDK),)
$(error "Please define RTE_SDK environment variable")
endif

ifeq ($(RTE_ANS),)
$(error "Please define RTE_ANS environment variable")
endif

ifeq ($(RTE_TARGET),)
$(error "Please define RTE_TARGET environment variable")
endif

ifeq ($(FF_PATH),)
	FF_PATH=${TOPDIR}
endif

ifeq ($(FF_DPDK),)
	FF_DPDK=${TOPDIR}/dpdk/x86_64-native-linuxapp-gcc
endif


CC = gcc
RM = rm -f

CPPFLAGS += -O3 -std=c++11 -g \
          -I$(RTE_ANS)/librte_ans/include \
          -I$(RTE_ANS)/librte_anssock/include \
	  -I$(FF_PATH)/lib

LDLIBS += $(RTE_ANS)/librte_anssock/librte_anssock.a \
          -L$(RTE_SDK)/$(RTE_TARGET)/lib \
          -Wl,--whole-archive -Wl,-lrte_mbuf -Wl,-lrte_mempool_ring -Wl,-lrte_mempool -Wl,-lrte_ring -Wl,-lrte_eal \
	  -Wl,-lrte_ethdev -Wl,-lrte_timer \
	  -Wl,--no-whole-archive -Wl,-export-dynamic -lrt -pthread -ldl -lnuma -lstdc++ \
	  -L${FF_PATH}/lib -Wl,--whole-archive,-lfstack,--no-whole-archive \
	  -Wl,--no-whole-archive -lm  -lcrypto 

OBJS = main_epoll.o main_client.o
TARGET = main_epoll main_client
all:$(OBJS)
	$(CC) -o main_epoll main_epoll.o $(CPPFLAGS) $(LDLIBS)
	$(CC) -o main_client main_client.o $(CPPFLAGS) $(LDLIBS)

$(OBJS):%.o:%.cpp
	$(CC) -c $(CPPFLAGS) $< -o $@
clean:
	-$(RM) $(TARGET) $(OBJS)
