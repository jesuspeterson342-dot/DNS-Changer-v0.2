# DNS Changer

A powerful and intelligent DNS management utility for Linux systems with both command-line interface (CLI) and graphical user interface (GUI). Automatically detects your system's network manager and applies DNS changes using the appropriate method.

## Features

- **🔄 Change DNS**: Set a custom DNS server address with automatic validation
- **♻️ Restore DNS**: Restore the previous DNS settings from automatic backups
- **📊 Check Status**: View current DNS configuration in a clear format
- **✅ IP Validation**: Robust IPv4 address validation to ensure correct DNS server addresses
- **💾 Automatic Backup**: Creates backups before any changes to prevent configuration loss
- **🔧 Multi-Manager Support**: Intelligent detection and support for:
  - NetworkManager (nmcli) - Modern Linux distributions
  - systemd-resolved - systemd-based systems
  - resolvconf - Traditional Debian/Ubuntu systems
  - openresolv - Alternative resolver management
  - Fallback mode - Direct `/etc/resolv.conf` modification
- **🎨 GUI Application**: Modern, user-friendly graphical interface built with CustomTkinter
- **⚡ Quick DNS Presets**: One-click access to popular DNS providers (Cloudflare, Google, Quad9)
- **🛡️ Safety First**: Automatic rollback on failure to prevent DNS breakage
- **🖥️ Desktop Integration**: Application menu launcher with desktop entry file

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

This will:
- Compile the C++ program with optimizations
- Install the CLI binary to `/usr/local/bin/dnschanger`
- Install the GUI to `/usr/local/share/dnschanger/`
- Create a desktop entry for application menu integration

### Using Makefile

Alternatively, use the provided Makefile:

```bash
make              # Compile the program
sudo make install # Install CLI, GUI, and desktop entry
sudo make uninstall # Remove all installed files
```

### Manual Compilation

Compile directly with g++:

```bash
g++ -std=c++17 -O2 -Wall src/main.cpp -o dnschanger
sudo cp dnschanger /usr/local/bin/
```

### GUI Setup

Install Python dependencies (required for GUI):

```bash
pip3 install -r gui/requirements.txt
```

Or install manually:

```bash
pip3 install customtkinter
```

**Recommended**: Use a virtual environment:

```bash
python3 -m venv venv
source venv/bin/activate
pip install customtkinter
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

**Option 1**: Use the convenient launcher script (recommended):

```bash
./run_gui.sh
```

This script automatically:
- Checks for Python and dependencies
- Grants X11 access to root
- Launches the GUI with proper environment
- Cleans up X11 permissions after exit

**Option 2**: Launch directly:

```bash
sudo python3 gui/gui.py
```

**Option 3**: Launch from application menu (after installation):

Search for "DNS Changer" in your application menu.

The GUI provides:
- **Quick DNS buttons** for popular providers (Cloudflare 1.1.1.1, Google 8.8.8.8, Quad9 9.9.9.9)
- **Custom DNS input** with real-time IP validation
- **Restore functionality** to revert to previous settings
- **Status checking** to view current DNS configuration
- **Real-time output** showing command execution and results
- **Modern dark theme** using CustomTkinter
- **Automatic executable detection** (finds dnschanger in multiple locations)

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

The program uses intelligent detection to identify which network management system is running on your Linux distribution:

1. **NetworkManager** (nmcli) - Detected by checking:
   - `systemctl is-active NetworkManager`
   - Presence of `/run/NetworkManager/NetworkManager.pid`

2. **systemd-resolved** - Detected by checking:
   - `systemctl is-active systemd-resolved`
   - Presence of `/run/systemd/resolve/stub-resolv.conf` or `/run/systemd/resolve/resolv.conf`

3. **openresolv** - Detected by checking:
   - Presence of `/etc/resolvconf.conf`
   - Configuration contains `name_servers` directive

4. **resolvconf** (Debian/Ubuntu) - Detected by checking:
   - Presence of `/run/resolvconf/resolv.conf`

5. **Fallback** - Direct `/etc/resolv.conf` modification if no manager is detected

### Setting DNS (set command)
1. ✅ Validates that the user has root privileges
2. 🔍 Detects the active network manager
3. ✅ Validates the IP address format (IPv4: 4 octets, each 0-255)
4. 💾 Backs up the current DNS configuration to `/etc/resolv.conf.backup`
5. 🔧 Applies the new DNS settings using the appropriate method:
   - **NetworkManager**: Uses `nmcli dev modify` and `nmcli con reload`
   - **systemd-resolved**: Modifies `/etc/systemd/resolved.conf` and restarts service
   - **resolvconf**: Writes to `/etc/resolvconf/resolv.conf.d/head` and runs `resolvconf -u`
   - **openresolv**: Updates `/etc/resolvconf.conf` and runs `resolvconf -u`
   - **Fallback**: Directly writes to `/etc/resolv.conf`
6. 🛡️ If setting fails, automatically restores the previous configuration

### Restoring DNS (restore command)
1. ✅ Validates that the user has root privileges
2. 🔍 Detects the active network manager
3. 📂 Reads the backup file from `/etc/resolv.conf.backup`
4. ✅ Verifies backup file exists and has content
5. ♻️ Restores the original DNS settings using the appropriate method for the detected manager

### Checking Status (status command)
1. ✅ Validates that the user has root privileges
2. 🔍 Detects the active network manager
3. 📊 Displays current DNS configuration:
   - **NetworkManager**: Shows DNS from `nmcli -t -f IP4.DNS dev show`
   - **systemd-resolved**: Shows output from `resolvectl status` or reads `/etc/systemd/resolved.conf`
   - **openresolv**: Shows `name_servers` from `/etc/resolvconf.conf` and current `/etc/resolv.conf`
   - **resolvconf/Fallback**: Parses and displays nameservers from `/etc/resolv.conf`

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
│   └── main.cpp              # Main C++ source code (847 lines)
├── gui/
│   ├── gui.py                # Python GUI application (317 lines)
│   └── requirements.txt      # Python dependencies
├── venv/                     # Python virtual environment (optional)
├── bulid/                    # Compiled binaries directory
│   └── my_program.bin        # Compiled executable
├── install.sh                # Installation script
├── run_gui.sh                # GUI launcher script with X11 setup
├── Makefile                  # Build and installation automation
├── dnschanger.desktop        # Desktop entry file for app menu
├── README.md                 # English documentation
├── README.ru.md              # Russian documentation
└── LICENSE                   # License file

```

