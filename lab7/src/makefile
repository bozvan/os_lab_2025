TARGETS = tcpclient tcpserver udpclient udpserver

all: $(TARGETS)

$(TARGETS): %: %.c
	gcc -Wall -Wextra -std=c99 -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean
