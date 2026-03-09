#include <iostream>
#include <string>
#include <fstream>
#include <unistd.h>
#include <regex>
#include <sstream>
#include <cstdlib>

// Enum for network manager types
enum class NetworkManager {
    NETWORK_MANAGER = 0,  // NetworkManager (nmcli)
    SYSTEMD_RESOLVED = 1, // systemd-resolved
    RESOLVCONF = 2,       // resolvconf (Debian/Ubuntu)
    OPENRESOLV = 3,       // openresolv
    UNKNOWN = -1          // Unknown or not detected
};

// Convert NetworkManager enum to string for logging
std::string networkManagerToString(NetworkManager manager) {
    switch (manager) {
        case NetworkManager::NETWORK_MANAGER:
            return "NetworkManager (nmcli)";
        case NetworkManager::SYSTEMD_RESOLVED:
            return "systemd-resolved";
        case NetworkManager::RESOLVCONF:
            return "resolvconf";
        case NetworkManager::OPENRESOLV:
            return "openresolv";
        case NetworkManager::UNKNOWN:
            return "Unknown";
        default:
            return "Invalid";
    }
}

// Check if user has root privileges
bool isRoot() {
    return geteuid() == 0;
}

// Execute command and return exit code
int executeCommand(const std::string& command) {
    int ret = system(command.c_str());
    return WEXITSTATUS(ret);
}

// Execute command and capture output
bool executeCommandWithOutput(const std::string& command, std::string& output) {
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return false;
    }

    char buffer[256];
    output.clear();
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }

    int ret = pclose(pipe);
    return WEXITSTATUS(ret) == 0;
}

// Check if a systemd service is active
bool isServiceActive(const std::string& service_name) {
    std::string output;
    std::string command = "systemctl is-active " + service_name + " 2>/dev/null";
    if (executeCommandWithOutput(command, output)) {
        return output.find("active") == 0;
    }
    return false;
}

// Validate IP address format
bool isValidIP(const std::string& ip) {
    // Simple regex for IPv4 validation
    std::regex ipv4_pattern(R"(^(\d{1,3}\.){3}\d{1,3}$)");
    if (!std::regex_match(ip, ipv4_pattern)) {
        return false;
    }

    // Check if each octet is in range 0-255
    std::regex octet_pattern(R"(\d+)");
    std::sregex_iterator iter(ip.begin(), ip.end(), octet_pattern);
    std::sregex_iterator end;

    while (iter != end) {
        int num = std::stoi(iter->str());
        if (num > 255) {
            return false;
        }
        ++iter;
    }

    return true;
}

// Check which network manager is active on the system
NetworkManager checkNetworkManager() {
    // Check for NetworkManager — verify service is running
    if (isServiceActive("NetworkManager")) {
        // Double-check with PID file
        if (std::ifstream("/run/NetworkManager/NetworkManager.pid")) {
            return NetworkManager::NETWORK_MANAGER;
        }
    }

    // Check for systemd-resolved — verify service is running
    if (isServiceActive("systemd-resolved")) {
        // Double-check with stub resolver
        if (std::ifstream("/run/systemd/resolve/stub-resolv.conf") ||
            std::ifstream("/run/systemd/resolve/resolv.conf")) {
            return NetworkManager::SYSTEMD_RESOLVED;
        }
    }

    // Check for openresolv — has its own config file distinct from Debian resolvconf
    if (std::ifstream("/etc/resolvconf.conf")) {
        // Verify it's openresolv by checking for name_servers directive
        std::ifstream conf("/etc/resolvconf.conf");
        std::string line;
        while (std::getline(conf, line)) {
            if (line.find("name_servers") != std::string::npos) {
                return NetworkManager::OPENRESOLV;
            }
        }
    }

    // Check for resolvconf (Debian/Ubuntu) — runtime-managed resolv.conf
    if (std::ifstream("/run/resolvconf/resolv.conf")) {
        return NetworkManager::RESOLVCONF;
    }

    return NetworkManager::UNKNOWN;
}

// Backup functions for each network manager
bool backupNetworkManager(std::ofstream& backup_file) {
    std::string output;
    if (!executeCommandWithOutput("nmcli -t -f IP4.DNS dev show $(nmcli -t -f DEVICE dev status | head -n1) 2>/dev/null", output)) {
        std::cerr << "Error: Cannot get NetworkManager DNS settings" << std::endl;
        return false;
    }

    if (output.empty()) {
        std::cerr << "Warning: No DNS settings found in NetworkManager" << std::endl;
    }

    backup_file << output;
    return true;
}

