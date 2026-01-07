#pragma once
#include <mutex>
#include <nlohmann/json.hpp>

namespace hakoniwa::api {
enum class ServerServiceContextStatus {
    SERVER_SERVICE_NOT_READY = 0,
    SERVER_SERVICE_STARTED,
    SERVER_SERVICE_ERROR,
    SERVER_SERVICE_NUM
};

class ServerContext {
public:
    ServerContext() = default;
    ~ServerContext() = default;
    ServerServiceContextStatus get_status() const {
        return status_;
    }
    void set_status(ServerServiceContextStatus status) {
        status_ = status;
    }
    std::mutex& get_mutex() {
        return mtx_;
    }
    nlohmann::json& get_config() {
        return config_;
    }
    const std::string& get_client_node_id() const {
        return client_node_id_;
    }
    void set_client_node_id(const std::string& node_id) {
        client_node_id_ = node_id;
    }
private:
    std::mutex mtx_;
    ServerServiceContextStatus status_{ServerServiceContextStatus::SERVER_SERVICE_NOT_READY};
    nlohmann::json config_;
    std::string client_node_id_;
};

} // namespace hakoniwa::api