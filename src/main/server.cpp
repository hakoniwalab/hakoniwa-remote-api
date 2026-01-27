#include "hakoniwa/api/server_core.hpp"


int main(int argc, const char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <path_to_remote_api.json> <server_nodeId> <path_to_endpoints.json>" << std::endl;
        return 1;
    }
    std::string config_path = argv[1];
    std::string nodeId = argv[2];
    std::string endpoints_config_path = argv[3];
    std::shared_ptr<hakoniwa::pdu::EndpointContainer> endpoint_container =
        std::make_shared<hakoniwa::pdu::EndpointContainer>(nodeId, endpoints_config_path);
    // Example usage of ServerCore
    hakoniwa::api::ServerCore server(config_path, nodeId, true);

    if (endpoint_container->initialize() != HakoPduErrorType::HAKO_PDU_ERR_OK) {
        std::cerr << "Failed to initialize EndpointContainer: " << endpoint_container->last_error() << std::endl;
        return 1;
    }

    if (!server.initialize(endpoint_container)) {
        std::cerr << "Server initialization failed: " << server.last_error() << std::endl;
        return 1;
    }
    if (!server.initialize_rpc_services()) {
        std::cerr << "Server RPC initialization failed: " << server.last_error() << std::endl;
        return 1;
    }
    if (endpoint_container->start_all() != HakoPduErrorType::HAKO_PDU_ERR_OK) {
        std::cerr << "Failed to start all endpoints: " << endpoint_container->last_error() << std::endl;
        return 1;
    }
    if (!server.start()) {
        std::cerr << "Server start failed: " << server.last_error() << std::endl;
        return 1;
    }
    while (!endpoint_container->is_running_all()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Run the server for some time (e.g., 10 seconds)
    while (server.is_running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (!server.stop()) {
        std::cerr << "Server stop failed: " << server.last_error() << std::endl;
        return 1;
    }
    endpoint_container->stop_all();

    return 0;
}
