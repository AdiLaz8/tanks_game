# Root Makefile for Tanks Game Project
# This orchestrates the building of all components in the correct order

# Default target
all: usercommon algorithm gamemanager simulator

# Build UserCommon components (dependency for others)
usercommon:
	@echo "=== Building UserCommon ==="
	$(MAKE) -C UserCommon

# Build Algorithm shared library (depends on UserCommon)
algorithm: usercommon
	@echo "=== Building Algorithm ==="
	$(MAKE) -C Algorithm

# Build GameManager shared library (depends on UserCommon)
gamemanager: usercommon
	@echo "=== Building GameManager ==="
	$(MAKE) -C GameManager

# Build Simulator executables (depends on UserCommon)
simulator: usercommon
	@echo "=== Building Simulator ==="
	$(MAKE) -C Simulator

# Clean all components
clean:
	@echo "=== Cleaning all components ==="
	$(MAKE) -C UserCommon clean || true
	$(MAKE) -C Algorithm clean
	$(MAKE) -C GameManager clean
	$(MAKE) -C Simulator clean

# Install simulator to root directory
install: simulator
	@echo "=== Installing simulator to root ==="
	$(MAKE) -C Simulator install

# Test targets
test: all
	@echo "=== Running tests ==="
	$(MAKE) -C Simulator test

# Quick build (parallel where possible)
quick: usercommon
	@echo "=== Quick parallel build ==="
	$(MAKE) -j2 algorithm gamemanager
	$(MAKE) simulator

# Rebuild everything from scratch
rebuild: clean all

# Show build status
status:
	@echo "=== Build Status ==="
	@echo "UserCommon objects:"
	@ls -la UserCommon/*.o 2>/dev/null || echo "  No UserCommon objects found"
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
	@echo "  usercommon - Build UserCommon objects"
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

.PHONY: all usercommon algorithm gamemanager simulator clean install test quick rebuild status help