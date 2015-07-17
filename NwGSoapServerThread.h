#ifndef _NWGSOAPSEVINTERFACE_H
#define _NWGSOAPSEVINTERFACE_H

#include <QThread>
#include <QWaitCondition>

#define BACKLOG (1000) // Max. request backlog 
struct soap;

namespace thp
{
	class WMTSRepository;
}

class  NwGSoapServerThread : public QThread
{
public:
	NwGSoapServerThread();
	virtual ~NwGSoapServerThread();

	//��GSOAP����
	void startGSoapServer();

	//ֹͣ����
	void stopGSoapServer();

	///��ʼ�����������������ã���˿ڰ󶨡����� ���ճ�ʱʱ�� �ص��������� 
	bool initServerStartParam();
protected:
	// QThread���غ���
	virtual void run();
	QString _GetLocalIPAddress();
private:
	volatile bool            m_bExit;              // �˳���־
	QWaitCondition           m_waitCondition;
	struct soap              *m_psoap; 
};

#endif