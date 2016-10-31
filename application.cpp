#include "application.h"
#include "OICServer.h"
#include "QQmlContext"
#include "QDebug"
#include <poll.h>
#include <unistd.h>

extern uint64_t get_current_ms();
Application::Application(int &argc, char *argv[]) : QGuiApplication(argc, argv)
{
    server = 0;
    engine.rootContext()->setContextProperty("app", this);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    QObject* root = engine.rootObjects().first();
    QObject *frontSlider= root->findChild<QObject*>("frontSlider");
    frontSlider->setProperty("value", 0);
    QObject *backSlider= root->findChild<QObject*>("backSlider");
    backSlider->setProperty("value", 0);
    QObject *tableSlider= root->findChild<QObject*>("tableSlider");
    tableSlider->setProperty("value", 0);
    QObject *redSlider= root->findChild<QObject*>("redSlider");
    redSlider->setProperty("value", 0);
    QObject *blueSlider= root->findChild<QObject*>("blueSlider");
    blueSlider->setProperty("value", 0);
    QObject *greenSlider= root->findChild<QObject*>("greenSlider");
    greenSlider->setProperty("value", 0);

    QObject *ambientPowerSlider = root->findChild<QObject*>("ambientPowerSlider");
    ambientPowerSlider->setProperty("value", 0);

    server = new OICServer("Lampa", "0685B960-736F-46F7-BEC0-9E6CBD61ADC2", [&](COAPPacket* packet){
        this->send_packet(packet);
    });

    cbor* ambientInitial = new cbor(CBOR_TYPE_MAP);
    ambientInitial->append("rt", "oic.r.colour.rgb");
    ambientInitial->append("dimmingSetting", "0,0,0");


    cbor* frontInitial = new cbor(CBOR_TYPE_MAP);
    frontInitial->append("rt", "oic.r.light.dimming");
    frontInitial->append("dimmingSetting", (long long) 0);
    frontInitial->append("range", "0,255");

    cbor* backInitial = new cbor(CBOR_TYPE_MAP);
    backInitial->append("rt", "oic.r.light.dimming");
    backInitial->append("dimmingSetting",(long long) 0);
    backInitial->append("range", "0,255");


    cbor* tableInitial = new cbor(CBOR_TYPE_MAP);
    tableInitial->append("rt", "oic.r.light.dimming");
    tableInitial->append("dimmingSetting", (long long)0);
    tableInitial->append("range", "0,255");


    cbor* ambientPowerInitial = new cbor(CBOR_TYPE_MAP);
    ambientPowerInitial->append("rt", "oic.r.light.dimming");
    ambientPowerInitial->append("dimmingSetting", (long long)0);
    ambientPowerInitial->append("range", "0,100");


    OICResource* front = new OICResource("/lampa/front", "oic.r.light.dimming","oic.if.rw", [=](cbor data){
        frontInitial->toMap()->insert("dimmingSetting", data.getMapValue("dimmingSetting")); //update dimming settings
        int val = frontInitial->getMapValue("dimmingSetting").toInt();

        qDebug() << "Front updated" << val;
        frontSlider->setProperty("value",val);
    }, frontInitial);

    OICResource* back = new OICResource("/lampa/back", "oic.r.light.dimming","oic.if.rw", [=](cbor data){
        backInitial->toMap()->insert("dimmingSetting", data.getMapValue("dimmingSetting"));
        int val = data.getMapValue("dimmingSetting").toInt();

        qDebug() << "Back updated" << val;

        backSlider->setProperty("value",val);

    }, backInitial);

    OICResource* table = new OICResource("/lampa/table", "oic.r.light.dimming","oic.if.rw", [=](cbor data){
        tableInitial->toMap()->insert("dimmingSetting", data.getMapValue("dimmingSetting"));
        int val = data.getMapValue("dimmingSetting").toInt();
        qDebug() << "Table updated" << val;
        tableSlider->setProperty("value",val);

    }, tableInitial);

    OICResource* ambientPower = new OICResource("/lampa/ambientPower", "oic.r.light.dimming","oic.if.rw", [=](cbor data){
        ambientPowerInitial->toMap()->insert("dimmingSetting", data.getMapValue("dimmingSetting"));
        int val = data.getMapValue("dimmingSetting").toInt();
        qDebug() << "Ambient power updated" << val;
        ambientPowerSlider->setProperty("value",val);

    }, ambientPowerInitial);

    OICResource* ambient = new OICResource("/lampa/ambient", "oic.r.colour.rgb","oic.if.rw", [=](cbor data){
        ambientInitial->toMap()->insert("dimmingSetting", data.getMapValue("dimmingSetting"));
        List<String> vals = data.getMapValue("dimmingSetting").toString().split(",");

        if (vals.size() ==3){
            int red = atoi(vals.at(0).c_str());
            int green = atoi(vals.at(1).c_str());
            int blue = atoi(vals.at(2).c_str());

            qDebug() << "Ambient updated" << red << green << blue;

            redSlider->setProperty("value",red);
            greenSlider->setProperty("value",green);
            blueSlider->setProperty("value",blue);
        }else{
            qWarning() << "received invalid message";
        }
    }, ambientInitial);

    server->addResource(front);
    server->addResource(back);
    server->addResource(table);
    server->addResource(ambientPower);
    server->addResource(ambient);

    server->start();





    m_running = true;
    pthread_create(&m_thread, NULL, &Application::run, this);
    pthread_create(&m_discoveryThread, NULL, &Application::runDiscovery, this);
}
bool Application::isRunning(){
    return m_running;
}