### Source Code (`/src`)
- **`main.cpp`** - Main program implementation (C++17)
  - **Enum `NetworkManager`**: Defines supported network managers
  - **Detection functions**: `checkNetworkManager()`, `isServiceActive()`
  - **Validation**: `isValidIP()` - IPv4 address validation with regex
  - **Backup functions**: Manager-specific backup implementations
    - `backupNetworkManager()`, `backupSystemdResolved()`, `backupOpenresolv()`, `backupResolvconf()`
  - **DNS setting functions**: Manager-specific DNS configuration
    - `setDnsNetworkManager()`, `setDnsSystemdResolved()`, `setDnsResolvconf()`, `setDnsOpenresolv()`, `setDnsFallback()`
  - **Restore functions**: Manager-specific restoration
    - `restoreDnsNetworkManager()`, `restoreDnsSystemdResolved()`, `restoreDnsOpenresolv()`, `restoreDnsResolvconf()`
  - **Status functions**: Manager-specific status display
    - `showStatusNetworkManager()`, `showStatusSystemdResolved()`, `showStatusResolvconf()`, `showStatusOpenresolv()`
  - **Utility functions**: `isRoot()`, `executeCommand()`, `executeCommandWithOutput()`
  - **Command-line parsing**: Handles `set`, `restore`, and `status` commands

### GUI (`/gui`)
- **`gui.py`** - CustomTkinter-based graphical interface
  - **Class `DNSChangerApp`**: Main application window (400x500px, dark theme)
  - **Executable detection**: `_find_dnschanger()` - searches multiple locations
  - **Quick DNS section**: Preset buttons for Cloudflare, Google, Quad9
  - **Custom DNS section**: Input field with validation, apply/restore/status buttons
  - **Output section**: Real-time command output with timestamps
  - **Threading**: Non-blocking command execution to prevent UI freezing
  - **Root checking**: Validates root privileges on startup
  - **X11 handling**: Checks for DISPLAY environment variable
- **`requirements.txt`** - Python package dependencies (customtkinter>=5.2.0)

### Scripts
- **`install.sh`** - Automated installation script
  - Checks for root privileges and g++ compiler
  - Compiles with C++17 and O2 optimization
  - Installs CLI, GUI, and desktop entry
- **`run_gui.sh`** - GUI launcher with proper setup
  - Manages X11 permissions for root
  - Handles virtual environment detection
  - Preserves environment variables
- **`Makefile`** - Build automation
  - Targets: `all`, `clean`, `install`, `uninstall`
  - Installs to `/usr/local/bin/` and `/usr/local/share/dnschanger/`

### Desktop Integration
- **`dnschanger.desktop`** - FreeDesktop entry file
  - Uses `pkexec` for privilege escalation
  - Categories: System, Settings, Network
  - Installed to `/usr/share/applications/`

### System Files (Created at Runtime)

- `/etc/resolv.conf.backup` - Backup of DNS settings created when running `set` command
- `/usr/local/bin/dnschanger` - Installed CLI binary location (after installation)
- `/usr/local/share/dnschanger/` - Installed GUI location (after installation)

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

### CLI Issues

**Error: "This program must be run as root!"**
- **Cause**: The program requires root privileges to modify system DNS settings
- **Solution**: Use `sudo` when running the program
- **Example**: `sudo dnschanger set 8.8.8.8`

**Error: "Invalid DNS IP address"**
- **Cause**: The provided IP address doesn't match IPv4 format
- **Solution**: Check that the IP address is valid (e.g., `8.8.8.8`)
- **Valid format**: 4 octets separated by dots, each octet 0-255
- **Invalid examples**: `256.1.1.1`, `8.8.8`, `invalid.address`

