# Changelog

All notable changes to this project will be documented in this file.

## [v0.5] - 2024-04-30

### Fix
- Fix multi process starvation (loop vs webserver)

### Refactor
- Refactor of reboot event
- Configs admin wired with API Configs

### New paradigm
- Config update is/should be bootless
- Updating config should not be persisted (EEPROM) by default
- Loopcentric / non blocking: modules should not use delay() or such patterns to process stuff 

## [v0.4] - 2024-04-30

### Refactor
- New DHTML admin
- Mode Offshore for dev (local admin can use RMS API)
- Start refactor of configs

## [v0.3] - 2024-04-24

### Refactor
- Refactor WiFi to be more async friendly (non blocking)
- Use Async Web Server (web+ota)
- Add uglifyjs/uglifycss/html_minifier to the web pages precompilation

## [v0.2] - 2024-04-18

### Refactor
- Split code in modules using namespaces
- Create mapped setter/getter

## [v0.1] - 2024-04-07

### Refactor
- Added a script to compile html/js into C++ code


