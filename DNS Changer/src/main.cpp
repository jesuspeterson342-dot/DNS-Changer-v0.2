#include <iostream>
#include <string>
#include <fstream>
#include <unistd.h>
#include <regex>

// Check if user has root privileges
bool isRoot() {
    return geteuid() == 0;
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

// Get current DNS settings and save to file
bool save_current_dns(const std::string& filename) {
    // Open the source file for reading in binary mode
    std::ifstream resolv_conf("/etc/resolv.conf", std::ios::binary);
    if (!resolv_conf) {
        std::cerr << "Error: Cannot open /etc/resolv.conf for reading" << std::endl;
        return false;
    }

    // Open the backup file for writing (truncate existing content)
    std::ofstream backup_file(filename, std::ios::binary | std::ios::trunc);
    if (!backup_file) {
        std::cerr << "Error: Cannot create backup file: " << filename << std::endl;
        return false;
    }

    // Copy the entire content via stream buffer (efficient and safe)
    backup_file << resolv_conf.rdbuf();

    // Check that writing succeeded
    if (!backup_file) {
        std::cerr << "Error: Failed to write to backup file" << std::endl;
        return false;
    }

    // Close files (destructors will do this automatically, but for clarity)
    resolv_conf.close();
    backup_file.close();

    std::cout << "Current DNS settings saved to: " << filename << std::endl;
    return true;
}

// edit dns settings
bool set_dns(const std::string& dns) {
    // Validate IP address
    if (!isValidIP(dns)) {
        std::cerr << "Invalid DNS IP address: " << dns << std::endl;
        return false;
    }

    std::ofstream file("/etc/resolv.conf", std::ios::trunc);

    if (!file) {
        std::cerr << "Cannot open /etc/resolv.conf for writing\n";
        return false;
    }

    file << "nameserver " << dns << "\n";

    if (!file) {
        std::cerr << "Write error\n";
        return false;
    }

    return true;
}

// restore dns settings
bool restore_dns(const std::string& backup_file_path) {

    std::ifstream backup(backup_file_path, std::ios::binary);
    if (!backup) {
        std::cerr << "Backup file not found\n";
        return false;
    }

    std::ofstream resolv("/etc/resolv.conf", std::ios::binary | std::ios::trunc);
    if (!resolv) {
        std::cerr << "Cannot restore /etc/resolv.conf\n";
        return false;
    }

    resolv << backup.rdbuf();

    if (!resolv) {
        std::cerr << "Restore failed\n";
        return false;
    }

    std::cout << "DNS restored successfully\n";
    return true;
}

// Main function
int main(int argc, char* argv[]) {

    if (!isRoot()) {
        std::cerr << "Run as root!\n";
        return 1;
    }

    if (argc < 2) {
        std::cout << "Usage:\n";
        std::cout << "  set <dns>\n";
        std::cout << "  restore\n";
        return 1;
    }

    std::string command = argv[1];
    const std::string backup_path = "/etc/resolv.conf.backup";

    if (command == "set" && argc == 3) {

        if (!save_current_dns(backup_path))
            return 1;

        if (!set_dns(argv[2]))
            return 1;

        std::cout << "DNS changed successfully\n";
    }
    else if (command == "restore") {

        if (!restore_dns(backup_path))
            return 1;
    }
    else {
        std::cout << "Invalid command\n";
        return 1;
    }

    return 0;
}