bool backupSystemdResolved(std::ofstream& backup_file) {
    std::ifstream resolved_conf("/etc/systemd/resolved.conf", std::ios::binary);
    if (!resolved_conf) {
        std::cerr << "Error: Cannot open /etc/systemd/resolved.conf for reading" << std::endl;
        return false;
    }

    backup_file << resolved_conf.rdbuf();
    return resolved_conf.good() || resolved_conf.eof();
}

bool backupOpenresolv(std::ofstream& backup_file) {
    std::ifstream resolvconf_conf("/etc/resolvconf.conf", std::ios::binary);
    if (!resolvconf_conf) {
        std::cerr << "Error: Cannot open /etc/resolvconf.conf for reading" << std::endl;
        return false;
    }

    backup_file << resolvconf_conf.rdbuf();
    return resolvconf_conf.good() || resolvconf_conf.eof();
}

bool backupResolvconf(std::ofstream& backup_file) {
    std::ifstream resolv_conf("/etc/resolv.conf", std::ios::binary);
    if (!resolv_conf) {
        std::cerr << "Error: Cannot open /etc/resolv.conf for reading" << std::endl;
        return false;
    }

    backup_file << resolv_conf.rdbuf();
    return resolv_conf.good() || resolv_conf.eof();
}

// Get current DNS settings and save to file
bool saveCurrentDns(const std::string& filename, NetworkManager manager) {
    // Remove old backup if exists
    std::remove(filename.c_str());

    std::ofstream backup_file(filename);
    if (!backup_file) {
        std::cerr << "Error: Cannot create backup file: " << filename << std::endl;
        return false;
    }

    bool success = false;

    switch (manager) {
        case NetworkManager::NETWORK_MANAGER:
            success = backupNetworkManager(backup_file);
            break;

        case NetworkManager::SYSTEMD_RESOLVED:
            success = backupSystemdResolved(backup_file);
            break;

        case NetworkManager::OPENRESOLV:
            success = backupOpenresolv(backup_file);
            break;

        case NetworkManager::RESOLVCONF:
        case NetworkManager::UNKNOWN:
            success = backupResolvconf(backup_file);
            break;
    }

    if (!success) {
        std::cerr << "Error: Failed to backup DNS settings" << std::endl;
        backup_file.close();
        std::remove(filename.c_str());
        return false;
    }

    if (!backup_file) {
        std::cerr << "Error: Failed to write to backup file" << std::endl;
        backup_file.close();
        std::remove(filename.c_str());
        return false;
    }

    backup_file.close();

    // Verify backup file was created and has content
    std::ifstream verify(filename);
    if (!verify || verify.peek() == std::ifstream::traits_type::eof()) {
        std::cerr << "Error: Backup file is empty or unreadable" << std::endl;
        return false;
    }

    std::cout << "✓ Current DNS settings saved to: " << filename << std::endl;
    return true;
}

// DNS setting functions for each network manager

bool setDnsNetworkManager(const std::string& dns) {
    std::string command = "nmcli dev modify $(nmcli -t -f DEVICE dev status | head -n1) ipv4.dns " + dns;
    if (executeCommand(command) != 0) {
        std::cerr << "Error: Failed to set DNS via nmcli" << std::endl;
        return false;
    }

    if (executeCommand("nmcli con reload") != 0) {
        std::cerr << "Error: Failed to reload NetworkManager connections" << std::endl;
        return false;
    }

    return true;
}

