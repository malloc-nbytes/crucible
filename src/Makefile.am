bin_PROGRAMS = cruc cruc-debug-build

cruc_SOURCES = main.c loc.c io.c lex.c mem.c strv.c kwd.c visitor.c grammar.c parse.c err.c utils.c type.c ast.c
cruc_CFLAGS = -O2 -march=native -pipe -I$(top_srcdir)/src/include -Wextra -Wall
cruc_LDADD = 

cruc_debug_build_SOURCES = $(cruc_SOURCES)
cruc_debug_build_CFLAGS = -g -O0 -I$(top_srcdir)/src/include
cruc_debug_build_LDADD = 

debug: cruc-debug-build
