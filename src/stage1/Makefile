PROGRAM ?= cruc.stage1

OCAMLOPT ?= ocamlopt
OCAMLDEP ?= ocamldep
OCAMLFLAGS ?= -O2
OCAMLDEPFLAGS ?=
LDFLAGS ?=
LDLIBS ?=

# Override SOURCES when the project needs a deliberate module list.
SOURCES ?= $(filter-out test_%.ml,$(sort $(wildcard *.ml)))
DEPENDENCIES ?= .depend

# ocamldep determines an order suitable for native linking.
NATIVE_SOURCES := $(shell test -z "$(SOURCES)" || $(OCAMLDEP) -sort $(OCAMLDEPFLAGS) $(SOURCES))
OBJECTS := $(NATIVE_SOURCES:.ml=.cmx)
COMMON_TEST_SOURCES := $(filter-out main.ml,$(SOURCES))
TEST_LOWER_SOURCES := $(COMMON_TEST_SOURCES) test_lower.ml
TEST_LOWER_NATIVE_SOURCES := $(shell $(OCAMLDEP) -sort $(OCAMLDEPFLAGS) $(TEST_LOWER_SOURCES))
TEST_LOWER_OBJECTS := $(TEST_LOWER_NATIVE_SOURCES:.ml=.cmx)
TEST_X86_SOURCES := $(COMMON_TEST_SOURCES) test_x86.ml
TEST_X86_NATIVE_SOURCES := $(shell $(OCAMLDEP) -sort $(OCAMLDEPFLAGS) $(TEST_X86_SOURCES))
TEST_X86_OBJECTS := $(TEST_X86_NATIVE_SOURCES:.ml=.cmx)

.PHONY: all clean rebuild test

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	@test -n "$(OBJECTS)" || { echo "error: no OCaml source files found" >&2; exit 1; }
	$(OCAMLOPT) $(LDFLAGS) -o $@ $^ $(LDLIBS)

test: $(PROGRAM) test_lower test_x86
	./test_lower
	./test_x86
	$(CC) -o test_x86_asm test_x86.s test_x86_runtime.c
	timeout 10s ./test_x86_asm; test $$? -eq 24
	$(CC) -o test_array_asm test_array.s
	timeout 10s ./test_array_asm; test $$? -eq 6
	$(CC) -o test_pointer_asm test_pointer.s
	timeout 10s ./test_pointer_asm; test $$? -eq 22
	$(CC) -o test_cast_asm test_cast.s
	timeout 10s ./test_cast_asm; test $$? -eq 2
	$(CC) -o test_struct_asm test_struct.s
	timeout 10s ./test_struct_asm; test $$? -eq 0
	$(CC) -o test_member_asm test_member.s
	timeout 10s ./test_member_asm; test $$? -eq 14
	sh ./test_driver.sh ./$(PROGRAM)

test_lower: $(TEST_LOWER_OBJECTS)
	$(OCAMLOPT) $(LDFLAGS) -o $@ $^ $(LDLIBS)

test_x86: $(TEST_X86_OBJECTS)
	$(OCAMLOPT) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.cmx: %.ml
	$(OCAMLOPT) $(OCAMLFLAGS) -c $<

ifneq ($(strip $(SOURCES)),)
$(DEPENDENCIES): $(SOURCES) test_lower.ml test_x86.ml
	$(OCAMLDEP) -native $(OCAMLDEPFLAGS) $(SOURCES) test_lower.ml test_x86.ml > $@
endif

ifeq ($(filter clean rebuild,$(MAKECMDGOALS)),)
-include $(DEPENDENCIES)
endif

clean:
	rm -f $(PROGRAM) test_lower test_x86 test_x86.s test_x86_asm test_array.s test_array_asm test_pointer.s test_pointer_asm test_cast.s test_cast_asm test_struct.s test_struct_asm test_member.s test_member_asm $(DEPENDENCIES) $(SOURCES:.ml=.cmi) $(SOURCES:.ml=.cmx) $(SOURCES:.ml=.o) test_lower.cmi test_lower.cmx test_lower.o test_x86.cmi test_x86.cmx test_x86.o

rebuild: clean
	$(MAKE) all
