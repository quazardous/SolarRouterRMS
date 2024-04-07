#!/bin/bash

# Makefile for Solar_Router project using Arduino IDE 2.x / arduino-cli

include ./.env.local

# HELP
.DEFAULT_GOAL := help
help:
	@grep -E -h '##' $(MAKEFILE_LIST) | grep -v MAKEFILE_LIST | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m  %-30s\033[0m %s\n", $$1, $$2}'
.PHONY: help

# FUNCTIONS
define dist_copy
	@if [[ -f $(1) ]]; then \
		echo '$(1) already exists'; \
	else \
		echo cp $(1)-dist $(1); \
		cp $(1)-dist $(1); \
	fi
endef

# Arduino-cli Makefile
# Set the Arduino-cli executable path
ARDUINO_CLI = arduino-cli --config-file "$(CONFIG_FILE)"

# Set the sketch file
SKETCH = SolarRouterRMS.ino

# Set the build directory
BUILD_DIR = ./build/

ESPRESSIF_BOARDS_URL = https://dl.espressif.com/dl/package_esp32_index.json

# Set the Arduino-cli commands
COMPILE_CMD = $(ARDUINO_CLI) compile -v --fqbn $(BOARD) --build-path $(BUILD_DIR) -t $(SKETCH)
UPLOAD_CMD = $(ARDUINO_CLI) upload -v -p $(PORT) --fqbn $(BOARD) --input-dir $(BUILD_DIR)

setup: ## Setup the Arduino-cli environment
setup: env
	$(ARDUINO_CLI) config add board_manager.additional_urls $(ESPRESSIF_BOARDS_URL)
	$(ARDUINO_CLI) core download esp32:esp32
	$(ARDUINO_CLI) core install esp32:esp32
	$(ARDUINO_CLI) lib install RemoteDebug2
	$(ARDUINO_CLI) lib install PubSubClient
	$(ARDUINO_CLI) lib install OneWire
	$(ARDUINO_CLI) lib install DallasTemperature
	$(ARDUINO_CLI) lib install ArduinoJson
	$(ARDUINO_CLI) lib install UrlEncode

# Default target
all: compile

compile-pages: ## Compile the web pages into C++ code
compile-pages:
	python3 tools/gen-pages.py ./pages/ ./src/pages/

compile: ## Compile the sketch
compile: compile-pages
	$(COMPILE_CMD)

upload: ## Upload the sketch to the board
	$(UPLOAD_CMD)

clean: ## Clean the build directory
	rm -rf $(BUILD_DIR)

env: .env.local

.env.local: .env.local-dist
	$(call dist_copy,.env.local)

