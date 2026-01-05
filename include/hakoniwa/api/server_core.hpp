#pragma once

#include <atomic>
#include <string>
#include <string_view>
#include <memory>

namespace hakoniwa::time_source { class ITimeSource; }

namespace hakoniwa::api {

class ServerCore {
public:
    ServerCore(std::string config_path);
    ~ServerCore(); // stop() して join する前提

    bool start();  // non-blocking
    bool stop();   // idempotent

    bool is_running() const noexcept { return is_running_.load(); }
    std::string_view last_error() const noexcept;

private:
    bool serve();
    void set_last_error(std::string msg);

private:
    std::string config_path_;
    std::string node_id_;
    std::string last_error_;

    std::atomic<bool> is_running_{false};
    std::atomic<bool> stop_requested_{false};

    std::unique_ptr<hakoniwa::time_source::ITimeSource> time_source_;
};

} // namespace hakoniwa::api
