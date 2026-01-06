#pragma once
#include <mutex>

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
private:
    std::mutex mtx_;
    ServerServiceContextStatus status_{ServerServiceContextStatus::SERVER_SERVICE_NOT_READY};
};

} // namespace hakoniwa::api