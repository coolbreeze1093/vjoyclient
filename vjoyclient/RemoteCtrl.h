#pragma once
#include<QUdpSocket>
#include <QObject>
#include <QMap>
class RemoteCtrl:public QObject
{
	Q_OBJECT
public:
	RemoteCtrl();
	~RemoteCtrl();
	bool bind(int port);
	void close();
private slots:
	void revData();
signals:
	void ctrlData(const QVariantMap&);
private:
	QUdpSocket m_udp;
	QByteArray m_array;
	
};

