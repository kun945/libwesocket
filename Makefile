##
## create by ChenKun(ckmx945@gmail.com)
## at 2016-04-18 10:28:03
##

include target.mk

SOURCEDIRS 	= $(shell ls -AxR $(SRCDIR) | grep ":" | grep -v "\.svn" | tr -d ':')
SOURCES 	= $(foreach d, $(SOURCEDIRS), $(wildcard $(d)/*.c))
SRCOBJS 	= $(patsubst %.c, %.o, $(SOURCES))
BUILDOBJS 	= $(subst $(SRCDIR), $(OBJECTDIR), $(SRCOBJS))
BUILDDEPS 	= $(patsubst %.o, %.d, $(BUILDOBJS))


#gcc flags
TMP_INCLUDES	= -I$(SRCDIR) -I$(INCLUDESDIR)
CFLAGS 	= $(COMMON_CFLAGS) $(COMMON_INCLUDES) $(TMP_INCLUDES) -D_GNU_SOURCE

LDFLAGS += -lm

ifdef DEBUG
	CFLAGS 	+= -g
else
	CFLAGS 	+= -O2
endif # DEBUG

#ar flags
ARFLAGS 	= rv

# Generate the target
LIBSO 		= $(OBJECTDIR)/libws.so
LIBA 		= $(OBJECTDIR)/libws.a

# Test target
TEST_DIR 	= $(TOPDIR)/test
TEST_SRCS 	= $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS 	= $(patsubst %.c, %.o, $(TEST_SRCS))
TEST_DEPS 	= $(patsubst %.c, %.d, $(TEST_SRCS))
TEST_TAGS 	= $(basename $(TEST_SRCS))

.PHONY : all release install clean info test

-include $(BUILDDEPS)

all: $(LIBSO) $(LIBA)

$(LIBA) : $(BUILDOBJS)
	$(AR) $(ARFLAGS) $@ $(BUILDOBJS)
	$(RANLIB) $@

$(LIBSO) : $(BUILDOBJS)
	$(CC) -shared $(subst $(SRCDIR), $(OBJECTDIR), $^) $(LDFLAGS) -o $@

$(OBJECTDIR)%.o : $(SRCDIR)%.c
	@[ ! -d $(dir $(subst $(SRCDIR), $(OBJECTDIR), $@)) ] & mkdir -p $(dir $(subst $(SRCDIR), $(OBJECTDIR), $@))
	$(CC) $(CFLAGS) -o $(subst $(SRCDIR), $(OBJECTDIR), $@) -c $<

test : $(BUILDOBJS)
	@for tag in $(TEST_TAGS); do echo "CC $$tag"; $(CC) $(CFLAGS) -o $(addsuffix .o, $$tag) -c $(addsuffix .c, $$tag); \
		$(CC) $(addsuffix .o, $$tag) $(filter-out $(MAIN_OBJ), $(BUILDOBJS)) -lev $(LDFLAGS) -static -o $$tag; done

release : $(LIBSO) $(LIBA)
	@[ ! -d $(RELEASEDIR)/include ] && mkdir -p $(RELEASEDIR)/include
	@[ ! -d $(RELEASEDIR)/lib ] && mkdir -p $(RELEASEDIR)/lib
	@cp -f $(LIBSO) $(RELEASEDIR)/lib
	@cp -f $(LIBA) $(RELEASEDIR)/lib
	@cp -rf $(INCLUDESDIR) $(RELEASEDIR)/include/iotm
	@tar -czvf $(RELEASEDIR)/libws_v$(VERSION_MAJOR).$(VERSION_MINOR)_$(shell date "+%Y-%m-%d_%H-%M-%S").tar.gz -C $(RELEASEDIR) lib include

clean :
	find $(OBJECTDIR) -name "*.o" -o -name "*.d" | xargs rm -f
	rm -rf $(LIBSO)
	rm -rf $(LIBA)
	rm -rf $(RELEASEDIR)/lib
	rm -rf $(RELEASEDIR)/include
	rm -rf $(TEST_OBJS)
	rm -rf $(TEST_TAGS)
	rm -rf $(TEST_DEPS)

info :
	@echo "TOOL_CHINA_PAT="$(TOOL_CHAIN_PATH)
	@echo "CROSS_COMPILE="$(CROSS_COMPILE)
	@echo "TOPDIR="$(TOPDIR)
	@echo "CFLAGS="$(CFLAGS)
	@echo "LDFLAGS="$(LDFLAGS)
	@echo "SOURCES"=$(SOURCES)
	@echo "SRCOBJS"=$(SRCOBJS)
	@echo "BUILDOBJS"=$(BUILDOBJS)

