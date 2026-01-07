using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using hakoniwa.pdu.interfaces;

namespace hakoniwa.pdu.msgs.hako_srv_msgs
{
    public class AckEventRequest
    {
        protected internal readonly IPdu _pdu;
        public IPdu GetPdu() { return _pdu; }

        public AckEventRequest(IPdu pdu)
        {
            _pdu = pdu;
        }
        public string name
        {
            get => _pdu.GetData<string>("name");
            set => _pdu.SetData("name", value);
        }
        public uint event_code
        {
            get => _pdu.GetData<uint>("event_code");
            set => _pdu.SetData("event_code", value);
        }
        public uint result_code
        {
            get => _pdu.GetData<uint>("result_code");
            set => _pdu.SetData("result_code", value);
        }
    }
}
