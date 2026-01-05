using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using hakoniwa.pdu.interfaces;

namespace hakoniwa.pdu.msgs.hako_srv_msgs
{
    public class AckEventResponse
    {
        protected internal readonly IPdu _pdu;
        public IPdu GetPdu() { return _pdu; }

        public AckEventResponse(IPdu pdu)
        {
            _pdu = pdu;
        }
        public uint ack_code
        {
            get => _pdu.GetData<uint>("ack_code");
            set => _pdu.SetData("ack_code", value);
        }
    }
}
