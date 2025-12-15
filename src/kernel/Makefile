NAME := kfs-1.bin

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
CPPFLAGS := $(CPPFLAGS) -D__is_kernel -Iinclude
LDFLAGS := $(LDFLAGS)
LIBS := $(LIBS) -nostdlib -lk -lgcc

DESTDIR ?=
PREFIX ?= /usr/local
EXEC_PREFIX ?= $(PREFIX)

INCDIR ?= $(PREFIX)/include
BOOTDIR ?= $(EXEC_PREFIX)/boot

ARCHDIR = arch/$(HOSTARCH)

ARCHSRCDIR = $(SRCDIR)/$(ARCHDIR)
ARCHOBJDIR = $(OBJDIR)/$(ARCHDIR)

include $(ARCHSRCDIR)/make.config

CFLAGS := $(CFLAGS) $(KERNEL_ARCH_CFLAGS)
CPPFLAGS := $(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
LDFLAGS := $(LDFLAGS) $(KERNEL_ARCH_LDFLAGS)
LIBS := $(LIBS) $(KERNEL_ARCH_LIBS)

KERNEL_OBJS=\
$(KERNEL_ARCH_OBJS) \
$(addprefix $(OBJDIR)/, \
kernel/kernel.o \
)

OBJS=\
$(ARCHOBJDIR)/crti.o \
$(ARCHOBJDIR)/crtbegin.o \
$(KERNEL_OBJS) \
$(ARCHOBJDIR)/crtend.o \
$(ARCHOBJDIR)/crtn.o

LINK_LIST=\
$(LDFLAGS) \
$(ARCHOBJDIR)/crti.o \
$(ARCHOBJDIR)/crtbegin.o \
$(KERNEL_OBJS) \
$(LIBS) \
$(ARCHOBJDIR)/crtend.o \
$(ARCHOBJDIR)/crtn.o \

all: $(NAME)

# Executables
$(NAME): $(OBJS) $(ARCHSRCDIR)/linker.ld
	$(CC) -T $(ARCHSRCDIR)/linker.ld -o $@ $(CFLAGS) $(LINK_LIST)

# Objects
$(ARCHOBJDIR)/crtbegin.o $(ARCHOBJDIR)/crtend.o:
	mkdir -p $(@D)
	OBJ=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(@D)
	$(CC) -MD -c $< -o $@ --std=gnu11 $(CFLAGS) $(CPPFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.S
	mkdir -p $(@D)
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)


clean:
	rm -rf $(OBJDIR) $(NAME)


install: install-headers install-kernel

install-headers:
	mkdir -p $(DESTDIR)$(INCDIR)/kernel
	cp -R --preserve=timestamps include/kernel/. $(DESTDIR)$(INCDIR)/kernel/.

install-kernel: $(NAME)
	mkdir -p $(DESTDIR)$(BOOTDIR)
	cp $(NAME) $(DESTDIR)$(BOOTDIR)

-include $(OBJS:.o=.d)
