#include <iostream>
#include <string>
#include <chrono>
#include <functional>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>
#include <iomanip>
#include <cstring>

// Enum for protocols
enum class Protocol {
    HTTP,
    HTTPS,
    FTP,
    SSH,
    UNKNOWN
};

// تحويل اسم البروتوكول إلى القيمة المطلوبة
Protocol get_protocol_from_string(const std::string& protocol_str) {
    if (protocol_str == "http") return Protocol::HTTP;
    if (protocol_str == "https") return Protocol::HTTPS;
    if (protocol_str == "ftp") return Protocol::FTP;
    if (protocol_str == "ssh") return Protocol::SSH;
    return Protocol::UNKNOWN;
}

// تحويل البروتوكول إلى string
std::string protocol_to_string(Protocol protocol) {
    switch (protocol) {
        case Protocol::HTTP: return "HTTP";
        case Protocol::HTTPS: return "HTTPS";
        case Protocol::FTP: return "FTP";
        case Protocol::SSH: return "SSH";
        default: return "UNKNOWN";
    }
}

// ScanResult class
class ScanResult {
public:
    ScanResult(Protocol protocol, std::string destination, std::string status = "UNKNOWN", double response_time = 0.0)
        : protocol_(protocol), destination_(destination), status_(status), response_time_(response_time) {}

    std::string toString() const {
        return "ScanResult(protocol=" + protocol_to_string(protocol_) + ", destination=" + destination_ + ", status=" + status_ +
               ", response_time=" + std::to_string(response_time_) + "ms)";
    }

private:
    Protocol protocol_;
    std::string destination_;
    std::string status_;
    double response_time_;
};

// NetworkScanner class
class NetworkScanner {
public:
    NetworkScanner(std::string destination) : destination_(destination) {}

    ScanResult scan(Protocol protocol) {
        switch (protocol) {
            case Protocol::HTTP:
                return perform_scan(80, protocol);
            case Protocol::HTTPS:
                return perform_scan(443, protocol);
            case Protocol::FTP:
                return perform_scan(21, protocol);
            case Protocol::SSH:
                return perform_scan(22, protocol);
            default:
                return ScanResult(protocol, destination_, "UNSUPPORTED");
        }
    }

private:
    std::string destination_;

    ScanResult perform_scan(int port, Protocol protocol) {
        auto start_time = std::chrono::high_resolution_clock::now();
        std::string status = "DOWN";
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            return ScanResult(protocol, destination_, status, 0.0);
        }

        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        if (inet_pton(AF_INET, destination_.c_str(), &server_addr.sin_addr) <= 0) {
            close(sock);
            return ScanResult(protocol, destination_, status, 0.0);
        }

        if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == 0) {
            status = "UP";
        }
        close(sock);

        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end_time - start_time;
        return ScanResult(protocol, destination_, status, duration.count() * 1000);
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <protocol> <domain>" << std::endl;
        return 1;
    }

    std::string protocol_str = argv[1];
    std::string domain = argv[2];

    Protocol protocol = get_protocol_from_string(protocol_str);
    if (protocol == Protocol::UNKNOWN) {
        std::cerr << "Unknown protocol: " << protocol_str << std::endl;
        return 1;
    }

    NetworkScanner scanner(domain);
    auto result = scanner.scan(protocol);
    std::cout << result.toString() << std::endl;

    return 0;
}
