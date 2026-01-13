bin_PROGRAMS = cruc cruc-debug-build

cruc_SOURCES = main.c loc.c
cruc_CFLAGS = -O2 -I$(top_srcdir)/src/include
cruc_LDADD = -lforge

cruc_debug_build_SOURCES = $(cruc_SOURCES)
cruc_debug_build_CFLAGS = -g -O0 -I$(top_srcdir)/src/include
cruc_debug_build_LDADD = -lforge

debug: cruc-debug-build
