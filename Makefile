CC ?= gcc
CFLAGS ?= -Wall -Wextra -Werror -O2

SRC_DIR := src
TARGET := maxint

SRC := $(SRC_DIR)/main.c

.PHONY: all check clean package

all: check $(TARGET)

check:
	@command -v $(CC) >/dev/null 2>&1 || { echo "Error: $(CC) not found"; exit 1; }

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC)

package: check
	@command -v dpkg-buildpackage >/dev/null 2>&1 || { echo "Error: dpkg-buildpackage not found"; exit 1; }
	@command -v dpkg-checkbuilddeps >/dev/null 2>&1 || { echo "Error: dpkg-checkbuilddeps not found"; exit 1; }
	dpkg-checkbuilddeps
	dpkg-buildpackage -us -uc -b

clean:
	rm -f $(TARGET) *.deb ../*.deb ../*.buildinfo ../*.changes ../*.dsc ../*.tar.*
	rm -rf bin
