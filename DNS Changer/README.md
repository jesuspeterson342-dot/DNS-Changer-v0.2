# DNS Changer

A powerful DNS management utility for Linux systems with both command-line interface (CLI) and graphical user interface (GUI).

## Features

- **Change DNS**: Set a custom DNS server address
- **Restore DNS**: Restore the previous DNS settings from a backup
- **Check Status**: View current DNS configuration
- **Validation**: IP address validation to ensure correct DNS server addresses
- **Backup**: Automatic backup of current DNS settings before changes
- **Multi-Manager Support**: Works with NetworkManager, systemd-resolved, resolvconf, and openresolv
- **GUI Application**: User-friendly graphical interface built with CustomTkinter
- **Quick DNS Presets**: One-click access to popular DNS providers (Cloudflare, Google, Quad9)

## Requirements

### For CLI (Command-Line Interface)
- Linux operating system
- C++17 or later compiler (g++)
- Root/sudo privileges to modify DNS settings

### For GUI (Graphical Interface)
- Python 3.7 or later
- CustomTkinter library (see installation below)

## Installation

### Quick Installation (Recommended)

Run the installation script as root:

```bash
sudo ./install.sh
```

This will compile the program and install it to `/usr/local/bin/dnschanger`.

### Manual Compilation

Compile directly with g++:

```bash
g++ -std=c++17 -O2 src/main.cpp -o dnschanger
sudo cp dnschanger /usr/local/bin/
```

### GUI Setup

Install Python dependencies:

```bash
pip3 install -r gui/requirements.txt
```

Or install manually:

```bash
pip3 install customtkinter
```

## Usage

### Command-Line Interface (CLI)

The program must be run with root privileges:

```bash
sudo dnschanger set <dns_ip>    # Set DNS server
sudo dnschanger restore         # Restore previous DNS
sudo dnschanger status          # Check current DNS
```

### Graphical User Interface (GUI)

Launch the GUI application:

```bash
sudo python3 gui/gui.py
```

The GUI provides:
- **Quick DNS buttons** for popular providers (Cloudflare, Google, Quad9)
- **Custom DNS input** for any DNS server
- **Restore functionality** to revert to previous settings
- **Status checking** to view current DNS configuration
- **Real-time output** showing command execution and results

### Commands

- **`set <dns_ip>`**: Changes the DNS server to the specified IP address
  - Example: `sudo dnschanger set 8.8.8.8`
  - Automatically backs up the current DNS settings to `/etc/resolv.conf.backup`

- **`restore`**: Restores the DNS settings from the backup file
  - Example: `sudo dnschanger restore`
  - Restores from `/etc/resolv.conf.backup`

- **`status`**: Displays current DNS configuration
  - Example: `sudo dnschanger status`

## Examples

### CLI Examples

Change DNS to Google's public DNS:
```bash
sudo dnschanger set 8.8.8.8
```

Change DNS to Cloudflare's DNS:
```bash
sudo dnschanger set 1.1.1.1
```

Change DNS to Quad9:
```bash
sudo dnschanger set 9.9.9.9
```

Check current DNS settings:
```bash
sudo dnschanger status
```

Restore previous DNS settings:
```bash
sudo dnschanger restore
```

### GUI Usage

1. Launch the GUI with `sudo python3 gui/gui.py`
2. Click one of the quick DNS buttons (Cloudflare, Google, Quad9) or enter a custom DNS IP
3. Click "Apply DNS" to set the custom DNS
4. Use "Check current DNS" to verify the configuration
5. Click "Restore DNS" to revert to previous settings

## How It Works

### Network Manager Detection

The program automatically detects which network management system is running on your Linux distribution:

1. **NetworkManager** (nmcli) - Common on Ubuntu, Fedora, and many modern distributions
2. **systemd-resolved** - Used by systemd-based distributions
3. **resolvconf** - Traditional Debian/Ubuntu systems
4. **openresolv** - Alternative resolver management
5. **Fallback** - Direct `/etc/resolv.conf` modification if no manager is detected

### Setting DNS
1. Validates that the user has root privileges
2. Detects the active network manager
3. Validates the IP address format (checks for valid IPv4)
4. Backs up the current DNS configuration to `/etc/resolv.conf.backup`
5. Applies the new DNS settings using the appropriate method for the detected network manager
6. If setting fails, automatically restores the previous configuration

### Restoring DNS
1. Validates that the user has root privileges
2. Detects the active network manager
3. Reads the backup file from `/etc/resolv.conf.backup`
4. Restores the original DNS settings using the appropriate method

### Checking Status
1. Validates that the user has root privileges
2. Displays current DNS configuration based on the detected network manager

## IP Validation

The program validates IPv4 addresses using the following criteria:
- Must have 4 octets separated by dots
- Each octet must be a number between 0 and 255

Examples of valid addresses: `8.8.8.8`, `1.1.1.1`, `192.168.1.1`
Examples of invalid addresses: `256.1.1.1`, `8.8.8`, `invalid.address`

