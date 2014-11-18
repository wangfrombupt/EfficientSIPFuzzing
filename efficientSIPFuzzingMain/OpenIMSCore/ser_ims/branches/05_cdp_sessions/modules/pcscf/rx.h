#ifndef RX_H_
#define RX_H_


AAAMessage *Rx_AAR(struct sip_msg *req, struct sip_msg *res, int tag);
AAAMessage* Rx_STR(struct sip_msg* msg, int tag);
int Rx_AAA(AAAMessage *dia_msg);

#endif /*RX_H_*/
