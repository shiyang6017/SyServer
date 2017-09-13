TARGET = SyServer

LIBS = $(wildcard ./base/*.c)

SRCS =  $(LIBS) config.c connection.c httpparse.c request.c response.c server.c uwsgi.c 

OBJS_DIR = ./base/
OBJS = $(addprefix $(OBJS_DIR), $(SRCS:.c=.o))

#BIN_DIR = prog/
BIN_DIR = /usr/local/bin/

#INSTALL_DIR = prog/
INSTALL_DIR = /usr/local/$(TARGET)/

CC = gcc -std=c11

CFLAGS = -g -Wall -DINSTALL_DIR=\"$(INSTALL_DIR)\" -D_XOPEN_SOURCE -D_GNU_SOURCE -I./ -I../
#CFLAGS =  -O2 -Wall -DINSTALL_DIR=\"$(INSTALL_DIR)\" -D_XOPEN_SOURCE -D_GNU_SOURCE -I./ -I../

all:
	mkdir -p $(OBJS_DIR)lib
	make $(TARGET)

install: all
	sudo mkdir -p $(INSTALL_DIR)
	sudo cp ../config.json $(INSTALL_DIR)
	sudo cp -a ../html/ $(INSTALL_DIR)
	sudo mv $(TARGET) $(BIN_DIR)$(TARGET)

uninstall: clean
	sudo -rm -f $(BIN_DIR)$(TARGET)
	sudo -rm -rf $(INSTALL_DIR)

$(TARGET): $(OBJS)
	gcc -o $@ $^
	
$(OBJS_DIR)%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean

clean:
	-rm -rf $(OBJS_DIR)
