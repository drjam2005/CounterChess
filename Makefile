.DEFAULT_GOAL := all
.PHONY: all network client server clean

CC = gcc
CFLAGS += -O0 -pthread
LDFLAGS += -Lvendor/X11/lib -Lvendor/raylib/lib -Wl,-rpath,./vendor/X11/lib
LDLIBS += -l:libraylib.a -lX11 -lm -ldl -pthread

ifdef RELEASE
MODE := release
else
MODE := reload
CFLAGS += -rdynamic -DRELOAD
endif
OBJDIR := build/$(MODE)

COMMON_SRC := src/my_structs.c src/helpers.c src/packets.c \
              src/chess/chess_board.c src/chess/chess_moves.c
CLIENT_SRC := $(COMMON_SRC) src/client.c src/plug/plug_client.c
SERVER_SRC := $(COMMON_SRC) src/server.c src/plug/plug_server.c
ifdef RELEASE
CLIENT_SRC += src/render_assets.c
SERVER_SRC += src/handle_game.c
else
CLIENT_LIB := build/librender_assets.so
SERVER_LIB := build/libhandle_game.so
endif
CLIENT_OBJ := $(patsubst src/%.c,$(OBJDIR)/%.o,$(CLIENT_SRC))
SERVER_OBJ := $(patsubst src/%.c,$(OBJDIR)/%.o,$(SERVER_SRC))

all: network
network: client server
client: build/client
server: build/server

# Relink when switching modes, even when the other mode's objects are older.
.PHONY: FORCE
FORCE:

build/client: $(CLIENT_OBJ) vendor/alpha/piece_assets.o $(CLIENT_LIB) FORCE
	$(CC) $(CFLAGS) $(LDFLAGS) $(CLIENT_OBJ) vendor/alpha/piece_assets.o -o $@ $(LDLIBS)

build/server: $(SERVER_OBJ) $(SERVER_LIB) FORCE
	$(CC) $(CFLAGS) $(LDFLAGS) $(SERVER_OBJ) -o $@ $(LDLIBS)

$(OBJDIR)/%.o: src/%.c Makefile
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

build/librender_assets.so: src/render_assets.c Makefile
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -MF build/librender_assets.d -shared -fPIC $< -o $@

build/libhandle_game.so: src/handle_game.c Makefile
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -MF build/libhandle_game.d -shared -fPIC $< -o $@

-include $(CLIENT_OBJ:.o=.d) $(SERVER_OBJ:.o=.d) \
         build/librender_assets.d build/libhandle_game.d

clean:
	rm -rf build/
