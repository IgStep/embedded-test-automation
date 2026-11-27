FW_BUILD    ?= build-fw
RENODE_TEST ?= renode-test

fw: ## Build the firmware: led_seq.elf and .bin in FW_BUILD (default build-fw)
	cmake -B $(FW_BUILD) -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake
	cmake --build $(FW_BUILD)

unit: ## Run the unit tests
	$(MAKE) -C tests/unit test

coverage: ## Run the unit tests and write a coverage report (tests/unit/build/coverage/index.html)
	$(MAKE) -C tests/unit coverage

host: ## Run the system tests on the PC simulation
	cd tests/system && python3 -m pytest --target host --junitxml=report-host.xml

sil: fw ## Run the SIL tests in Renode (report: sil-results/report.html)
	$(RENODE_TEST) tests/sil/led_seq.robot --variable ELF:$(CURDIR)/$(FW_BUILD)/led_seq.elf --results-dir sil-results

hil: ## Flash the board and run the system tests on real hardware (run on the Pi)
	cd tests/system && python3 -m pytest --target pi --fw ../../$(FW_BUILD)/led_seq.bin --junitxml=report-hil.xml

help: ## Show this list of targets
	@grep -E '^[a-zA-Z_-]+:.*## ' $(firstword $(MAKEFILE_LIST)) | \
		awk 'BEGIN {FS = ":.*## "}; {printf "  %-10s %s\n", $$1, $$2}'

.PHONY: fw unit coverage host sil hil help