## Project Structure

```
DNS Changer/
├── src/
│   └── main.cpp              # Main C++ source code
├── gui/
│   ├── gui.py                # Python GUI application
│   └── requirements.txt      # Python dependencies
├── build/                    # Compiled binaries (created during build)
├── install.sh                # Installation script
├── .gitignore                # Git ignore rules
├── README.md                 # English documentation
├── README.ru.md              # Russian documentation
└── LICENSE                   # MIT License

```

### Source Code (`/src`)
- `main.cpp` - Main program implementation
  - Network manager detection (NetworkManager, systemd-resolved, resolvconf, openresolv)
  - IP validation functions
  - DNS backup and restore functionality for each network manager
  - Root privilege checking
  - Command-line argument parsing

### GUI (`/gui`)
- `gui.py` - CustomTkinter-based graphical interface
  - Quick DNS preset buttons
  - Custom DNS input with validation
  - Real-time command output display
  - Threaded command execution to prevent UI freezing
- `requirements.txt` - Python package dependencies

### System Files

- `/etc/resolv.conf.backup` - Backup of DNS settings created when running `set` command
- `/usr/local/bin/dnschanger` - Installed binary location (after installation)

## Safety

- Automatic backups are created before modifying DNS settings
- IP address validation prevents invalid entries
- Requires root privileges to prevent unauthorized changes
- Original DNS settings can always be restored

## Popular DNS Servers

- **Cloudflare**: `1.1.1.1` (Fast and privacy-focused)
- **Google**: `8.8.8.8` (Reliable and widely used)
- **Quad9**: `9.9.9.9` (Security and privacy-focused)
- **OpenDNS**: `208.67.222.222` (Family-friendly filtering available)
- **AdGuard**: `94.140.14.14` (Ad-blocking DNS)

## Troubleshooting

**Error: "This program must be run as root!"**
- Solution: Use `sudo` when running the program
- Example: `sudo dnschanger set 8.8.8.8`

**Error: "The 'dnschanger' command was not found in PATH" (GUI)**
- Solution: Make sure the program is installed using `sudo ./install.sh`
- Or ensure the compiled binary is in your PATH

**Error: "Cannot open /etc/resolv.conf for reading"**
- Solution: Ensure the file exists and the program has read permissions

**Error: "Invalid DNS IP address"**
- Solution: Check that the IP address is in valid IPv4 format (e.g., `8.8.8.8`)

**Error: "Backup file not found"**
- Solution: The backup was never created. Run `set` command first to create a backup.

**GUI doesn't start**
- Solution: Install Python dependencies with `pip3 install -r gui/requirements.txt`
- Make sure you're running with sudo: `sudo python3 gui/gui.py`

**DNS changes don't persist after reboot**
- Some network managers may override manual DNS settings
- Consider using your network manager's GUI or configuration files for persistent changes

## Contributing

This is an open-source project that welcomes contributions! We are looking for help with:

- **Cross-platform Support**: Implementation for macOS, Windows, and other operating systems
- **DNS Services Integration**: Support for NetworkManager, systemd-resolved, and other DNS management systems
- **Feature Enhancements**: Additional functionality and improvements
- **Testing**: Comprehensive testing on various Linux distributions
- **Documentation**: Improvements to documentation and examples
- **Bug Reports**: Reporting and fixing issues

## Future Development

Planned features and improvements:

- [x] NetworkManager integration for modern Linux distributions
- [x] systemd-resolved support
- [x] GUI application for easier DNS management
- [ ] macOS support (using `scutil` and `networksetup`)
- [ ] Windows support (using WMI and Registry modifications)
- [ ] DNS server list management (multiple DNS servers)
- [ ] Configuration file support for managing multiple DNS profiles
- [ ] IPv6 DNS support
- [ ] DNS-over-HTTPS (DoH) and DNS-over-TLS (DoT) support
- [ ] Unit tests and integration tests
- [ ] Package managers support (apt, yum, pacman, brew, etc.)
- [ ] Desktop entry file for launching GUI from application menu

## License

This project is provided as-is for educational and personal use.

## Screenshots

### GUI Interface
The graphical interface provides an intuitive way to manage DNS settings with:
- Quick access buttons for popular DNS providers
- Custom DNS input field
- Real-time output display
- Modern dark theme using CustomTkinter

## Notes

- DNS changes may take a moment to propagate
- The program automatically detects and works with your system's network manager
- Supported network managers: NetworkManager, systemd-resolved, resolvconf, openresolv
- Always creates a backup before making changes for safety
- The GUI requires the CLI tool to be installed and accessible in PATH

## Security Considerations

- Always verify DNS server addresses before applying them
- Use trusted DNS providers to avoid DNS hijacking
- The program requires root privileges to modify system DNS settings
- Backups are stored in `/etc/resolv.conf.backup` - keep this file secure