**Error: "Backup file not found"**
- **Cause**: No backup exists at `/etc/resolv.conf.backup`
- **Solution**: Run `set` command first to create a backup before restoring

**Error: "Cannot open /etc/resolv.conf for reading"**
- **Cause**: File doesn't exist or lacks read permissions
- **Solution**: Ensure the file exists and the program has read permissions

**Error: "Failed to set DNS via nmcli"**
- **Cause**: NetworkManager command failed
- **Solution**: Check that NetworkManager is running: `systemctl status NetworkManager`

### GUI Issues

**Error: "dnschanger executable not found!"**
- **Cause**: The CLI program is not installed or not in PATH
- **Solution**:
  - Install using `sudo ./install.sh`
  - Or compile manually: `g++ -std=c++17 -O2 src/main.cpp -o dnschanger`
  - Ensure it's in `/usr/local/bin/` or another PATH location

**Error: "customtkinter module not found!"**
- **Cause**: Python dependency not installed
- **Solution**: Install with `pip3 install customtkinter` or `pip3 install -r gui/requirements.txt`

**Error: "Root privileges required!"**
- **Cause**: GUI not running with sudo
- **Solution**: Run with `sudo python3 gui/gui.py` or use `./run_gui.sh`

**Error: "No DISPLAY environment variable!"**
- **Cause**: X11 display not available to root
- **Solution**:
  - Use `sudo -E python3 gui/gui.py` to preserve environment
  - Or grant X11 access: `xhost +si:localuser:root`
  - Or use the launcher script: `./run_gui.sh`

**GUI window doesn't appear**
- **Cause**: X11 permission issues
- **Solution**:
  ```bash
  xhost +si:localuser:root
  sudo -E python3 gui/gui.py
  ```

### General Issues

**DNS changes don't persist after reboot**
- **Cause**: Network manager may override manual settings
- **Solution**:
  - For persistent changes, configure your network manager directly
  - NetworkManager: Use `nmcli` or network settings GUI
  - systemd-resolved: Edit `/etc/systemd/resolved.conf` permanently

**Changes don't take effect immediately**
- **Cause**: DNS cache or application-level caching
- **Solution**:
  - Flush DNS cache: `sudo systemd-resolve --flush-caches` (if using systemd-resolved)
  - Restart network: `sudo systemctl restart NetworkManager`
  - Test with: `nslookup google.com` or `dig google.com`

**"Warning: Unknown network manager"**
- **Cause**: No recognized network manager detected
- **Effect**: Program will use fallback method (direct `/etc/resolv.conf` modification)
- **Note**: This is not an error, just informational

## Contributing

This is an open-source project that welcomes contributions! We are looking for help with:

- **Cross-platform Support**: Implementation for macOS, Windows, and other operating systems
- **DNS Services Integration**: Support for NetworkManager, systemd-resolved, and other DNS management systems
- **Feature Enhancements**: Additional functionality and improvements
- **Testing**: Comprehensive testing on various Linux distributions
- **Documentation**: Improvements to documentation and examples
- **Bug Reports**: Reporting and fixing issues

## Technical Details

### Compilation
- **Language**: C++17
- **Compiler**: g++ with `-std=c++17 -O2 -Wall` flags
- **Dependencies**: Standard C++ library only (no external dependencies)
- **Binary size**: ~50KB (optimized)

### Python GUI
- **Python version**: 3.7+
- **Framework**: CustomTkinter 5.2.0+
- **Theme**: Dark mode with dark-blue color scheme
- **Window size**: 400x500px (fixed, non-resizable)

### Security Considerations
- ✅ Requires root privileges (checked via `geteuid()`)
- ✅ IP validation prevents injection attacks
- ✅ Automatic backups prevent configuration loss
- ✅ Rollback on failure prevents DNS breakage
- ⚠️ Backups stored in `/etc/resolv.conf.backup` - keep secure
- ⚠️ Always verify DNS server addresses before applying
- ⚠️ Use trusted DNS providers to avoid DNS hijacking

## Future Development

Planned features and improvements:

- [x] NetworkManager integration for modern Linux distributions
- [x] systemd-resolved support
- [x] resolvconf and openresolv support
- [x] GUI application for easier DNS management
- [x] Desktop entry file for launching GUI from application menu
- [x] Automatic network manager detection
- [x] Automatic backup and restore functionality
- [x] GUI launcher script with X11 setup
- [ ] macOS support (using `scutil` and `networksetup`)
- [ ] Windows support (using WMI and Registry modifications)
- [ ] Multiple DNS servers support (primary and secondary)
- [ ] Configuration file support for managing multiple DNS profiles
- [ ] IPv6 DNS support
- [ ] DNS-over-HTTPS (DoH) and DNS-over-TLS (DoT) support
- [ ] Unit tests and integration tests
- [ ] Package managers support (apt, yum, pacman, brew, etc.)
- [ ] Systemd service for persistent DNS management
- [ ] DNS performance testing and benchmarking
- [ ] Import/export DNS profiles

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
