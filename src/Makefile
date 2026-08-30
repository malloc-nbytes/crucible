PROGRAM ?= cruc

OCAMLOPT ?= ocamlopt
OCAMLDEP ?= ocamldep
OCAMLFLAGS ?= -O2
OCAMLDEPFLAGS ?=
LDFLAGS ?=
LDLIBS ?=

# Override SOURCES when the project needs a deliberate module list.
SOURCES ?= $(filter-out test_lower.ml,$(sort $(wildcard *.ml)))
TEST_SOURCES := $(filter-out main.ml,$(SOURCES)) test_lower.ml
TEST_NATIVE_SOURCES := $(shell $(OCAMLDEP) -sort $(OCAMLDEPFLAGS) $(TEST_SOURCES))
TEST_OBJECTS := $(TEST_NATIVE_SOURCES:.ml=.cmx)
DEPENDENCIES ?= .depend

# ocamldep determines an order suitable for native linking.
NATIVE_SOURCES := $(shell test -z "$(SOURCES)" || $(OCAMLDEP) -sort $(OCAMLDEPFLAGS) $(SOURCES))
OBJECTS := $(NATIVE_SOURCES:.ml=.cmx)

.PHONY: all clean rebuild test

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	@test -n "$(OBJECTS)" || { echo "error: no OCaml source files found" >&2; exit 1; }
	$(OCAMLOPT) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.cmx: %.ml
	$(OCAMLOPT) $(OCAMLFLAGS) -c $<

ifneq ($(strip $(SOURCES)),)
$(DEPENDENCIES): $(SOURCES) test_lower.ml
	$(OCAMLDEP) -native $(OCAMLDEPFLAGS) $(SOURCES) test_lower.ml > $@
endif

ifeq ($(filter clean rebuild,$(MAKECMDGOALS)),)
-include $(DEPENDENCIES)
endif

clean:
	rm -f $(PROGRAM) test-lower $(DEPENDENCIES) $(SOURCES:.ml=.cmi) $(SOURCES:.ml=.cmx) $(SOURCES:.ml=.o) test_lower.cmi test_lower.cmx test_lower.o

rebuild: clean
	$(MAKE) all

test: test-lower
	./test-lower

test-lower: $(TEST_OBJECTS)
	$(OCAMLOPT) $(LDFLAGS) -o $@ $^ $(LDLIBS)
