#include "RemoteCtrl.h"
#include <QDebug>
#include <QNetworkDatagram>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
RemoteCtrl::RemoteCtrl()
{
	connect(&m_udp, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
		[=](QAbstractSocket::SocketError socketError) { 
			qDebug()<<m_udp.errorString();
		});
	

}

RemoteCtrl::~RemoteCtrl()
{

}

bool RemoteCtrl::bind(int port)
{
	bool _r = m_udp.bind(port);
	if (_r)
	{
		connect(&m_udp, &QUdpSocket::readyRead, this, &RemoteCtrl::revData);
	}
	return _r;
}

void RemoteCtrl::close()
{
	m_udp.close();
}

void RemoteCtrl::revData()
{
	QByteArray _array;
	qint64 _size = m_udp.bytesAvailable();
	if (_size <= 0)
	{
		return;
	}
	QNetworkDatagram _data = m_udp.receiveDatagram(_size);
	qDebug() << _data.data();
	m_array.append(_data.data());

	QString _str(m_array);
	QStringList _list = _str.split("*&*",QString::SkipEmptyParts);
	QVariantMap _vMap;
	m_array.clear();
	if (_list.count() >= 2)
	{
		QJsonDocument _doc = QJsonDocument::fromJson(_list.last().toUtf8());
		
		if (_doc.isNull())
		{
			_doc = QJsonDocument::fromJson(_list.at(_list.count()-2).toUtf8());
			if (_doc.isNull())
			{
				m_array.append(_list.last());
				return;
			}
			
		}
		if (_doc.isObject())
		{
			_vMap = _doc.object().toVariantMap();
		}
	}
	else
	{
		QJsonDocument _doc = QJsonDocument::fromJson(_list.last().toUtf8());
		if (_doc.isNull())
		{
			m_array.append(_list.last());
			return;
		}
		if (_doc.isObject())
		{
			_vMap = _doc.object().toVariantMap();
		}
	}
	
	emit ctrlData(_vMap);
}

