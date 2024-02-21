.PHONY: all clean test install uninstall check test_make

INSTALL_DIR = ~/.local/

all: bin
	@scons -Q

lib:
	@mkdir -p lib

bin:
	@mkdir -p bin

test:
	@cd bin && ./rc

clean:
	@scons -c -Q
	@rm -rf bin/ lib/ .sconsign.dblite py/__pycache__

install: all
	@cp bin/post $(INSTALL_DIR)/bin/
	@cp -r py $(INSTALL_DIR)/lib/post_ana
	@cp lib/libpost.so $(INSTALL_DIR)/lib/post_ana
	@cp lib/libpost.so $(INSTALL_DIR)/lib/

uninstall:
	@rm -f $(INSTALL_DIR)/bin/post
	@rm -rf $(INSTALL_DIR)/lib/post_ana $(INSTALL_DIR)/lib/libpost.so
