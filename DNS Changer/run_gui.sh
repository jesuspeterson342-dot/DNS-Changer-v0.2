#!/bin/bash

# DNS Changer GUI Launcher
# This script properly launches the GUI with root privileges and X11 access

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=========================================="
echo "DNS Changer GUI Launcher"
echo "=========================================="
echo ""

# Check if running as root
if [ "$EUID" -eq 0 ]; then
    echo -e "${RED}Error: Do not run this script with sudo!${NC}"
    echo "This script will request sudo when needed."
    exit 1
fi

# Check if Python 3 is installed
if ! command -v python3 &> /dev/null; then
    echo -e "${RED}Error: Python 3 is not installed${NC}"
    exit 1
fi

# Check if virtual environment exists
if [ -d "venv" ]; then
    echo -e "${GREEN}✓ Virtual environment found${NC}"
    PYTHON_CMD="venv/bin/python3"

    # Check if customtkinter is installed in venv
    if ! $PYTHON_CMD -c "import customtkinter" 2>/dev/null; then
        echo -e "${YELLOW}Installing customtkinter in virtual environment...${NC}"
        venv/bin/pip install customtkinter
    fi
else
    echo -e "${YELLOW}No virtual environment found, using system Python${NC}"
    PYTHON_CMD="python3"

    # Check if customtkinter is installed
    if ! $PYTHON_CMD -c "import customtkinter" 2>/dev/null; then
        echo -e "${RED}Error: customtkinter is not installed${NC}"
        echo ""
        echo "Please install it:"
        echo "  pip3 install customtkinter"
        echo ""
        echo "Or create a virtual environment:"
        echo "  python3 -m venv venv"
        echo "  source venv/bin/activate"
        echo "  pip install customtkinter"
        exit 1
    fi
fi

# Allow root to access X11 display
echo -e "${YELLOW}Granting X11 access to root...${NC}"
xhost +si:localuser:root > /dev/null 2>&1

# Launch GUI with sudo, preserving environment
echo -e "${GREEN}Launching DNS Changer GUI...${NC}"
echo ""

sudo -E env PATH="$PATH" DISPLAY="$DISPLAY" XAUTHORITY="$XAUTHORITY" $PYTHON_CMD gui/gui.py

# Revoke X11 access from root (cleanup)
echo ""
echo -e "${YELLOW}Revoking X11 access from root...${NC}"
xhost -si:localuser:root > /dev/null 2>&1

echo -e "${GREEN}Done.${NC}"