bool setDnsSystemdResolved(const std::string& dns) {
    // Read existing config
    std::ifstream existing("/etc/systemd/resolved.conf");
    std::string content;
    std::string line;
    bool resolve_section_found = false;
    bool dns_added = false;

    while (std::getline(existing, line)) {
        // Skip old DNS lines in [Resolve] section
        if (line.find("[Resolve]") == 0) {
            resolve_section_found = true;
            content += line + "\n";
            continue;
        }
        if (resolve_section_found && !dns_added && (line.empty() || line[0] == '[')) {
            content += "DNS=" + dns + "\n";
            dns_added = true;
        }
        if (line.find("DNS=") != 0) {
            content += line + "\n";
        }
    }
    existing.close();

    // If no [Resolve] section found, add it
    if (!resolve_section_found) {
        content += "[Resolve]\nDNS=" + dns + "\n";
    } else if (!dns_added) {
        content += "DNS=" + dns + "\n";
    }

    std::ofstream file("/etc/systemd/resolved.conf", std::ios::trunc);
    if (!file) {
        std::cerr << "Error: Cannot open /etc/systemd/resolved.conf for writing" << std::endl;
        return false;
    }

    file << content;
    file.close();

    if (!file) {
        std::cerr << "Error: Failed to write to /etc/systemd/resolved.conf" << std::endl;
        return false;
    }

    if (executeCommand("systemctl restart systemd-resolved") != 0) {
        std::cerr << "Error: Failed to restart systemd-resolved" << std::endl;
        return false;
    }

    return true;
}

bool setDnsResolvconf(const std::string& dns) {
    std::ofstream file("/etc/resolvconf/resolv.conf.d/head", std::ios::trunc);
    if (!file) {
        std::cerr << "Error: Cannot open /etc/resolvconf/resolv.conf.d/head for writing" << std::endl;
        return false;
    }

    file << "nameserver " << dns << "\n";
    file.close();

    if (!file) {
        std::cerr << "Error: Failed to write to /etc/resolvconf/resolv.conf.d/head" << std::endl;
        return false;
    }

    if (executeCommand("resolvconf -u") != 0) {
        std::cerr << "Error: Failed to update resolvconf" << std::endl;
        return false;
    }

    return true;
}

bool setDnsOpenresolv(const std::string& dns) {
    // Read existing config
    std::ifstream existing("/etc/resolvconf.conf");
    std::string content;
    std::string line;
    bool dns_replaced = false;

    while (std::getline(existing, line)) {
        // Replace name_servers line
        if (line.find("name_servers=") == 0) {
            content += "name_servers=\"" + dns + "\"\n";
            dns_replaced = true;
        } else {
            content += line + "\n";
        }
    }
    existing.close();

    // If no name_servers line found, add it
    if (!dns_replaced) {
        content += "name_servers=\"" + dns + "\"\n";
    }

    std::ofstream file("/etc/resolvconf.conf", std::ios::trunc);
    if (!file) {
        std::cerr << "Error: Cannot open /etc/resolvconf.conf for writing" << std::endl;
        return false;
    }

    file << content;
    file.close();

    if (!file) {
        std::cerr << "Error: Failed to write to /etc/resolvconf.conf" << std::endl;
        return false;
    }

    if (executeCommand("resolvconf -u") != 0) {
        std::cerr << "Error: Failed to update openresolv" << std::endl;
        return false;
    }

    return true;
}

bool setDnsFallback(const std::string& dns) {
    std::ofstream file("/etc/resolv.conf", std::ios::trunc);
    if (!file) {
        std::cerr << "Error: Cannot open /etc/resolv.conf for writing" << std::endl;
        return false;
    }

    file << "nameserver " << dns << "\n";
    file.close();

    if (!file) {
        std::cerr << "Error: Failed to write to /etc/resolv.conf" << std::endl;
        return false;
    }

    return true;
}

// Set DNS settings
bool setDns(const std::string& dns, NetworkManager manager) {
    if (!isValidIP(dns)) {
        std::cerr << "Error: Invalid DNS IP address: " << dns << std::endl;
        return false;
    }

    bool success = false;

    switch (manager) {
        case NetworkManager::NETWORK_MANAGER:
            success = setDnsNetworkManager(dns);
            break;

        case NetworkManager::SYSTEMD_RESOLVED:
            success = setDnsSystemdResolved(dns);
            break;

        case NetworkManager::RESOLVCONF:
            success = setDnsResolvconf(dns);
            break;

        case NetworkManager::OPENRESOLV:
            success = setDnsOpenresolv(dns);
            break;

        case NetworkManager::UNKNOWN:
            std::cout << "Warning: Unknown network manager, using fallback method" << std::endl;
            success = setDnsFallback(dns);
            break;
    }

    if (!success) {
        std::cerr << "Error: Failed to set DNS for " << networkManagerToString(manager) << std::endl;
        return false;
    }

    return true;
}

// DNS restore functions for each network manager

