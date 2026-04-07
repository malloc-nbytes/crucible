bin_PROGRAMS = cruc cruc-debug-build

cruc_SOURCES = main.c io.c mem.c loc.c sv.c lexer.c token.c kw.c
cruc_CFLAGS = -O2 -march=native -pipe -I$(top_srcdir)/src/include -Wextra -Wall
cruc_LDADD = 

cruc_debug_build_SOURCES = $(cruc_SOURCES)
cruc_debug_build_CFLAGS = -g -O0 -I$(top_srcdir)/src/include
cruc_debug_build_LDADD = 

debug: cruc-debug-build
