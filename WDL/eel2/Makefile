CC=gcc
CFLAGS=-g -DWDL_FFT_REALSIZE=8  -Wall -Wno-unused-function -Wno-multichar -Wno-unused-result -Wshadow
LFLAGS=
CXX=g++

ifdef DEBUG
CFLAGS += -D_DEBUG -O0
else
CFLAGS += -DNDEBUG -O
endif

OBJS=nseel-caltab.o nseel-compiler.o nseel-eval.o nseel-lextab.o nseel-ram.o nseel-yylex.o nseel-cfunc.o fft.o

OBJS2=

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
  ifdef ARCH
    CFLAGS += -arch $(ARCH)
  else
    ARCH = i686
    CFLAGS += -arch i386
  endif
else
  ARCH := $(shell uname -m)
endif

ifneq ($(filter arm%,$(ARCH)),)
  CFLAGS += -fsigned-char -mfpu=vfp -march=armv6t2 -marm
endif

ifndef ALLOW_WARNINGS
  ifneq ($(UNAME_S),Darwin)
    CFLAGS += -Werror
  endif
endif
ifndef DEPRECATED_WARNINGS
  CFLAGS +=  -Wno-deprecated-declarations
endif


default: loose_eel

nseel-compiler.o: glue*.h ns-eel*.h
nseel-cfunc.o:  asm*.c ns-eel*.h
loose_eel.o: eel*.h ns-eel*.h
nseel-*.o: ns-eel*.h

vpath %.cpp ../lice ../swell
vpath %.mm ../swell
vpath %.c ../

ifdef MAXLOOP
  CFLAGS += -DNSEEL_LOOPFUNC_SUPPORT_MAXLEN=$(MAXLOOP)
else
  CFLAGS += -DNSEEL_LOOPFUNC_SUPPORT_MAXLEN=0
endif

ifndef NO_GFX
    OBJS += lice.o lice_image.o lice_line.o lice_ico.o lice_bmp.o lice_textnew.o lice_text.o lice_arc.o
    CFLAGS += -DEEL_LICE_WANT_STANDALONE 

  ifeq ($(UNAME_S),Darwin)
    CFLAGS += -mmacosx-version-min=10.5
    OBJS += swell-wnd.o swell-gdi.o swell.o swell-misc.o swell-dlg.o swell-menu.o swell-kb.o
    LFLAGS += -lobjc -framework Cocoa -framework Carbon
  else

    CFLAGS += -DSWELL_LICE_GDI
    ifdef GDK2
      CFLAGS += -DSWELL_TARGET_GDK=2 $(shell pkg-config --cflags gdk-2.0)
      LFLAGS += $(shell pkg-config --libs gdk-2.0) -lX11 -lXi
    else
      CFLAGS += -DSWELL_TARGET_GDK=3 $(shell pkg-config --cflags gdk-3.0)
      LFLAGS += $(shell pkg-config --libs gdk-3.0) -lX11 -lXi
    endif
    ifndef NOFREETYPE
      CFLAGS += -DSWELL_FREETYPE $(shell freetype-config --cflags)
      LFLAGS += $(shell freetype-config --libs)
    endif

    OBJS += swell-wnd-generic.o swell-gdi-lice.o swell.o swell-misc-generic.o \
            swell-dlg-generic.o swell-menu-generic.o swell-kb-generic.o \
            swell-gdi-generic.o swell-ini.o swell-generic-gdk.o

    LFLAGS += -ldl
  endif

endif


asm-nseel-x64.o: a2x64.php asm-nseel-x86-gcc.c
	php a2x64.php elf64

ifdef PORTABLE
  CFLAGS +=  -DEEL_TARGET_PORTABLE
else
  ifeq ($(UNAME_S),Darwin)
    ifeq ($(ARCH),x86_64)
      OBJS2 += asm-nseel-x64-macho.o
    endif
  endif
  ifeq ($(UNAME_S),Linux)
    ifeq ($(ARCH),x86_64)
      OBJS2 += asm-nseel-x64.o
    endif
  endif
endif
CXXFLAGS=$(CFLAGS)


%.o : %.mm
	$(CXX) $(CXXFLAGS) -c -o $@ $^

loose_eel: loose_eel.o $(OBJS) $(OBJS2)
	g++ -o $@ $^ $(CXXFLAGS) $(LFLAGS)

clean:
	-rm loose_eel loose_eel.o $(OBJS)