Application::~Application(){
    m_running = false;
    pthread_join(m_thread, 0);
    pthread_join(m_discoveryThread, 0);

    delete server;


}

String Application::convertAddress(sockaddr_in a){
    char addr[30];
    sprintf(addr, "%d.%d.%d.%d %d",
            (uint8_t) (a.sin_addr.s_addr),
            (uint8_t) (a.sin_addr.s_addr >> 8),
            (uint8_t) (a.sin_addr.s_addr >> 16 ),
            (uint8_t) (a.sin_addr.s_addr >> 24),
            htons(a.sin_port));

    return addr;
}

void* Application::run(void* param){
    Application* a = (Application*) param;
    OICServer* oic_server = a->getServer();

    const int on = 1;
    int fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    a->setSocketFd(fd);

    struct sockaddr_in serv,client;

    serv.sin_family = AF_INET;
    serv.sin_port = 0;
    serv.sin_addr.s_addr = htonl(INADDR_ANY);

    uint8_t buffer[1024];
    socklen_t l = sizeof(client);
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        qDebug("Unable to set reuse");
        return 0;
    }
    if( bind(fd, (struct sockaddr*)&serv, sizeof(serv) ) == -1)
    {
        qDebug("Unable to bind");
        return 0;
    }

    struct pollfd pfd;
    int res;

    pfd.fd = fd;
    pfd.events = POLLIN;
    uint64_t lastTick = get_current_ms();

    size_t rc;
    while(1){
        rc = poll(&pfd, 1, 20); // 1000 ms timeout
        if (rc >0){
            rc = recvfrom(fd,buffer,sizeof(buffer),0,(struct sockaddr *)&client,&l);
            COAPPacket* p = COAPPacket::parse(buffer, rc, a->convertAddress(client).c_str());

            oic_server->handleMessage(p);
            delete p;
        }
        oic_server->sendQueuedPackets();
        if ((get_current_ms() - lastTick) > 1000){
            lastTick = get_current_ms();
            oic_server->checkPackets();
        }
    }
}


void* Application::runDiscovery(void* param){
    Application* a = (Application*) param;
    OICServer* oic_server = a->getServer();

    const int on = 1;

    int fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    struct sockaddr_in serv,client;
    struct ip_mreq mreq;

    serv.sin_family = AF_INET;
    serv.sin_port = htons(5683);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);

    mreq.imr_multiaddr.s_addr=inet_addr("224.0.1.187");
    mreq.imr_interface.s_addr=htonl(INADDR_ANY);
    if (setsockopt(fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0) {
        return 0;
    }

    uint8_t buffer[1024];
    socklen_t l = sizeof(client);
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        qDebug() << "Unable to set reuse";
        return 0;
    }
    if( bind(fd , (struct sockaddr*)&serv, sizeof(serv) ) == -1)
    {
        qDebug() << "Unable to bind";
        return 0;
    }
    struct pollfd pfd;
    int res;

    pfd.fd = fd;
    pfd.events = POLLIN;
    size_t rc;

    uint64_t lastTick = get_current_ms();
    while(a->isRunning()){
        rc = poll(&pfd, 1, 200); // 1000 ms timeout
        if(rc > 0)
        {
            rc= recvfrom(fd,buffer,sizeof(buffer),0,(struct sockaddr *)&client,&l);
            COAPPacket* p = COAPPacket::parse(buffer, rc, a->convertAddress(client));
            oic_server->handleMessage(p);
            delete p;
        }
    }
}
void Application::send_packet(COAPPacket* packet){
    String destination = packet->getAddress();
    size_t pos = destination.find(" ");
    String ip = destination.substr(0, pos);
    uint16_t port = atoi(destination.substr(pos).c_str());

    struct sockaddr_in client;

    client.sin_family = AF_INET;
    client.sin_port = htons(port);
    client.sin_addr.s_addr = inet_addr(ip.c_str());

    qDebug() << "Send packet mid" << packet->getMessageId() << "dest=" << destination.c_str();
    send_packet(client, packet);
}
void Application::send_packet(sockaddr_in destination, COAPPacket* packet){
    uint8_t buffer[1024];
    size_t response_len;
    socklen_t l = sizeof(destination);
    packet->build(buffer, &response_len);
    sendto(m_socketFd, buffer, response_len, 0, (struct sockaddr*)&destination, l);
}

void Application::notifyObservers(QString name, QVariant v){
    if (server !=0){
        qDebug() << "notiftyObservers" << name << v;
        cbor value(CBOR_TYPE_MAP);

        if (name.contains("ambient")){
            value.append("rt", "oic.r.colour.rgb");
            value.append("dimmingSetting", v.toString().toLatin1().data());
        }else{
            value.append("rt", "oic.r.light.dimming");
            value.append("dimmingSetting", v.toInt());
            value.append("range", "0,255");
        }

        List<uint8_t> data;
        value.dump(&data);


        server->notify(name.toLatin1().data(), &data);
    }
}


