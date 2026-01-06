#pragma once

#include "hakoniwa/api/protocol.hpp"
#include "hakoniwa/pdu/rpc/rpc_services_server.hpp"

namespace hakoniwa::api {

class IServiceHandler {
public:
    virtual ~IServiceHandler() = default;
    virtual void handle(std::shared_ptr<hakoniwa::pdu::rpc::RpcServicesServer> service_rpc,  const hakoniwa::pdu::rpc::RpcRequest& request) = 0;
};
} // namespace hakoniwa::api
