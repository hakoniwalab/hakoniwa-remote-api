
import json
import os
import argparse

def get_pdu_size(pdu_size_path, service_type, packet_type):
    """
    Constructs the path to the PDU size file and reads the size.
    Example: <pdu_size_path>/hako_srv_msgs/SimControlRequestPacket.txt
    """
    # service_type is "hako_srv_msgs/SimControl", extract the parts
    type_folder, type_name = service_type.split('/')
    
    # Construct filename, e.g., SimControlRequestPacket.txt
    file_name = f"{type_name}{packet_type}Packet.txt"
    
    # Construct the full path
    size_file_path = os.path.join(pdu_size_path, type_folder, file_name)
    
    if not os.path.exists(size_file_path):
        print(f"Warning: Size file not found: {size_file_path}")
        return None
        
    with open(size_file_path, 'r') as f:
        return int(f.read().strip())

def update_pdu_sizes(rpc_json_path, pdu_size_path):
    """
    Updates the pduSize in the rpc.json file based on the size files.
    """
    with open(rpc_json_path, 'r') as f:
        rpc_data = json.load(f)

    for service in rpc_data.get("services", []):
        service_type = service.get("type")
        if not service_type:
            continue

        print(f"Processing service: {service['name']} ({service_type})")

        # Update server (Request) size
        server_size = get_pdu_size(pdu_size_path, service_type, "Request")
        if server_size is not None:
            service["pduSize"]["server"]["baseSize"] = server_size
            print(f"  Updated server baseSize to: {server_size}")

        # Update client (Response) size
        client_size = get_pdu_size(pdu_size_path, service_type, "Response")
        if client_size is not None:
            service["pduSize"]["client"]["baseSize"] = client_size
            print(f"  Updated client baseSize to: {client_size}")

    with open(rpc_json_path, 'w') as f:
        json.dump(rpc_data, f, indent=2)

    print(f"\nSuccessfully updated {rpc_json_path}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Update pduSize in rpc.json from pdu_size files.")
    parser.add_argument("rpc_json", help="Path to the rpc.json file.")
    parser.add_argument("pdu_size_dir", help="Path to the directory containing pdu_size files (e.g., messages/impl/pdu_size).")
    
    args = parser.parse_args()
    
    if not os.path.isfile(args.rpc_json):
        print(f"Error: rpc.json file not found at {args.rpc_json}")
    elif not os.path.isdir(args.pdu_size_dir):
        print(f"Error: pdu_size directory not found at {args.pdu_size_dir}")
    else:
        update_pdu_sizes(args.rpc_json, args.pdu_size_dir)
