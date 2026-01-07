#pragma once

#include "hakoniwa/api/protocol.hpp"
#include "hakoniwa/pdu/rpc/rpc_services_server.hpp"
#include "hakoniwa/api/server_context.hpp"

namespace hakoniwa::api {

class IServiceHandler {
public:
    virtual ~IServiceHandler() = default;
    virtual void handle(ServerContext& service_context, std::shared_ptr<hakoniwa::pdu::rpc::RpcServicesServer> service_rpc,  hakoniwa::pdu::rpc::RpcRequest& request) = 0;
    virtual void cancel() {
        is_canceled_ = true;
    }
    virtual bool is_canceled() const {
        return is_canceled_;
    }
    virtual void reset_canceled() {
        is_canceled_ = false;
    }
protected:
    bool is_canceled_{false};
};
} // namespace hakoniwa::api
