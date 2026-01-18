#pragma once

#include "hakoniwa/api/iservice_handler.hpp"
#include <functional>

namespace hakoniwa::api {

class JoinHandler : public IServiceHandler {
public:
    ~JoinHandler() = default;
    void handle(ServerContext& service_context, std::shared_ptr<hakoniwa::pdu::rpc::RpcServicesServer> service_rpc,  hakoniwa::pdu::rpc::RpcRequest& request) override;
};

class GetSimStateHandler : public IServiceHandler {
public:
    ~GetSimStateHandler() = default;
    void handle(ServerContext& service_context, std::shared_ptr<hakoniwa::pdu::rpc::RpcServicesServer> service_rpc,  hakoniwa::pdu::rpc::RpcRequest& request) override;
};

class SimControlHandler : public IServiceHandler {
public:
    ~SimControlHandler() = default;
    void handle(ServerContext& service_context, std::shared_ptr<hakoniwa::pdu::rpc::RpcServicesServer> service_rpc,  hakoniwa::pdu::rpc::RpcRequest& request) override;
};

class GetEventHandler : public IServiceHandler {
public:
    ~GetEventHandler() = default;
    void handle(ServerContext& service_context, std::shared_ptr<hakoniwa::pdu::rpc::RpcServicesServer> service_rpc,  hakoniwa::pdu::rpc::RpcRequest& request) override;
};

class AckEventHandler : public IServiceHandler {
public:
    explicit AckEventHandler(std::function<HakoPduErrorType()> post_start_cb = {})
        : post_start_cb_(std::move(post_start_cb)) {}
    ~AckEventHandler() = default;
    void handle(ServerContext& service_context, std::shared_ptr<hakoniwa::pdu::rpc::RpcServicesServer> service_rpc,  hakoniwa::pdu::rpc::RpcRequest& request) override;
private:
    std::function<HakoPduErrorType()> post_start_cb_;
};

} // namespace hakoniwa::api
