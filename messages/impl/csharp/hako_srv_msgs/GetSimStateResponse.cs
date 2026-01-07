using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using hakoniwa.pdu.interfaces;

namespace hakoniwa.pdu.msgs.hako_srv_msgs
{
    public class GetSimStateResponse
    {
        protected internal readonly IPdu _pdu;
        public IPdu GetPdu() { return _pdu; }

        public GetSimStateResponse(IPdu pdu)
        {
            _pdu = pdu;
        }
        public uint sim_state
        {
            get => _pdu.GetData<uint>("sim_state");
            set => _pdu.SetData("sim_state", value);
        }
        public long master_time
        {
            get => _pdu.GetData<long>("master_time");
            set => _pdu.SetData("master_time", value);
        }
        public bool is_pdu_created
        {
            get => _pdu.GetData<bool>("is_pdu_created");
            set => _pdu.SetData("is_pdu_created", value);
        }
        public bool is_simulation_mode
        {
            get => _pdu.GetData<bool>("is_simulation_mode");
            set => _pdu.SetData("is_simulation_mode", value);
        }
        public bool is_pdu_sync_mode
        {
            get => _pdu.GetData<bool>("is_pdu_sync_mode");
            set => _pdu.SetData("is_pdu_sync_mode", value);
        }
    }
}
