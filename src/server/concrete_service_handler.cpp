#include "concrete_service_handler.hpp"
#include "hakoniwa/hako_capi.h"

namespace hakoniwa::api {

void JoinHandler::handle(
    ServerContext& service_context,
    std::shared_ptr<hakoniwa::pdu::rpc::RpcServicesServer> service_rpc,
    const hakoniwa::pdu::rpc::RpcRequest& request)
{
    std::cout << "Handling join request from client: "
              << request.client_name << std::endl;

    Hako_int32 result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_OK;

    {
        std::lock_guard<std::mutex> lock(service_context.get_mutex());
        if (service_context.get_status()
            != ServerServiceContextStatus::SERVER_SERVICE_NOT_READY)
        {
            result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_ERROR;
        } else {
            service_context.set_status(
                ServerServiceContextStatus::SERVER_SERVICE_STARTED);
        }
    }

    hakoniwa::pdu::rpc::PduData reply_pdu;
    service_rpc->create_reply_buffer(
        request.header,
        hakoniwa::pdu::rpc::HAKO_SERVICE_STATUS_DONE,
        result_code,
        reply_pdu);

    service_rpc->send_reply(request.header, reply_pdu);
}

}