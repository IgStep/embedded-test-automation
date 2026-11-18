# Unit tests: build and run them on the PC.
#   make            build + run all tests
#   make clean      delete the build folder
#
# Reading a rule:
#   target: things it needs
#   <TAB>command that makes the target
# make only rebuilds a target when one of the things it needs has changed.

CC     = gcc
APP    = ../../nucleo_led/App
CFLAGS = -std=c11 -Wall -Wextra -Werror -g -O0 -I$(APP) -Iunity $(EXTRA)
UNITY  = unity/unity.c

# "make" with no name runs the first target: this one.
test: build/test_led_seq build/test_button
	./build/test_led_seq
	./build/test_button

build/test_led_seq: test_led_seq.c $(APP)/led_seq.c $(APP)/led_seq.h $(UNITY)
	mkdir -p build
	$(CC) $(CFLAGS) test_led_seq.c $(APP)/led_seq.c $(UNITY) -o build/test_led_seq

build/test_button: test_button.c $(APP)/button.c $(APP)/button.h $(UNITY)
	mkdir -p build
	$(CC) $(CFLAGS) test_button.c $(APP)/button.c $(UNITY) -o build/test_button

clean:
	rm -rf build

.PHONY: test clean