bool restoreDnsNetworkManager(std::ifstream& backup) {
    std::string line;
    std::string dns_servers;

    while (std::getline(backup, line)) {
        // Parse nmcli output format: IP4.DNS[1]:8.8.8.8
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos && line.find("IP4.DNS") == 0) {
            std::string ip = line.substr(colon_pos + 1);
            // Trim whitespace
            ip.erase(0, ip.find_first_not_of(" \t\r\n"));
            ip.erase(ip.find_last_not_of(" \t\r\n") + 1);
            if (!dns_servers.empty()) dns_servers += " ";
            dns_servers += ip;
        }
    }

    if (dns_servers.empty()) {
        std::cerr << "Warning: No DNS servers found in backup" << std::endl;
        return false;
    }

    std::string command = "nmcli dev modify $(nmcli -t -f DEVICE dev status | head -n1) ipv4.dns \"" + dns_servers + "\"";
    if (executeCommand(command) != 0) {
        std::cerr << "Error: Failed to restore DNS via nmcli" << std::endl;
        return false;
    }

    if (executeCommand("nmcli con reload") != 0) {
        std::cerr << "Error: Failed to reload NetworkManager connections" << std::endl;
        return false;
    }

    return true;
}

bool restoreDnsSystemdResolved(std::ifstream& backup) {
    std::ofstream resolved_conf("/etc/systemd/resolved.conf", std::ios::trunc);
    if (!resolved_conf) {
        std::cerr << "Error: Cannot open /etc/systemd/resolved.conf for writing" << std::endl;
        return false;
    }

    resolved_conf << backup.rdbuf();
    resolved_conf.close();

    if (!resolved_conf) {
        std::cerr << "Error: Failed to write to /etc/systemd/resolved.conf" << std::endl;
        return false;
    }

    if (executeCommand("systemctl restart systemd-resolved") != 0) {
        std::cerr << "Error: Failed to restart systemd-resolved" << std::endl;
        return false;
    }

    return true;
}

bool restoreDnsOpenresolv(std::ifstream& backup) {
    std::ofstream resolvconf_conf("/etc/resolvconf.conf", std::ios::trunc);
    if (!resolvconf_conf) {
        std::cerr << "Error: Cannot open /etc/resolvconf.conf for writing" << std::endl;
        return false;
    }

    resolvconf_conf << backup.rdbuf();
    resolvconf_conf.close();

    if (!resolvconf_conf) {
        std::cerr << "Error: Failed to write to /etc/resolvconf.conf" << std::endl;
        return false;
    }

    if (executeCommand("resolvconf -u") != 0) {
        std::cerr << "Error: Failed to update openresolv" << std::endl;
        return false;
    }

    return true;
}

bool restoreDnsResolvconf(std::ifstream& backup, bool update) {
    std::ofstream resolv("/etc/resolv.conf", std::ios::trunc);
    if (!resolv) {
        std::cerr << "Error: Cannot open /etc/resolv.conf for writing" << std::endl;
        return false;
    }

    resolv << backup.rdbuf();
    resolv.close();

    if (!resolv) {
        std::cerr << "Error: Failed to write to /etc/resolv.conf" << std::endl;
        return false;
    }

    if (update) {
        if (executeCommand("resolvconf -u") != 0) {
            std::cerr << "Error: Failed to update resolvconf" << std::endl;
            return false;
        }
    }

    return true;
}

// Restore DNS settings from backup
bool restoreDns(const std::string& backup_file_path, NetworkManager manager) {
    // Verify backup file exists and is readable
    std::ifstream backup(backup_file_path);
    if (!backup) {
        std::cerr << "Error: Backup file not found: " << backup_file_path << std::endl;
        return false;
    }

    // Verify backup file has content
    if (backup.peek() == std::ifstream::traits_type::eof()) {
        std::cerr << "Error: Backup file is empty: " << backup_file_path << std::endl;
        return false;
    }

    bool success = false;

    switch (manager) {
        case NetworkManager::NETWORK_MANAGER:
            success = restoreDnsNetworkManager(backup);
            break;

        case NetworkManager::SYSTEMD_RESOLVED:
            success = restoreDnsSystemdResolved(backup);
            break;

        case NetworkManager::OPENRESOLV:
            success = restoreDnsOpenresolv(backup);
            break;

        case NetworkManager::RESOLVCONF:
            success = restoreDnsResolvconf(backup, true);
            break;

        case NetworkManager::UNKNOWN:
            success = restoreDnsResolvconf(backup, false);
            break;
    }

    if (!success) {
        std::cerr << "Error: Failed to restore DNS for " << networkManagerToString(manager) << std::endl;
        return false;
    }

    return true;
}

