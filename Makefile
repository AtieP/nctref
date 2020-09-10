rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

SOURCES := $(call rwildcard,src/,*.c)
HEADERS := $(call rwildcard,src/,*.h)

.PHONY: install clean

ntc: $(SOURCES) $(HEADERS)
	cc -o ntc -std=gnu99 -Og -g -fms-extensions -Isrc $(SOURCES)

install: ntc
	mv ./ntc /usr/local/bin

clean:
	rm ./ntc