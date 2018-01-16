#--------------------------------------------------------------------
ifeq ($(GCOV_ON),1)
GCOV= -ftest-coverage -fprofile-arcs -lgcov
endif

CC = gcc
CXX = g++
AR = ar cru
CFLAGS = -Wall -D_REENTRANT -D_GNU_SOURCE -g -fPIC $(GCOV)
SOFLAGS = -shared

INC = -Iinc -Isrc -I../src/common
SYSINC = 
COMMON_INC=/usr/local/include/

SRC = src
LIB = lib
BIN = bin
TEST = test
TESTR = testr
CONF = conf

INCFLAGS = $(INC)             \
	   -I$(SYSINC) -I$(COMMON_INC) -I$(TEST)

CFLAGS += $(INCFLAGS)

LDFLAGS = -L/usr/local/lib/	-L$(LIB) \
		-lz -lpthread -ldl \
		-lboost_timer -lboost_chrono -lboost_system -lboost_filesystem -lboost_thread -lboost_date_time -lpthread -lrt \
		-levent -lcurl  -lsnappy -lyajl  -llog4cpp \
		-lhiredis -lstdc++  $(GCOV)


QKPACK = libqkpack
QKPACK_A = $(LIB)/$(QKPACK).a
QKPACK_SO = $(LIB)/$(QKPACK).so

EXE = $(BIN)/qkpack_test

#--------------------------------------------------------------------

LIBOBJS =  $(SRC)/qkpack_service.o \
	   $(SRC)/config/qkpack_acl.o \
	   $(SRC)/config/easycurl.o \
	   $(SRC)/core/qkpack_handler.o \
	   $(SRC)/core/redis_slot.o \
	   $(SRC)/core/redis_node.o \
	   $(SRC)/core/redis_cluster.o \
	   $(SRC)/core/qkpack_metrics.o \
	   $(SRC)/parser/qkpack_parser.o \
	   $(SRC)/parser/qkpack_parser_impl.o \
	   $(SRC)/parser/qkpack_yajl_json.o \
	   $(SRC)/parser/qkpack_yajl_validate.o \
	   $(SRC)/parser/qkpack_yajl_json_impl.o \
	   $(SRC)/protocol/redis_proto.o \
	   $(SRC)/util/string_util.o \
	   $(SRC)/util/filekv_parser.o \
	   $(SRC)/util/long_hash_value.o \
	   $(SRC)/util/short_string.o \
	   $(SRC)/util/log4cpp_wrapper.o \
	   $(SRC)/util/file_util.o \
	   $(SRC)/util/snprintf.o \
	   $(SRC)/metrics/uniform_sample.o \
	   $(SRC)/metrics/timer_context.o \
	   $(SRC)/metrics/timer.o \
	   $(SRC)/metrics/snapshot.o \
	   $(SRC)/metrics/metric_registry.o \
	   $(SRC)/metrics/ewma.o \
	   $(SRC)/metrics/meter.o \
	   $(SRC)/metrics/histogram.o \
	   $(SRC)/metrics/exp_decay_sample.o \
	   $(SRC)/snappys/snappystreamcfg.o \
	   $(SRC)/snappys/osnappystream.o \
	   $(SRC)/snappys/isnappystream.o \
	   $(SRC)/snappys/snappy_compress.o

TARGET = $(QKPACK_A)                     \
	 $(QKPACK_SO)                    


#--------------------------------------------------------------------
.PHONY: all clean install

all: $(TARGET)

$(QKPACK_SO): $(LIBOBJS)
	$(CXX) $(LDFLAGS) $(SOFLAGS) $^ -o $@

$(QKPACK_A): $(LIBOBJS)
	$(AR) $@ $^

clean:
	rm -rf  $(LIBOBJS) $(TARGET) $(TESTOBJS) test-output/* 

#--------------------------------------------------------------------

# make rule

%.o : %.cpp
	$(CXX) $(CFLAGS) -c $^ -o $@
	
%.o : %.cc
	$(CXX) $(CFLAGS) -c $^ -o $@	