// DNS status functions for each network manager

bool showStatusNetworkManager() {
    std::string output;
    if (!executeCommandWithOutput("nmcli -t -f IP4.DNS dev show 2>/dev/null", output)) {
        std::cerr << "Error: Cannot get NetworkManager DNS settings" << std::endl;
        return false;
    }

    if (output.empty()) {
        std::cout << "No DNS servers configured in NetworkManager" << std::endl;
        return true;
    }

    std::cout << "Current DNS servers (NetworkManager):" << std::endl;
    std::istringstream iss(output);
    std::string line;
    int count = 1;
    while (std::getline(iss, line)) {
        if (line.find("IP4.DNS") == 0) {
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                std::string ip = line.substr(colon_pos + 1);
                ip.erase(0, ip.find_first_not_of(" \t\r\n"));
                ip.erase(ip.find_last_not_of(" \t\r\n") + 1);
                std::cout << "  " << count++ << ". " << ip << std::endl;
            }
        }
    }
    return true;
}

bool showStatusSystemdResolved() {
    std::string output;
    if (!executeCommandWithOutput("resolvectl status 2>/dev/null || systemd-resolve --status 2>/dev/null", output)) {
        // Fallback: read from resolved.conf
        std::ifstream conf("/etc/systemd/resolved.conf");
        if (!conf) {
            std::cerr << "Error: Cannot read systemd-resolved configuration" << std::endl;
            return false;
        }

        std::cout << "Current DNS configuration (systemd-resolved):" << std::endl;
        std::string line;
        bool found = false;
        while (std::getline(conf, line)) {
            if (line.find("DNS=") == 0) {
                std::cout << "  " << line << std::endl;
                found = true;
            }
        }
        if (!found) {
            std::cout << "  No DNS servers configured" << std::endl;
        }
        return true;
    }

    std::cout << "Current DNS status (systemd-resolved):" << std::endl;
    std::cout << output << std::endl;
    return true;
}

bool showStatusResolvconf() {
    std::ifstream resolv("/etc/resolv.conf");
    if (!resolv) {
        std::cerr << "Error: Cannot open /etc/resolv.conf for reading" << std::endl;
        return false;
    }

    std::cout << "Current DNS servers (/etc/resolv.conf):" << std::endl;
    std::string line;
    int count = 1;
    bool found = false;
    while (std::getline(resolv, line)) {
        if (line.find("nameserver") == 0) {
            std::string ip = line.substr(10);
            ip.erase(0, ip.find_first_not_of(" \t\r\n"));
            ip.erase(ip.find_last_not_of(" \t\r\n") + 1);
            std::cout << "  " << count++ << ". " << ip << std::endl;
            found = true;
        }
    }

    if (!found) {
        std::cout << "  No nameservers found" << std::endl;
    }

    return true;
}

bool showStatusOpenresolv() {
    std::ifstream conf("/etc/resolvconf.conf");
    if (!conf) {
        std::cerr << "Error: Cannot open /etc/resolvconf.conf for reading" << std::endl;
        return false;
    }

    std::cout << "Current DNS configuration (openresolv):" << std::endl;
    std::string line;
    bool found = false;
    while (std::getline(conf, line)) {
        if (line.find("name_servers") == 0) {
            std::cout << "  " << line << std::endl;
            found = true;
        }
    }

    if (!found) {
        std::cout << "  No DNS servers configured" << std::endl;
    }

    // Also show current resolv.conf
    std::cout << "\nCurrent /etc/resolv.conf:" << std::endl;
    return showStatusResolvconf();
}

// Show current DNS status
bool showStatus(NetworkManager manager) {
    bool success = false;

    switch (manager) {
        case NetworkManager::NETWORK_MANAGER:
            success = showStatusNetworkManager();
            break;

        case NetworkManager::SYSTEMD_RESOLVED:
            success = showStatusSystemdResolved();
            break;

        case NetworkManager::OPENRESOLV:
            success = showStatusOpenresolv();
            break;

        case NetworkManager::RESOLVCONF:
        case NetworkManager::UNKNOWN:
            success = showStatusResolvconf();
            break;
    }

    if (!success) {
        std::cerr << "Error: Failed to get DNS status for " << networkManagerToString(manager) << std::endl;
        return false;
    }

    return true;
}

