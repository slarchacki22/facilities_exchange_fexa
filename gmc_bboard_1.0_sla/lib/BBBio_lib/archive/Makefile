
SHELL := /bin/bash
INCLUDES = -I../../include/BBBio_lib

CPPFLAGS += -Wall $(INCLUDES)

CFLAGS += $(CPPFLAGS)

BBIO_OBJS = BBBiolib_ADCTSC.o \
            BBBiolib.o \
            BBBiolib_McSPI.o \
            BBBiolib_PWMSS.o

BBIO_SRCS = $(BBIO_OBJS:.o=.c)
BBIO_DEPS = $(BBIO_SRCS:%.c=.depend/%.d)

libBBBio.a: $(BBIO_OBJS)
	ar -rs $@ $(BBIO_OBJS)
	ranlib $@


-include $(BBIO_DEPS)

%.o:%.c
	gcc -c $(CFLAGS) $^
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
	rm -f *.o libBBBio.a -rf .depend
