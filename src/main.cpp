#include "Agent.hpp"
#include "qlogging.h"
#include <QApplication>
#include <polkitqt1-subject.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Settings of the window 
    app.setApplicationName("pkhey");
    app.setQuitOnLastWindowClosed(false);

    //Getting user session 
    PolkitQt1::UnixSessionSubject subject(getpid());

    //Initializing agent 
    Agent agent;
    if(!agent.registerListener(subject, "/org/freedesktop/PolicyKit1/AuthenticationAgent")) {
        qCritical() << "Failed to register PolKit agent";
        return 1;
    }
    qInfo() << "Polkit agent was successfully registered, waiting for requests";

    // Running 
    return app.exec();
}
