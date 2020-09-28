rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

SOURCES := $(call rwildcard,src/,*.c)
HEADERS := $(call rwildcard,src/,*.h)

PREFIX = /usr/local

.PHONY: install clean

ntc: $(SOURCES) $(HEADERS)
	cc $(if $(GAS),-DSYNTAX_GAS,) -Wall -o ntc -std=gnu99 -O0 -g -fms-extensions -Isrc $(SOURCES)

install: ntc
	mv ./ntc $(PREFIX)/bin

clean:
	rm ./ntc
