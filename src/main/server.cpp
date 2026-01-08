#include "hakoniwa/api/server_core.hpp"


int main(int argc, const char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <path_to_remote_api.json> <server_nodeId>" << std::endl;
        return 1;
    }
    // Example usage of ServerCore
    hakoniwa::api::ServerCore server(argv[1], argv[2], true);
    if (!server.initialize()) {
        std::cerr << "Server initialization failed: " << server.last_error() << std::endl;
        return 1;
    }
    if (!server.start()) {
        std::cerr << "Server start failed: " << server.last_error() << std::endl;
        return 1;
    }

    // Run the server for some time (e.g., 10 seconds)
    while (server.is_running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (!server.stop()) {
        std::cerr << "Server stop failed: " << server.last_error() << std::endl;
        return 1;
    }

    return 0;
}