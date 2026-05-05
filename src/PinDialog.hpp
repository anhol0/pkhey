#ifndef PIN_DIALOG_HPP
#define PIN_DIALOG_HPP

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QInputDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <qapplication.h>
#include <qboxlayout.h>
#include <qlineedit.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <qtmetamacros.h>
#include <qwidget.h>

class PinDialog : public QWidget{
    Q_OBJECT
public:
    explicit PinDialog(QWidget *parent = nullptr);
    void reset();
signals:
    void pinSubmitted(const QString &pin);
    void cancelled();
protected:
    virtual void closeEvent(QCloseEvent *event) override;
    QLineEdit *pInput; 
    QPushButton *pSubmit;
    QPushButton *pCancel;
};

#endif
