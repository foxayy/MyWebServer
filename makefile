all: server

CXX ?= g++

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2
endif

COMMON_OBJS = ./common/config.o ./common/timer/timer.o ./common/http/http_conn.o
SRC_OBJS = ./src/main.o ./src/webserver.o

server: $(SRC_OBJS) $(COMMON_OBJS)
	$(CXX) -o server $^ $(CXXFLAGS) -lpthread -lmysqlclient

clean:
	rm -f server
	rm -f *.o $(SRC_OBJS) $(COMMON_OBJS)
