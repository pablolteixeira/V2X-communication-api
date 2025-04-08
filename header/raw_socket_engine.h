class RawSocketEngine 
{
protected:
    RawSocketEngine() {
    }
    
    ~RawSocketEngine() {
    }
    
    int raw_send(Ethernet::Address dst, Ethernet::Protocol prot, const void* data, unsigned int size) {
    }
    
    int raw_receive(Ethernet::Address* src, Ethernet::Protocol* prot, void* data, unsigned int size) {
    }
    
protected:
};