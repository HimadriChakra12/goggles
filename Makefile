CFLAGS = -std=c99 -Wall -Wextra -pedantic -O2
NAMES  = $(patsubst %/gen.c,%,$(wildcard */gen.c))

.SECONDEXPANSION:
.DELETE_ON_ERROR:

all: $(NAMES:=/)

$(NAMES:=/): %/: %/$$*.goggle

$(NAMES): %: %/

%.goggle: $$(dir $$@)gen
	$< > $@

%/gen: %/gen.c tools/gen.h
	$(CC) $(CFLAGS) -I tools -o $@ $<

clean:
	rm -f $(foreach n,$(NAMES),$n/gen $n/$n.goggle)

.PHONY: all clean $(NAMES) $(NAMES:=/)
