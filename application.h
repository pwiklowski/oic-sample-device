#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include "QGuiApplication"
#include "OICServer.h"
#include <QQmlApplicationEngine>
#include <arpa/inet.h>
#include <net/if.h>

class Application : public QGuiApplication
{
    Q_OBJECT
public:
    explicit Application(int& argc, char *argv[]);
    ~Application();
    OICServer* getServer(){return server;}
    void setSocketFd(int s) { m_socketFd = s;}
    bool isRunning();
signals:

public slots:
    static void* runDiscovery(void* param);
    static void* run(void* param);

    void notifyObservers(QString name, QVariant value);

private:
    bool m_running;
    String convertAddress(sockaddr_in a);
    void send_packet(sockaddr_in destination, COAPPacket* packet);
    void send_packet(COAPPacket* packet);

   int m_socketFd;
    OICServer* server;
    QQmlApplicationEngine engine;
    pthread_t m_thread;
    pthread_t m_discoveryThread;
    quint16 m_front;
};

#endif // APPLICATION_H
