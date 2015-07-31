/*
 *  Http�������
 *
 *  ����OGC ��׼�淶�е�WMS��WFS����
    �ο���ַ��
	��TOKEN���󣬷��������
 *
 * 
 */

#ifndef __HTTPWMSSERVICE_H
#define __HTTPWMSSERVICE_H
#include "stdsoap2.h"
#include <QMutex>

class TokenSessionData;
class CLogWriter;

class HttpWMSService
{
public:
	HttpWMSService(void);
	~HttpWMSService(void);

	// ����WMS��GetMap����
	 int dwmGetMap(struct soap * soap, char * path);
	
private:
	int _dwmGetMapWindows(struct soap * soap, char * path);
	int _dwmGetMapUnix(struct soap * soap, char * path);

	///����ͼƬ����
	/// @param baMapData	[in]	ͼƬ����
	/// @param strformat	[in]	ͼƬ��ʽ
	 int _sendData(struct soap *soap, QByteArray &baMapData, const char* szformat);

	 int _sendCapabilities(struct soap *soap);

	///����쳣��Ϣ
	/// @param strMessage	[in]	������Ϣ
	/// @param strCode	    [in]	�������
	/// @param strVersion	[in]	����汾
	 int _sendExceptionMessage(struct soap *soap, const QString  & strMessage, const QString &strCode, QString strVersion ="1.0.0");
	 bool _initLogWriter();

private:
	//QMutex m_mx;
	CLogWriter* m_pLogWriter;
};


#endif 
