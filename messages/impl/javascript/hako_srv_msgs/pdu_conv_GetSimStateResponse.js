import * as PduUtils from '../pdu_utils.js';
import { GetSimStateResponse } from './pdu_jstype_GetSimStateResponse.js';


/**
 * Deserializes a binary PDU into a GetSimStateResponse object.
 * @param {ArrayBuffer} binary_data
 * @returns { GetSimStateResponse }
 */
export function pduToJs_GetSimStateResponse(binary_data) {
    const js_obj = new GetSimStateResponse();
    const meta_parser = new PduUtils.PduMetaDataParser();
    const meta = meta_parser.load_pdu_meta(binary_data);
    if (meta === null) {
        throw new Error("Invalid PDU binary data: MetaData not found or corrupted");
    }
    binary_read_recursive_GetSimStateResponse(meta, binary_data, js_obj, meta.base_off);
    return js_obj;
}

export function binary_read_recursive_GetSimStateResponse(meta, binary_data, js_obj, base_off) {
    const view = new DataView(binary_data);
    const littleEndian = true;
    // member: sim_state, type: uint32 (primitive)

    
    {
        const bin = PduUtils.readBinary(binary_data, base_off + 0, 4);
        js_obj.sim_state = PduUtils.binToValue("uint32", bin);
    }
    
    // member: master_time, type: int64 (primitive)

    
    {
        const bin = PduUtils.readBinary(binary_data, base_off + 8, 8);
        js_obj.master_time = PduUtils.binToValue("int64", bin);
    }
    
    // member: is_pdu_created, type: bool (primitive)

    
    {
        const bin = PduUtils.readBinary(binary_data, base_off + 16, 4);
        js_obj.is_pdu_created = PduUtils.binToValue("bool", bin);
    }
    
    // member: is_simulation_mode, type: bool (primitive)

    
    {
        const bin = PduUtils.readBinary(binary_data, base_off + 20, 4);
        js_obj.is_simulation_mode = PduUtils.binToValue("bool", bin);
    }
    
    // member: is_pdu_sync_mode, type: bool (primitive)

    
    {
        const bin = PduUtils.readBinary(binary_data, base_off + 24, 4);
        js_obj.is_pdu_sync_mode = PduUtils.binToValue("bool", bin);
    }
    
    return js_obj;
}

/**
 * Serializes a GetSimStateResponse object into a binary PDU.
 * @param { GetSimStateResponse } js_obj
 * @returns {ArrayBuffer}
 */
export function jsToPdu_GetSimStateResponse(js_obj) {
    const base_allocator = new PduUtils.DynamicAllocator();
    const bw_container = new PduUtils.BinaryWriterContainer(new PduUtils.PduMetaData());

    binary_write_recursive_GetSimStateResponse(0, bw_container, base_allocator, js_obj);

    const base_data_size = base_allocator.size();
    const heap_data_size = bw_container.heap_allocator.size();
    
    bw_container.meta.heap_off = PduUtils.PDU_META_DATA_SIZE + base_data_size;
    bw_container.meta.total_size = bw_container.meta.heap_off + heap_data_size;

    const final_buffer = new ArrayBuffer(bw_container.meta.total_size);
    const final_view = new Uint8Array(final_buffer);

    PduUtils.writeBinary(final_view, 0, bw_container.meta.to_bytes());
    PduUtils.writeBinary(final_view, bw_container.meta.base_off, base_allocator.toArray());
    PduUtils.writeBinary(final_view, bw_container.meta.heap_off, bw_container.heap_allocator.toArray());

    return final_buffer;
}

export function binary_write_recursive_GetSimStateResponse(parent_off, bw_container, allocator, js_obj) {
    const littleEndian = true;
    // member: sim_state, type: uint32 (primitive)

    
    {
        const bin = PduUtils.typeToBin("uint32", js_obj.sim_state, 4);
        allocator.add(bin, parent_off + 0);
    }
    
    // member: master_time, type: int64 (primitive)

    
    {
        const bin = PduUtils.typeToBin("int64", js_obj.master_time, 8);
        allocator.add(bin, parent_off + 8);
    }
    
    // member: is_pdu_created, type: bool (primitive)

    
    {
        const bin = PduUtils.typeToBin("bool", js_obj.is_pdu_created, 4);
        allocator.add(bin, parent_off + 16);
    }
    
    // member: is_simulation_mode, type: bool (primitive)

    
    {
        const bin = PduUtils.typeToBin("bool", js_obj.is_simulation_mode, 4);
        allocator.add(bin, parent_off + 20);
    }
    
    // member: is_pdu_sync_mode, type: bool (primitive)

    
    {
        const bin = PduUtils.typeToBin("bool", js_obj.is_pdu_sync_mode, 4);
        allocator.add(bin, parent_off + 24);
    }
    
}
