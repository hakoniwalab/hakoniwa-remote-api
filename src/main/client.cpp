#include "hakoniwa/api/client_core.hpp"

int main(int argc, const char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <path_to_remote_api.json> <nodeId>" << std::endl;
        return 1;
    }
    std::string config_path = argv[1];
    std::string node_id = argv[2];
    // Example usage of ClientCore
    hakoniwa::api::ClientCore client(node_id, config_path);
    if (!client.initialize()) {
        std::cerr << "Client initialization failed: " << client.last_error() << std::endl;
        return 1;
    }
    if (!client.start()) {
        std::cerr << "Client start failed: " << client.last_error() << std::endl;
        return 1;
    }
    while (!client.is_pdu_end_point_running()) {
        std::cout << "Waiting for PDU endpoint to be running..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // keyboard input for api calls
    std::cout << "Client joined the simulation. Press Enter to stop..." << std::endl;
    while (true) {
        // get keyboard input
        std::string input;
        std::getline(std::cin, input);
        // Join the simulation
        if (input == "join") {
            std::cout << "Joining the simulation..." << std::endl;
            if (!client.join()) {
                std::cerr << "Join failed: " << client.last_error() << std::endl;
            } else {
                std::cout << "Joined the simulation successfully." << std::endl;
            }
        }
        else if (input == "state") {
            hakoniwa::api::HakoSimulationStateInfo state;
            if (!client.get_sim_state(state)) {
                std::cerr << "GetSimState failed: " << client.last_error() << std::endl;
            } else {
                std::cout << "Simulation State: time=" << state.master_time
                          << " usec, status=" << static_cast<int>(state.sim_state) << std::endl;
                std::cout << "  is_pdu_created=" << state.is_pdu_created
                          << ", is_simulation_mode=" << state.is_simulation_mode
                          << ", is_pdu_sync_mode=" << state.is_pdu_sync_mode << std::endl;
            }
        }
        else if (input == "g:event") {
            hakoniwa::api::HakoSimulationAssetEvent event;
            if (!client.get_event(event)) {
                std::cerr << "GetEvent failed: " << client.last_error() << std::endl;
            } else {
                std::cout << "Received event: " << static_cast<int>(event) << std::endl;
            }
        }
        else if ((input == "c:start") ||
                 (input == "c:stop") ||
                 (input == "c:reset")) {
            std::string command_name = input.substr(2);
            std::cout << "Sending control command: " << command_name << std::endl;
            hakoniwa::api::HakoSimulationControlCommand command;
            if (command_name == "start") {
                command = hakoniwa::api::HakoSimulationControlCommand::HakoSimControl_Start;
            } else if (command_name == "stop") {
                command = hakoniwa::api::HakoSimulationControlCommand::HakoSimControl_Stop;
            } else if (command_name == "reset") {
                command = hakoniwa::api::HakoSimulationControlCommand::HakoSimControl_Reset;
            } else {
                std::cerr << "Unknown command: " << command_name << std::endl;
                continue;
            }
            if (!client.sim_control(command)) {
                std::cerr << "SimControl " << command_name << " failed: " << client.last_error() << std::endl;
            } else {
                std::cout << command_name << " command sent successfully." << std::endl;
            }
        }
        else if ((input == "a:start") ||
                 (input == "a:stop") ||
                 (input == "a:reset")) {
            std::string event_name = input.substr(2);
            std::cout << "Acknowledging event: " << event_name << std::endl;
            hakoniwa::api::HakoSimulationAssetEvent event;
            if (event_name == "start") {
                event = hakoniwa::api::HakoSimulationAssetEvent::HakoSimAssetEvent_Start;
            } else if (event_name == "stop") {
                event = hakoniwa::api::HakoSimulationAssetEvent::HakoSimAssetEvent_Stop;
            } else if (event_name == "reset") {
                event = hakoniwa::api::HakoSimulationAssetEvent::HakoSimAssetEvent_Reset;
            } else {
                std::cerr << "Unknown event: " << event_name << std::endl;
                continue;
            }
            if (!client.ack_event(event)) {
                std::cerr << "AckEvent " << event_name << " failed: " << client.last_error() << std::endl;
            } else {
                std::cout << event_name << " event acknowledged successfully." << std::endl;
            }
        }
        else if (input == "q" || input == "quit" || input == "exit") {
            std::cout << "Exiting..." << std::endl;
            break;
        }
    }
    
    if (!client.stop()) {
        std::cerr << "Client stop failed: " << client.last_error() << std::endl;
        return 1;
    }

    return 0;
}