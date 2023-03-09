#include "UiNetlibInterfaces.h"
#include "logging.h"
#include "dataio.h"

std::unique_ptr<NetOps> netOps1;

class CallBacks : public UiCallbacks{
public:

    void bindSucceeded() override {

    }

    void newAuthMessage(std::string peerName, std::unique_ptr<AuthMessage> authMsg) override {
        getLogger()->info("MSG+AUTH: {}", authMsg->name);
    }

    void newTextMessage(std::string peerName, std::unique_ptr<TextMessage> txtMsg) override {
        getLogger()->info("MSG+TXT: {}", txtMsg->text);
        TextMessage txt(txtMsg->text);
        netOps1->sendMessage(peerName, txt);
    }

    void newImageMessage(std::string peerName, std::unique_ptr<ImageMessage> imgMsg) override {
        auto image = imgMsg->image.get();
        getLogger()->info("MSG+IMG: {}", spdlog::to_hex(*image));
    }

    void peerDisconnected(const std::string peerName) override {
        getLogger()->info("Peer {} disconnected!", peerName);
    }

};

int main(){
    init_logging();
    getLogger()->info("Start of the program....");
    UiCallbacks *cl(new CallBacks());
    netOps1 = createNetworking(1234, cl);
    std::unique_ptr<NetOps> netOps2 = createNetworking(2348, cl);
    sleep(1);
    Peer p = {"two_three", "127.0.0.1", 2348};
    netOps1->connectPeer(p);
    AuthMessage auth("MyName!");
    netOps1->sendMessage(p.name, auth);
    TextMessage text("HellOOO,FROMMM MEEE!");
    netOps1->sendMessage(p.name, text);
    std::vector<uint8_t> bytes= {22,33,43,55,66,77,11,12,32,88};
    auto imgBytes = std::make_shared<std::vector<uint8_t>>(bytes.begin(), bytes.end());
    ImageMessage img(imgBytes);
    netOps1->sendMessage(p.name, img);
    //netOps2->stopListening();

    return 0;
}

