FW_BUILD    ?= build-fw

fw:
	cmake -B $(FW_BUILD) -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake
	cmake --build $(FW_BUILD)

unit:
	$(MAKE) -C tests/unit test

coverage:
	$(MAKE) -C tests/unit coverage

host:
	cd tests/system && python3 -m pytest --target host --junitxml=report-host.xml

RENODE_TEST ?= renode-test

sil: fw
	$(RENODE_TEST) tests/sil/led_seq.robot --variable ELF:$(CURDIR)/$(FW_BUILD)/led_seq.elf --results-dir sil-results

hil:
	cd tests/system && python3 -m pytest --target pi --fw ../../$(FW_BUILD)/led_seq.bin --junitxml=report-hil.xml

.PHONY: fw unit coverage