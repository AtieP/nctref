rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

SOURCES := $(call rwildcard,src/,*.c)
HEADERS := $(call rwildcard,src/,*.h)

PREFIX = /usr/local

.PHONY: install clean

ntc: $(SOURCES) $(HEADERS)
ifdef OW
	wcl $(if $(GAS),-DSYNTAX_GAS=1,) -fe="ntc.exe" -0 -bcl=dos -mt -d0 -os -om -ob -oi -ol -ox -lr -za99 -i=src $(SOURCES)
else
	cc $(if $(GAS),-DSYNTAX_GAS,) -Wall -o ntc -std=gnu99 -Os -g -fms-extensions -Isrc $(SOURCES)
endif

install: ntc
	mv ./ntc $(PREFIX)/bin

clean:
	rm ./ntc
