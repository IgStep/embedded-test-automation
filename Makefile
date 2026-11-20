FW_BUILD    ?= build-fw

fw:
	cmake -B $(FW_BUILD) -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake
	cmake --build $(FW_BUILD)

unit:
	$(MAKE) -C tests/unit test

coverage:
	$(MAKE) -C tests/unit coverage

.PHONY: fw unit coverage