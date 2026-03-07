# DNS Changer

A command-line utility written in C++ that allows you to change and restore DNS settings on Linux systems.

## Features

- **Change DNS**: Set a custom DNS server address
- **Restore DNS**: Restore the previous DNS settings from a backup
- **Validation**: IP address validation to ensure correct DNS server addresses
- **Backup**: Automatic backup of current DNS settings before changes
- **Root Check**: Verifies that the program is run with root privileges

## Requirements

- Linux operating system
- C++17 or later compiler
- Root/sudo privileges to modify DNS settings
- CMake 3.20 or later (for building)

## Building

Using CMake:

```bash
cd "DNS Changer"
mkdir build
cd build
cmake ..
make
```

Or compile directly:

```bash
g++ -std=c++17 src/main.cpp -o my_program
```

## Usage

The program must be run with root privileges:

```bash
sudo ./my_program set <dns_ip>
```

```bash
sudo ./my_program restore
```

### Commands

- **`set <dns_ip>`**: Changes the DNS server to the specified IP address
  - Example: `sudo ./my_program set 8.8.8.8`
  - Automatically backs up the current DNS settings to `/etc/resolv.conf.backup`

- **`restore`**: Restores the DNS settings from the backup file
  - Example: `sudo ./my_program restore`
  - Restores from `/etc/resolv.conf.backup`

## Examples

Change DNS to Google's public DNS:
```bash
sudo ./my_program set 8.8.8.8
```

Change DNS to Cloudflare's DNS:
```bash
sudo ./my_program set 1.1.1.1
```

Restore previous DNS settings:
```bash
sudo ./my_program restore
```

## How It Works

### Setting DNS
1. Validates that the user has root privileges
2. Validates the IP address format (checks for valid IPv4)
3. Backs up the current `/etc/resolv.conf` to `/etc/resolv.conf.backup`
4. Replaces the DNS settings in `/etc/resolv.conf` with the new server

### Restoring DNS
1. Validates that the user has root privileges
2. Reads the backup file from `/etc/resolv.conf.backup`
3. Restores the original DNS settings to `/etc/resolv.conf`

## IP Validation

The program validates IPv4 addresses using the following criteria:
- Must have 4 octets separated by dots
- Each octet must be a number between 0 and 255

Examples of valid addresses: `8.8.8.8`, `1.1.1.1`, `192.168.1.1`
Examples of invalid addresses: `256.1.1.1`, `8.8.8`, `invalid.address`

## Project Structure

### Directories

#### `/src` - Source Code
- `main.cpp` - Main program source code
  - Includes IP validation functions
  - DNS backup and restore functionality
  - Root privilege checking
  - File I/O operations for DNS configuration

#### `/build` - Build Artifacts (Compiled Binaries)
- `my_program.bin` - Binary executable (Linux)

### System Files

- `/etc/resolv.conf` - System DNS configuration file (modified by the program)
- `/etc/resolv.conf.backup` - Backup of DNS settings created when running `set` command

## Safety

- Automatic backups are created before modifying DNS settings
- IP address validation prevents invalid entries
- Requires root privileges to prevent unauthorized changes
- Original DNS settings can always be restored

## Troubleshooting

**Error: "Run as root!"**
- Solution: Use `sudo` when running the program

**Error: "Cannot open /etc/resolv.conf for reading"**
- Solution: Ensure the file exists and the program has read permissions

**Error: "Invalid DNS IP address"**
- Solution: Check that the IP address is in valid IPv4 format (e.g., `8.8.8.8`)

**Error: "Backup file not found"**
- Solution: The backup was never created. Run `set` command first to create a backup.

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

- [ ] macOS support (using `scutil` and `networksetup`)
- [ ] Windows support (using WMI and Registry modifications)
- [ ] NetworkManager integration for modern Linux distributions
- [ ] systemd-resolved support
- [ ] DNS server list management (multiple DNS servers)
- [ ] Configuration file support for managing multiple DNS profiles
- [ ] GUI application for easier DNS management
- [ ] Unit tests and integration tests
- [ ] Package managers support (apt, yum, pacman, brew, etc.)

## License

LICENSE MIT:
MIT License

Copyright (c) 2026 Davud

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software.

## Notes

- DNS changes may take a moment to propagate
- The program modifies `/etc/resolv.conf` directly, which requires root access
- On some systems, DNS settings might be managed by NetworkManager or systemd-resolved. This tool directly modifies `/etc/resolv.conf`, which may be overridden by these services
