#pragma once

#include "hakoniwa/api/protocol.hpp"
#include "hakoniwa/pdu/rpc/rpc_services_server.hpp"
#include "hakoniwa/api/server_context.hpp"
#include <atomic> // Added for std::atomic

namespace hakoniwa::api {

class IServiceHandler {
public:
    virtual ~IServiceHandler() = default;
    virtual void handle(ServerContext& service_context, std::shared_ptr<hakoniwa::pdu::rpc::RpcServicesServer> service_rpc,  hakoniwa::pdu::rpc::RpcRequest& request) = 0;
    virtual void cancel() {
        is_canceled_ = true;
    }
    virtual bool is_canceled() const {
        return is_canceled_.load(); // Use .load() for atomic read
    }
    virtual void reset_canceled() {
        is_canceled_ = false;
    }
protected:
    std::atomic<bool> is_canceled_{false}; // Changed to std::atomic<bool>
};
} // namespace hakoniwa::api
