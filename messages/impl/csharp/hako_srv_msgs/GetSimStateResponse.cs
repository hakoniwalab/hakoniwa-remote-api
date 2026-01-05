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
    }
}