// Main function
int main(int argc, char* argv[]) {
    // Check root privileges
    if (!isRoot()) {
        std::cerr << "Error: This program must be run as root!" << std::endl;
        std::cerr << "Try: sudo " << argv[0] << " <command>" << std::endl;
        return 1;
    }

    // Check command line arguments
    if (argc < 2) {
        std::cout << "DNS Changer - Manage DNS settings across different network managers\n";
        std::cout << "\nUsage:\n";
        std::cout << "  " << argv[0] << " set <dns_ip>    - Set DNS server and backup current settings\n";
        std::cout << "  " << argv[0] << " restore         - Restore DNS settings from backup\n";
        std::cout << "  " << argv[0] << " status          - Show current DNS configuration\n";
        std::cout << "\nExamples:\n";
        std::cout << "  " << argv[0] << " set 8.8.8.8\n";
        std::cout << "  " << argv[0] << " set 1.1.1.1\n";
        std::cout << "  " << argv[0] << " restore\n";
        std::cout << "  " << argv[0] << " status\n";
        return 1;
    }

    std::string command = argv[1];
    const std::string backup_path = "/etc/resolv.conf.backup";

    // Detect network manager
    NetworkManager manager = checkNetworkManager();
    std::cout << "Detected network manager: " << networkManagerToString(manager) << std::endl;

    if (manager == NetworkManager::UNKNOWN) {
        std::cout << "Warning: Could not detect network manager, will use fallback method" << std::endl;
    }

    // Handle 'set' command
    if (command == "set") {
        if (argc != 3) {
            std::cerr << "Error: 'set' command requires DNS IP address" << std::endl;
            std::cerr << "Usage: " << argv[0] << " set <dns_ip>" << std::endl;
            return 1;
        }

        std::string dns = argv[2];

        std::cout << "\n=== Backing up current DNS settings ===" << std::endl;
        if (!saveCurrentDns(backup_path, manager)) {
            std::cerr << "\n✗ Failed to backup current DNS settings" << std::endl;
            std::cerr << "Aborting to prevent loss of DNS configuration" << std::endl;
            return 1;
        }

        std::cout << "\n=== Setting new DNS server ===" << std::endl;
        std::cout << "New DNS: " << dns << std::endl;
        if (!setDns(dns, manager)) {
            std::cerr << "\n✗ Failed to set DNS" << std::endl;
            std::cerr << "Attempting to restore previous settings..." << std::endl;

            if (restoreDns(backup_path, manager)) {
                std::cout << "✓ Previous DNS settings restored successfully" << std::endl;
            } else {
                std::cerr << "✗ Failed to restore previous settings!" << std::endl;
                std::cerr << "Backup file location: " << backup_path << std::endl;
            }
            return 1;
        }

        std::cout << "\n✓ DNS changed successfully to " << dns << std::endl;
        std::cout << "Backup saved to: " << backup_path << std::endl;
    }
    // Handle 'restore' command
    else if (command == "restore") {
        std::cout << "\n=== Restoring DNS settings from backup ===" << std::endl;

        if (!restoreDns(backup_path, manager)) {
            std::cerr << "\n✗ Failed to restore DNS settings" << std::endl;
            std::cerr << "Please check the backup file: " << backup_path << std::endl;
            return 1;
        }

        std::cout << "\n✓ DNS settings restored successfully" << std::endl;
    }
    // Handle 'status' command
    else if (command == "status") {
        std::cout << "\n=== Current DNS Configuration ===" << std::endl;

        if (!showStatus(manager)) {
            std::cerr << "\n✗ Failed to get DNS status" << std::endl;
            return 1;
        }

        std::cout << std::endl;
    }
    // Invalid command
    else {
        std::cerr << "Error: Invalid command '" << command << "'" << std::endl;
        std::cerr << "Valid commands: set, restore, status" << std::endl;
        std::cerr << "Run '" << argv[0] << "' without arguments for usage information" << std::endl;
        return 1;
    }

    return 0;
}
