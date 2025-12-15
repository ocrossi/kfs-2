NAME_LIBC := libc.a
NAME_LIBK := libk.a

COMMONDIR ?= ../common

SRCDIR := src
OBJDIR := obj

DEFAULT_HOST != $(COMMONDIR)/default_host.sh
HOST ?= $(DEFAULT_HOST)
HOSTARCH != $(COMMONDIR)/target_to_arch.sh $(HOST)

CFLAGS ?= -O2 -g
CPPFLAGS ?=
LDFLAGS ?=
LIBS ?=

CFLAGS := $(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS := $(CPPFLAGS) -D__is_libc -Iinclude
LIBK_CFLAGS := $(CFLAGS)
LIBK_CPPFLAGS := $(CPPFLAGS) -D__is_libk

DESTDIR ?=
PREFIX ?= /usr/local
EXEC_PREFIX ?= $(PREFIX)

INCDIR ?= $(PREFIX)/include
LIBDIR ?= $(EXEC_PREFIX)/lib

ARCHDIR = arch/$(HOSTARCH)

ARCHSRCDIR = $(SRCDIR)/$(ARCHDIR)
ARCHOBJDIR = $(OBJDIR)/$(ARCHDIR)

include $(ARCHSRCDIR)/make.config

CFLAGS := $(CFLAGS) $(ARCH_CFLAGS)
CPPFLAGS := $(CPPFLAGS) $(ARCH_CPPFLAGS)
LIBK_CFLAGS := $(LIBK_CFLAGS) $(KERNEL_ARCH_CFLAGS)
LIBK_CPPFLAGS := $(CPPFLAGS) -D__is_libk

LIBK_SUFFIX := .libk

FREEOBJS=\
$(addprefix $(OBJDIR)/, \
$(ARCH_FREEOBJS) \
stdio/printf.o \
stdio/putchar.o \
stdio/puts.o \
stdlib/abort.o \
string/memcmp.o \
string/memcpy.o \
string/memmove.o \
string/memset.o \
string/strlen.o \
)

HOSTEDOBJS=\
$(addprefix $(OBJDIR)/, \
$(ARCH_HOSTEDOBJS) \
)

OBJS=\
$(FREEOBJS) \
$(HOSTEDOBJS) \

LIBK_OBJS=$(FREEOBJS:.o=$(LIBK_SUFFIX).o)

BINARIES := $(NAME_LIBK)


.SUFFIXES: .o $(LIBK_SUFFIX).o .c .S

.PHONY: all clean install install-headers install-libs


all: $(BINARIES)

$(NAME_LIBC): $(OBJS)
	$(AR) rcus $@ $^

$(NAME_LIBK): $(LIBK_OBJS)
	$(AR) rcus $@ $^


$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(@D)
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.S
	mkdir -p $(@D)
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)


$(OBJDIR)/%.libk.o: $(SRCDIR)/%.c
	mkdir -p $(@D)
	$(CC) -MD -c $< -o $@ -std=gnu11 $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

$(OBJDIR)/%.libk.o: $(SRCDIR)/%.S
	mkdir -p $(@D)
	$(CC) -MD -c $< -o $@ $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)


clean:
	rm -f $(BINARIES) *.a
	rm -f $(OBJS) $(LIBK_OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) $(LIBK_OBJS:.o=.d) *.d */*.d */*/*.d


install: install-headers install-libs

install-headers:
	mkdir -p $(DESTDIR)$(INCDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCDIR)/.

install-libs: $(BINARIES)
	mkdir -p $(DESTDIR)$(LIBDIR)
	cp $(BINARIES) $(DESTDIR)$(LIBDIR)


-include $(OBJS:%.o=.d)
-include $(LIBK_OBJS:.o=.d)
