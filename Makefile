ifneq "$(CROSS_COMPILE)" ""
	CROSS=$(CROSS_COMPILE)
endif

ifeq "$(CROSS)" ""
	ARCHFLAGS=
endif

ifeq "$(DESTDIR)" ""
	DESTDIR=/
endif

ifeq "$(PREFIX)" ""
	PREFIX=/usr
endif

ifeq "$(CONF)" ""
	CONF=Debug
endif

CC=$(CROSS)gcc
AR=$(CROSS)ar
STRIP=$(CROSS)strip

CFLAGS_COMMON=-fPIC -Wall -Werror

ifeq "$(CONF)" "Debug"
    CFLAGS=-g -rdynamic -O0 $(CFLAGS_COMMON) $(ARCHFLAGS)
endif

ifeq "$(CONF)" "Release"
    CFLAGS=-O2 -DNDEBUG $(CFLAGS_COMMON) $(ARCHFLAGS) 
endif

ifeq "$(CONF)" "Size"
    CFLAGS=-Os -DNDEBUG $(CFLAGS_COMMON) $(ARCHFLAGS) -DLOGGING_LEVEL=3
endif

CXXFLAGS=$(CFLAGS)
BUILD=build/lib/$(CROSS)$(CONF)
BUILD_SAMPLE=build/sample/$(CROSS)$(CONF)
DIST=dist/$(CROSS)$(CONF)

NAME=m2mp_client
TARGET_SHARED=$(DIST)/lib$(NAME).so
TARGET_STATIC=$(DIST)/lib$(NAME).a
TARGET_SAMPLE_STATIC=$(DIST)/m2mp_client_sample_static
TARGET_SAMPLE_SHARED=$(DIST)/m2mp_client_sample_shared

# SPECIFIC part
INCLUDE=-Isrc -I$(DESTDIR)/usr/include
SRCS=$(shell find src -name "*.c")
SRCS_SAMPLE=$(shell find src_sample -name "*.c")

LDFLAGS=-L$(DIST)
# I'm commenting these but they might be useful someday:
#-ldl \
#-lz \
#-ljson \
#-lssl \
#-lcrypto \
#-lpthread

OBJ=$(addprefix $(BUILD)/,$(patsubst %.c,%.o,$(wildcard $(SRCS))))
OBJ_SAMPLE=$(addprefix $(BUILD)/,$(patsubst %.c,%.o,$(wildcard $(SRCS_SAMPLE))))

all: build build_sample

run: $(TARGET_SAMPLE_STATIC)
	$(TARGET_SAMPLE_STATIC) $(RUN_ARGS)

run-valgrind: $(TARGET_SAMPLE_STATIC)
	valgrind --leak-check=full --show-reachable=yes --track-origins=yes $(TARGET_SAMPLE_STATIC) $(RUN_ARGS)

run-valgrind-gdb: $(TARGET_SAMPLE_STATIC)
	sudo LD_LIBRARY_PATH=$(DIST) valgrind --db-attach=yes --leak-check=full --show-reachable=yes --track-origins=yes $(TARGET_SAMPLE) $(RUN_ARGS)
# For some unknown reasons, it seems the --db-attach requires valgrind to be run in sudo

run-callgrind: $(TARGET_SAMPLE_STATIC)
	@rm -f callgrind.*
	@valgrind --tool=callgrind $(TARGET_SAMPLE_STATIC) -- $(RUN_ARGS)
	kcachegrind callgrind.out.* &

build: $(TARGET_SAMPLE_STATIC) $(TARGET_SAMPLE_SHARED)
	@[ ! -e dist/last ] || rm dist/last
	@ln -s $(CROSS)$(CONF) dist/last

build_sample: $(TARGET_SAMPLE)
	
$(DIST): 
	[ -e $(DIST) ] || mkdir -p $(DIST)
	touch $(DIST)

$(TARGET_SHARED): $(OBJ) $(DIST)
	$(CC) -shared -o $(TARGET_SHARED) $(OBJ) $(CFLAGS) $(LDFLAGS) 
	
$(TARGET_STATIC): $(OBJ) $(DIST)
	$(AR) rcs $(TARGET_STATIC) $(OBJ)

$(TARGET_SAMPLE_SHARED): $(OBJ_SAMPLE) $(TARGET_SHARED)
	$(CC) -o $(TARGET_SAMPLE_SHARED) $(OBJ_SAMPLE) -lm2mp_client $(CFLAGS) $(LDFLAGS)
	
$(TARGET_SAMPLE_STATIC): $(OBJ_SAMPLE) $(TARGET_STATIC)
	$(CC) -o $(TARGET_SAMPLE_STATIC) $(OBJ_SAMPLE) $(TARGET_STATIC) $(CFLAGS) $(LDFLAGS)
	
small: $(TARGET_SAMPLE_STATIC) $(TARGET_SAMPLE_SHARED) 
	$(STRIP) $(TARGET_SAMPLE_STATIC) 
	$(STRIP) $(TARGET_SAMPLE_SHARED)
	[ ! -e /usr/bin/upx ] || /usr/bin/upx -9 $(TARGET_SAMPLE_STATIC)
	[ ! -e /usr/bin/upx ] || /usr/bin/upx -9 $(TARGET_SAMPLE_SHARED)
	
$(BUILD)/%.o: %.c $(BUILD)
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDE)  $<

build: $(BUILD)

$(BUILD): Makefile
	find src src_sample src_test -type d -exec mkdir -p $(BUILD)/{} \;
	touch $(BUILD)

clean:
	rm -Rf build dist

$(DESTDIR)$(PREFIX):
	[ -d $(DESTDIR)$(PREFIX) ] || mkdir -p $(DESTDIR)$(PREFIX)

# Test this with: make install DESTDIR=./install PREFIX=/
install: build Makefile $(DESTDIR)$(PREFIX)
	# Binaries
	[ -d $(DESTDIR)$(PREFIX)/bin ] || mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -u dist/last/m2mp_client.so $(DESTDIR)$(PREFIX)/lib/m2mp_client.so
	cp -u dist/last/m2mp_client_sample $(DESTDIR)$(PREFIX)/bin/m2mp_client_sample

