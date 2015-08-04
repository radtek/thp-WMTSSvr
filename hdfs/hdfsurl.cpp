#include <QtCore>
#include <QtGui>
#include <QtXml>
#include "hdfsurl.h"
#include <QByteArray>

size_t writeData(void *pIn, size_t nSize, size_t nItems, void *pOut)
{
	QByteArray* pAr = (QByteArray*)pOut;
	pAr->append( (const char*)pIn, nSize * nItems);

	return nSize * nItems;//	һ��д�˶����ֽ�
}

bool _initCurl(CURL* handle)
{
	// ���û�ȡ��ϸ����ϵ��Ϣ ��ǰΪ��״̬���Թر� �� 
	CURLcode res;
	//res = curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
	//if(res != CURLE_OK)
	//	return false;

	// ����û�н�����ʾ
	res = curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1L);
	if(res != CURLE_OK)
		return false;

	// ���ó�ʱ Ĭ�ϳ�ʱ
	//curl_easy_setopt(handle, CURLOPT_TIMEOUT, 2); 

	// ����д���ݻص�����
	res = curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeData);
	if(res != CURLE_OK)
		return false;

	// �����Զ�����
	res = curl_easy_setopt(handle, CURLOPT_AUTOREFERER, true);
	if(res != CURLE_OK)
		return false;
	//���ص�ͷ������Location(һ��ֱ�������urlû�ҵ�)�����������Location��Ӧ������ 
	res = curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);
	if(res != CURLE_OK)
		return false;
	//���Ҵ�������ֹ����̫��
	res = curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 1);		
	if(res != CURLE_OK)
		return false;
	//���ӳ�ʱ�������ֵ�������̫�̿��ܵ����������󲻵��ͶϿ�
	res = curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 3 );	
	if(res != CURLE_OK)
		return false;

	return true;
}
