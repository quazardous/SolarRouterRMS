#!/bin/bash

# Makefile for Solar_Router project using Arduino IDE 2.x / arduino-cli

include ./.env.local

VERSION = $(shell cat ./VERSION)

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
setup: env patch-lib
	$(ARDUINO_CLI) config add board_manager.additional_urls $(ESPRESSIF_BOARDS_URL)
	$(ARDUINO_CLI) core download esp32:esp32
	$(ARDUINO_CLI) core install esp32:esp32

patch-lib: setup-lib
	sed -i 's/#define ELEGANTOTA_USE_ASYNC_WEBSERVER 0/#define ELEGANTOTA_USE_ASYNC_WEBSERVER 1/' $(ARDUINO_LIBRARIES)/ElegantOTA/src/ElegantOTA.h
	sed -i 's/#define CONFIG_ASYNC_TCP_RUNNING_CORE -1/#define CONFIG_ASYNC_TCP_RUNNING_CORE 1/' $(ARDUINO_LIBRARIES)/AsyncTCP/src/AsyncTCP.h

setup-lib: ## Install the required libraries
	$(ARDUINO_CLI) lib install RemoteDebug2
	$(ARDUINO_CLI) lib install PubSubClient
	$(ARDUINO_CLI) lib install OneWire
	$(ARDUINO_CLI) lib install DallasTemperature
	$(ARDUINO_CLI) lib install ArduinoJson
	$(ARDUINO_CLI) lib install UrlEncode
	$(ARDUINO_CLI) lib install Hashtable
	$(ARDUINO_CLI) lib install ArrayList
	$(ARDUINO_CLI) lib install AsyncTCP
	$(ARDUINO_CLI) lib install "ESP Async WebServer"
	$(ARDUINO_CLI) lib install ElegantOTA

arduino-update: ## Update the Arduino-cli
	$(ARDUINO_CLI) update

arduino-config-dump: ## Update the Arduino-cli
	$(ARDUINO_CLI) config dump

upgrade-lib: ## Upgrade the installed libraries
	$(ARDUINO_CLI) lib upgrade

setup-web: ## Setup the web pages
setup-web: ./web/pages/lib/simple.css ./web/lib/simple.min.css ./web/pages/lib/reef.js ./web/lib/reef.min.js

./web/pages/lib/simple.css:
	curl https://cdn.simplecss.org/simple.css --output ./web/pages/lib/simple.css

./web/lib/simple.min.css:
	curl https://cdn.simplecss.org/simple.min.css --output ./web/lib/simple.min.css

./web/pages/lib/reef.js:
	curl https://cdn.jsdelivr.net/npm/reefjs@13.0.2/dist/reef.js --output ./web/pages/lib/reef.js

./web/lib/reef.min.js:
	curl https://cdn.jsdelivr.net/npm/reefjs@13.0.2/dist/reef.min.js --output ./web/lib/reef.min.js

dev-server: ## Start the development server	
	http-server ./web/pages/ -p 7000 --cors

light-server: ## Start the development server	
	light-server -s ./web/pages/ -p 7000

# Default target
all: compile

compile-pages: ## Build the web pages into C++ code
compile-pages: setup-web
	- rm -f ./web/build/*.css
	- rm -f ./web/build/*.js
	- rm -f ./web/build/*.html
	- cp ./web/lib/*.js ./web/build/
	- cp ./web/lib/*.css ./web/build/
	- find ./web/pages -maxdepth 1 -type f -name '*.html' -exec sh -c 'html-minifier --collapse-whitespace --remove-comments --remove-optional-tags --remove-redundant-attributes --remove-script-type-attributes --remove-tag-whitespace --use-short-doctype --minify-css true --minify-js true "$$0" -o "./web/build/$$(basename "$$0" .html).html"' {} \;
	- find ./web/pages/js -maxdepth 1 -type f -name '*.js' -exec sh -c 'uglifyjs "$$0" -o "./web/build/$$(basename "$$0" .js).js"' {} \;
	- find ./web/pages/css -maxdepth 1 -type f -name '*.css' -exec sh -c 'uglifycss "$$0" --output "./web/build/$$(basename "$$0" .css).css"' {} \;
	- python3 tools/gen-pages.py ./web/build/ ./src/pages/

compile: ## Build the sketch
	$(COMPILE_CMD)

upload: ## Upload the sketch to the board
	$(UPLOAD_CMD)

clean: ## Clean the build directory
	rm -rf $(BUILD_DIR)

monitor: ## Monitor the serial port
	$(ARDUINO_CLI) monitor -p $(PORT) -c baudrate=115200

env: .env.local

.env.local: .env.local-dist
	$(call dist_copy,.env.local)

zip: ## Create a zip file with the sketch
	rm -f ../SolarRouterRMS-$(VERSION).zip
	cd .. && zip -r ./SolarRouterRMS-$(VERSION).zip ./SolarRouterRMS -x ./SolarRouterRMS/.git/\* -x .env.local -x ./SolarRouterRMS/build/\* -x ./SolarRouterRMS/.vscode/\*

