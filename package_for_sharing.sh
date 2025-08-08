#!/bin/bash

# Script to package .so files for sharing with other students
# Usage: ./package_for_sharing.sh

echo "=== Packaging .so files for sharing ==="

# Create a directory for sharing
SHARE_DIR="shared_libraries_318772340_206580102"
mkdir -p "$SHARE_DIR"

# Copy the .so files
echo "Copying GameManager library..."
cp ../GameManager/GameManager_318772340_206580102.so "$SHARE_DIR/"

echo "Copying Algorithm library..."
cp ../Algorithm/Algorithm_318772340_206580102.so "$SHARE_DIR/"

# Create a README file with instructions
cat > "$SHARE_DIR/README.txt" << 'EOF'
SHARED LIBRARIES FOR TANK GAME SIMULATOR
========================================

Student IDs: 318772340_206580102

Files included:
- GameManager_318772340_206580102.so (GameManager implementation)
- Algorithm_318772340_206580102.so (Player and TankAlgorithm implementation)

Instructions for other students:
1. Place these .so files in your project's appropriate directories
2. Use them with the comparative simulator:
   ./simulator_318772340_206580102 -comparative game_map=<map_file> game_managers_folder=<folder> algorithm1=Algorithm_318772340_206580102.so algorithm2=Algorithm_318772340_206580102.so

3. For competitive mode (when implemented):
   ./simulator_318772340_206580102 -competitive game_managers=GameManager_318772340_206580102.so algorithms=Algorithm_318772340_206580102.so maps=<map_files>

Note: These libraries were built on macOS and may need to be rebuilt on Linux for compatibility.
EOF

# Create a tar.gz archive for easy sharing
echo "Creating archive..."
tar -czf "${SHARE_DIR}.tar.gz" "$SHARE_DIR"

echo ""
echo "=== Files ready for sharing ==="
echo "📁 Directory: $SHARE_DIR/"
echo "📦 Archive: ${SHARE_DIR}.tar.gz"
echo ""
echo "Files included:"
ls -la "$SHARE_DIR/"
echo ""
echo "You can now share the '$SHARE_DIR' folder or '${SHARE_DIR}.tar.gz' archive with other students!"
echo ""
echo "To extract the archive on another system:"
echo "  tar -xzf ${SHARE_DIR}.tar.gz" 