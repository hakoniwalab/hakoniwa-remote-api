#pragma once

#include "hakoniwa/api/iservice_handler.hpp"

namespace hakoniwa::api {

class JoinHandler : public IServiceHandler {
public:
    ~JoinHandler() = default;
    void handle(ServerContext& service_context, std::shared_ptr<hakoniwa::pdu::rpc::RpcServicesServer> service_rpc,  const hakoniwa::pdu::rpc::RpcRequest& request) override;
    void cancel() override;
};
} // namespace hakoniwa::api
