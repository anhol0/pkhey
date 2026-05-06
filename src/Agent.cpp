#include "Agent.hpp"
#include <polkitqt1-agent-session.h>
#include <polkitqt1-identity.h>
#include <qlogging.h>
#include <QString>
#include <qobject.h>

Agent::Agent(QObject *parent) : PolkitQt1::Agent::Listener(parent) {
    dialog = new PinDialog();
    connect(dialog, &PinDialog::pinSubmitted, this, 
        [this](const QString &pin) {
            if(pendingSession) {
                pendingSession->setResponse(pin);
            }
        }
    );

    connect(dialog, &PinDialog::cancelled, this, [this]() {
        if(pendingSession) {
            pendingSession->cancel();
            return;
        }
    });
}

void Agent::initiateAuthentication ( const QString & actionId,
    const QString & message,
    const QString & iconName,
    const PolkitQt1::Details & details,
    const QString & cookie,
    const PolkitQt1::Identity::List & identities,
    PolkitQt1::Agent::AsyncResult * result
) {
    if(identities.isEmpty()) {
        result->setError("No identities were provided!");
        result->setCompleted();
        return;
    }
    
    PolkitQt1::Identity identity = identities.first();
    QString name = identity.toString();
    dialog->setIdentity("Authenticating as " + name);

    pendingIdentity = identity;
    pendingCookie = cookie;
    pendingResult = result;
    pendingSession = new PolkitQt1::Agent::Session(pendingIdentity, pendingCookie, pendingResult, this);

    connect(pendingSession, &PolkitQt1::Agent::Session::request, this, 
        [this](const QString &prompt, bool echo) {
            dialog->reset();
            dialog->show(); 
        }
    );

    connect(pendingSession, &PolkitQt1::Agent::Session::completed, this, 
        [this](bool gainedAuthorization) {
            pendingSession->deleteLater();
            pendingSession = nullptr;

            if(pendingResult) {
                if(gainedAuthorization) {
                    pendingResult->setCompleted();
                } else {
                    pendingResult->setError("Authentication failed");
                    pendingResult->setCompleted();
                }
                pendingResult = nullptr;
            }
            dialog->hide();
        }
    );

    connect(pendingSession, &PolkitQt1::Agent::Session::showError, this, [](const QString &text) {
        qWarning() << "Polkit session error: " << text;
    });

    pendingSession->initiate();
    return;
};

bool Agent::initiateAuthenticationFinish () {
    return true;
};

void Agent::cancelAuthentication () {
    qDebug() << "Authentication was cancelled by polkitd";
    if(pendingSession) {
        pendingSession->cancel();
        pendingSession->deleteLater();
        pendingSession = nullptr;
    }
    if(pendingResult) {
        pendingResult->setError("Authentication cancelled");
        pendingResult->setCompleted();
        pendingResult = nullptr;
    }
    dialog->hide();
    return;
}
