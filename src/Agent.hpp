#ifndef AGENT_HPP
#define AGENT_HPP

#include <PolkitQt1/Agent/Listener>
#include <QDebug>
#include <polkitqt1-agent-session.h>
#include <qobject.h>
#include "PinDialog.hpp"

class Agent : public PolkitQt1::Agent::Listener {
    Q_OBJECT
public:
        explicit Agent(QObject *parent = nullptr);
        // What happens in authentication request 
        void initiateAuthentication ( const QString & actionId,
            const QString & message,
            const QString & iconName,
            const PolkitQt1::Details & details,
            const QString & cookie,
            const PolkitQt1::Identity::List & identities,
            PolkitQt1::Agent::AsyncResult * result
        ) override; 
        // What to do when authentication is finished
        bool initiateAuthenticationFinish () override;
        // Authentication cancel handling
        void cancelAuthentication () override;
protected:
        PinDialog *dialog = nullptr;
        PolkitQt1::Agent::AsyncResult *pendingResult = nullptr;
        PolkitQt1::Agent::Session *pendingSession = nullptr;
        PolkitQt1::Identity pendingIdentity;
        QString pendingCookie;
};

#endif
