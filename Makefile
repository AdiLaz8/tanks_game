# Root Makefile for Tanks Game Project
# This orchestrates the building of all components in the correct order

# Default target
all: algorithm gamemanager simulator

# Build Algorithm shared library
algorithm:
	@echo "=== Building Algorithm ==="
	$(MAKE) -C Algorithm

# Build GameManager shared library
gamemanager:
	@echo "=== Building GameManager ==="
	$(MAKE) -C GameManager

# Build Simulator executables
simulator:
	@echo "=== Building Simulator ==="
	$(MAKE) -C Simulator

# Clean all components
clean:
	@echo "=== Cleaning all components ==="
	$(MAKE) -C Algorithm clean
	$(MAKE) -C GameManager clean
	$(MAKE) -C Simulator clean
	rm -f UserCommon/*.o

# Install simulator to root directory
install: simulator
	@echo "=== Installing simulator to root ==="
	$(MAKE) -C Simulator install

# Test targets
test: all
	@echo "=== Running tests ==="
	$(MAKE) -C Simulator test

quick:
	@echo "=== Quick parallel build ==="
	$(MAKE) -j2 algorithm gamemanager
	$(MAKE) simulator

rebuild: clean all

# Show build status
status:
	@echo "=== Build Status ==="
	@echo "Algorithm shared library:"
	@ls -la Algorithm/*.so 2>/dev/null || echo "  No Algorithm .so found"
	@echo "GameManager shared library:"
	@ls -la GameManager/*.so 2>/dev/null || echo "  No GameManager .so found"
	@echo "Simulator executables:"
	@ls -la Simulator/simulator_* 2>/dev/null || echo "  No Simulator executables found"

# Help target
help:
	@echo "Available targets:"
	@echo "  all        - Build all components (default)"
	@echo "  algorithm  - Build Algorithm shared library"
	@echo "  gamemanager- Build GameManager shared library"
	@echo "  simulator  - Build Simulator executables"
	@echo "  clean      - Clean all build artifacts"
	@echo "  install    - Install simulator to root directory"
	@echo "  test       - Run tests"
	@echo "  quick      - Quick parallel build"
	@echo "  rebuild    - Clean and rebuild everything"
	@echo "  status     - Show current build status"
	@echo "  help       - Show this help message"

.PHONY: all algorithm gamemanager simulator clean install test quick rebuild status help