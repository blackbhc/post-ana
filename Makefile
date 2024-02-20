.PHONY: all clean test install uninstall check test_make

INSTALL_DIR = ~/.local/

all: bin
	@scons

lib:
	@mkdir -p lib

bin:
	@mkdir -p bin

test:
	@cd bin && ./rc

clean:
	@scons -c
	@rm -rf bin/ lib/ .sconsign.dblite

install: all
	@cp bin/rc $(INSTALL_DIR)/bin/
	@cp lib/libpostana.so $(INSTALL_DIR)/lib/

uninstall:
	@rm -f $(INSTALL_DIR)/bin/rc
	@rm -f $(INSTALL_DIR)/lib/ligpostana.so
