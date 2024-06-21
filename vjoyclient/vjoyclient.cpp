#include "vjoyclient.h"
#include <QMessageBox>
#include <QDebug>
#include <QSettings>

vjoyclient::vjoyclient(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    QSettings _setting("config.ini");
    ui.lineEdit_rmPort->setText(_setting.value("port").toString());
    connect(ui.lineEdit_rmPort, &QLineEdit::textChanged, [this]() {
        QSettings _setting("config.ini");
        _setting.setValue("port", ui.lineEdit_rmPort->text());
        
        });
    connect(ui.pushButton_start, &QPushButton::clicked, this, [this] {
        if (m_isClicked)
        {
            ui.pushButton_start->setText(QString::fromLocal8Bit("开始"));
            m_vjoy.terminate();
            m_isClicked = false;
            m_rc.close();
        }
        else
        {
            ui.pushButton_start->setText(QString::fromLocal8Bit("结束"));
            if (!m_vjoy.init(ui.comboBox_did->currentText().toInt()))
            {
                QMessageBox::warning(nullptr,QStringLiteral("提示"),QStringLiteral("vjoy 初始化失败！"));
            }
            else
            {
                m_vjoy.startffb();
            }
            if (!m_rc.bind(ui.lineEdit_rmPort->text().toInt()))
            {
                QMessageBox::warning(nullptr, QStringLiteral("提示"), QStringLiteral("网络初始化失败！"));
            }
            m_isClicked = true;
        }
        });

    connect(ui.checkBox_localTest, &QCheckBox::stateChanged, [this](int value) {
        if (value == 2)
        {
            disconnect(&m_rc, &RemoteCtrl::ctrlData, this, &vjoyclient::ctrlData);
            connect(ui.horizontalSlider_c1, &QSlider::valueChanged, this, &vjoyclient::setValue);
            connect(ui.horizontalSlider_c2, &QSlider::valueChanged, this, &vjoyclient::setValue);
            connect(ui.horizontalSlider_c3, &QSlider::valueChanged, this, &vjoyclient::setValue);
            connect(ui.horizontalSlider_c4, &QSlider::valueChanged, this, &vjoyclient::setValue);

        }
        else
        {
            connect(&m_rc, &RemoteCtrl::ctrlData, this, &vjoyclient::ctrlData);
            disconnect(ui.horizontalSlider_c1, &QSlider::valueChanged, this, &vjoyclient::setValue);
            disconnect(ui.horizontalSlider_c2, &QSlider::valueChanged, this, &vjoyclient::setValue);
            disconnect(ui.horizontalSlider_c3, &QSlider::valueChanged, this, &vjoyclient::setValue);
            disconnect(ui.horizontalSlider_c4, &QSlider::valueChanged, this, &vjoyclient::setValue);
        }
        });

    connect(&m_rc, &RemoteCtrl::ctrlData, this, &vjoyclient::ctrlData);
}

vjoyclient::~vjoyclient()
{}

void vjoyclient::setValue()
{
    m_value.Throttle = ui.horizontalSlider_c1->value();
    m_value.Rudder = ui.horizontalSlider_c2->value();
    m_value.Elevator = ui.horizontalSlider_c3->value();
    m_value.Aileron = ui.horizontalSlider_c4->value();
    m_vjoy.setCtrlValue(m_value);
}

void vjoyclient::ctrlData(const QVariantMap&value)
{
    m_value.Throttle = value.value("channel1").toDouble();
    m_value.Rudder = value.value("channel2").toDouble();
    m_value.Aileron = value.value("channel3").toDouble();
    m_value.Elevator = value.value("channel4").toDouble();

    qDebug() << "m_value.Throttle "<< m_value.Throttle
        <<"m_value.Rudder"<< m_value.Rudder
        <<"m_value.Aileron"<< m_value.Aileron
        <<"m_value.Elevator"<< m_value.Elevator;

    ui.horizontalSlider_c1->setValue(m_value.Throttle);
    ui.horizontalSlider_c2->setValue(m_value.Rudder);
    ui.horizontalSlider_c3->setValue(m_value.Elevator);
    ui.horizontalSlider_c4->setValue(m_value.Aileron);

    ui.spinBox->setValue(m_value.Throttle);
    ui.spinBox_2->setValue(m_value.Rudder);
    ui.spinBox_3->setValue(m_value.Elevator);
    ui.spinBox_4->setValue(m_value.Aileron);

    m_vjoy.setCtrlValue(m_value);
}