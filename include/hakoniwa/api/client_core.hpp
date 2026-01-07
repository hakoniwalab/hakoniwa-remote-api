#pragma once

#include "hakoniwa/api/protocol.hpp"
#include <memory>
#include <string>
#include "hakoniwa/pdu/rpc/rpc_services_client.hpp" // Directly include for full definition

namespace hakoniwa::api {

class ClientCore {
public:
  /**
   * @brief Construct a new Client Core object
   *
   * @param client_name The name of this client node.
   * @param config_path Path to the remote-api.json configuration file.
   */
  explicit ClientCore(std::string client_name, std::string config_path);
  ~ClientCore();

  /**
   * @brief Initializes the client core.
   * Parses the configuration and sets up the RPC client.
   * @return true if initialization is successful, false otherwise.
   */
  bool initialize();

  bool start();
  bool stop();

  /**
   * @brief Joins the simulation.
   * @return true on success, false on failure.
   */
  bool join();

  /**
   * @brief Get the current simulation state.
   * @param[out] state The current simulation state.
   * @return true on success, false on failure.
   */
  bool get_sim_state(HakoSimulationState &state);

  /**
   * @brief Send a simulation control command.
   * @param command The command to send (Start, Stop, Reset).
   * @return true on success, false on failure.
   */
  bool sim_control(HakoSimulationControlCommand command);

  /**
   * @brief Get the next event for this asset.
   * @param[out] event The event code.
   * @return true on success, false on failure.
   */
  bool get_event(HakoSimulationAssetEvent &event);

  /**
   * @brief Acknowledge that an event has been processed.
   * @param event The event that was processed.
   * @return true on success, false on failure.
   */
  bool ack_event(HakoSimulationAssetEvent event);

  /**
   * @brief Get the last error message.
   * @return A string containing the last error message.
   */
  std::string last_error() const;

private:
  void set_last_error(const std::string &msg);

  std::string client_name_;
  std::string config_path_;
  std::string server_node_id_;
  std::string rpc_config_path_;
  mutable std::string last_error_;

  std::shared_ptr<hakoniwa::pdu::rpc::RpcServicesClient> rpc_client_; // Corrected class name
  bool is_initialized_ = false;
};

} // namespace hakoniwa::api