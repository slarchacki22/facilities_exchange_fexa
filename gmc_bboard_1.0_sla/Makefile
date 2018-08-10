
SHELL := /bin/bash
INCLUDES = -I./include

CPPFLAGS += -Wall $(INCLUDES)

CFLAGS += $(CPPFLAGS)
LDFLAGS += -L./lib/BBBio_lib
LIBS = -lBBBio -lm

MCG_OBJS = eqep.o \
           gmc.o \
           mc_api.o \
           mux.o \
           eqep.o

MCG_SRCS = $(MCG_OBJS:.o=.c)
MCG_DEPS = $(MCG_SRCS:%.c=.depend/%.d)

target: BBBio_lib gmc

gmc: $(MCG_OBJS)
	gcc -o $@ $^ $(LDFLAGS) $(LIBS)

BBBio_lib:
	cd lib/BBBio_lib && $(MAKE)

-include $(MCG_DEPS)

%.o:%.c
	gcc -c $(CFLAGS) $*.c
	@gcc -MM $(CFLAGS) $^ > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@$(shell [[ ! -d .depend ]] && mkdir .depend)
	@mv $*.d .depend
	@rm -f $*.d.tmp
	
.PHONY: clean
clean:
	rm -f *.o gmc -rf .depend
	cd lib/BBBio_lib && $(MAKE) clean
