#pragma once

#include "iservice_handler.hpp"

namespace hakoniwa::api {

class JoinHandler : public IServiceHandler {
public:
    ~JoinHandler() = default;
    void handle(std::shared_ptr<hakoniwa::pdu::rpc::RpcServicesServer> service_rpc,  const hakoniwa::pdu::rpc::RpcRequest& request) override;
};
} // namespace hakoniwa::api
