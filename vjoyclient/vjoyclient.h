#pragma once

#include <QtWidgets/QWidget>
#include "ui_vjoyclient.h"
#include "VjoyAgent.h"
#include "RemoteCtrl.h"

class vjoyclient : public QWidget
{
    Q_OBJECT

public:
    vjoyclient(QWidget *parent = nullptr);
    ~vjoyclient();
private slots:
    void setValue();
    void ctrlData(const QVariantMap&);
private:
    Ui::vjoyclientClass ui;

    VJoy::VjoyAgent m_vjoy;

    VJoy::CtrlValue m_value;

    bool m_isClicked=false;

    RemoteCtrl m_rc;
};
