using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using hakoniwa.pdu.interfaces;

namespace hakoniwa.pdu.msgs.hako_srv_msgs
{
    public class GetEventResponse
    {
        protected internal readonly IPdu _pdu;
        public IPdu GetPdu() { return _pdu; }

        public GetEventResponse(IPdu pdu)
        {
            _pdu = pdu;
        }
        public uint event_code
        {
            get => _pdu.GetData<uint>("event_code");
            set => _pdu.SetData("event_code", value);
        }
    }
}
