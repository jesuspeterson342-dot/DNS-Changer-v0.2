#!/bin/bash

# DNS Changer Installation Script
# This script compiles and installs the DNS Changer utility

set -e

echo "==================================="
echo "DNS Changer Installation Script"
echo "==================================="
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Error: This script must be run as root (use sudo)"
    exit 1
fi

# Check for g++ compiler
if ! command -v g++ &> /dev/null; then
    echo "Error: g++ compiler not found. Please install it first."
    echo "  Ubuntu/Debian: sudo apt install g++"
    echo "  Fedora/RHEL:   sudo dnf install gcc-c++"
    echo "  Arch:          sudo pacman -S gcc"
    exit 1
fi

# Compile the program
echo "Compiling DNS Changer..."
g++ -std=c++17 -O2 src/main.cpp -o dnschanger

if [ $? -ne 0 ]; then
    echo "Error: Compilation failed"
    exit 1
fi

echo "✓ Compilation successful"

# Install the binary
echo "Installing CLI to /usr/local/bin/dnschanger..."
cp dnschanger /usr/local/bin/dnschanger
chmod 755 /usr/local/bin/dnschanger

# Install GUI
echo "Installing GUI to /usr/local/share/dnschanger..."
mkdir -p /usr/local/share/dnschanger
cp -r gui/* /usr/local/share/dnschanger/

# Install desktop entry (optional)
if [ -d "/usr/share/applications" ]; then
    echo "Installing desktop entry..."
    sed 's|/usr/local/share/dnschanger/gui.py|/usr/local/share/dnschanger/gui.py|g' dnschanger.desktop > /usr/share/applications/dnschanger.desktop
    chmod 644 /usr/share/applications/dnschanger.desktop
    echo "✓ Desktop entry installed"
fi

echo ""
echo "✓ Installation complete!"
echo ""
echo "CLI Usage:"
echo "  sudo dnschanger set 8.8.8.8"
echo "  sudo dnschanger restore"
echo "  sudo dnschanger status"
echo ""
echo "GUI Usage:"
echo "  Install Python dependencies first:"
echo "    pip3 install -r /usr/local/share/dnschanger/requirements.txt"
echo "  Then run:"
echo "    sudo python3 /usr/local/share/dnschanger/gui.py"
echo "  Or launch 'DNS Changer' from your application menu"
echo ""
echo "To uninstall, run: sudo make uninstall"
