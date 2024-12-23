CC = g++
CFLAGS = -Iinclude -Llib
LIBS = -lraylib
SRC = src/nim.cpp
OUT = nim
INSTALL_DIR = /usr/local/bin
ASSETS_DIR = /usr/share/nim

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LIBS)

install: all
	mkdir -p $(ASSETS_DIR)
	cp $(OUT) $(INSTALL_DIR)/$(OUT)
	cp assets/* $(ASSETS_DIR)
	chmod +x $(INSTALL_DIR)/$(OUT)

uninstall:
	rm -f $(INSTALL_DIR)/$(OUT)
	rm -rf $(ASSETS_DIR)

clean:
	rm -f $(OUT)
