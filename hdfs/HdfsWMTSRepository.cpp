#include "../StdAfx.h"
#include "HdfsWMTSRepository.h"
#include <io.h>
#include <sstream>
#include "../Tile.h"
//#include "HdfsWMTSLevel.h"
#include <iostream>
#include <algorithm>
#include "../WMTSConfig.h"
#include <CLogThreadMgr.h>
#include "../WMTSFactory.h"
#include "../WMTSLayer.h"
#include "json/json.h"
#include "curl/curl.h"
#include "HdfsUrl.h"

using namespace thp;

HdfsWMTSRepository::HdfsWMTSRepository()
{

}

HdfsWMTSRepository::~HdfsWMTSRepository()
{
	curl_global_cleanup();
}

bool thp::HdfsWMTSRepository::init(int nMode)
{
	// ��ʼ��curl
	curl_global_init(CURL_GLOBAL_ALL);

	// ���ļ��ṹ��ʼ��
	bool bSuccess = false;
	int nLayerCount = 0;
	int nFileSysType = WMTSConfig::Instance()->getFileSysType();
	if( 0x0001 == (nMode&0x0001) )
	{
		nLayerCount = _initByDirWithWebhdfs();
	}
	else
		nLayerCount = 0;

	if(nLayerCount > 0)
		return true;

	return bSuccess;
}

int HdfsWMTSRepository::_initByDirWithWebhdfs()
{
	// TODO: ����hdfs
	QString qsHdfsServer = WMTSConfig::Instance()->getHdfsServer();
	if( qsHdfsServer.isEmpty() )
	{
		QString qsError = QString("hdfs,��ʼ��ʧ��,�����ַȱʧ").arg(qsHdfsServer);
		m_pLogWriter->debugLog(qsError);
	}
	std::string sHdfsServer = (const char*)qsHdfsServer.toLocal8Bit();
	int nHdfsNNPort = WMTSConfig::Instance()->getHdfsNameNodeWebPort();

	// Ҫ������Ŀ¼ ͼ����Ŀ¼
	QString qsDataDir = WMTSConfig::Instance()->getDataDir(); 
	if( qsDataDir.isEmpty() )
	{
		QString qsError = QString("hdfs,��ʼ��ʧ��,����Ŀ¼ȱʧ").arg(qsDataDir);
		m_pLogWriter->debugLog(qsError);
	}
	std::string sDataDir = (const char*)qsDataDir.toLocal8Bit();
	int nLayerCount = 0;

	// ��ʽ�� webhdfs url
	std::stringstream ss;
	ss << "http://" << sHdfsServer << ":" << nHdfsNNPort
		<< "/webhdfs/v1";
		//<< sDataDir;

	std::string sBaseUrl = ss.str();

	// ��ʽ������ webhdfs url
	std::string sLsUrl = sBaseUrl + sDataDir + "?op=LISTSTATUS";

	// ����Ŀ¼
	CURL* curlHandle = curl_easy_init();

	// ���û�ȡ��ϸ����ϵ��Ϣ ��ǰΪ��״̬���Թر� �� 
	CURLcode res;

	QByteArray arDirectoryStatus;

	// ��ʼ�����Ӳ���
	::_initCurl(curlHandle);

	// ��������д�����
	res = curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &arDirectoryStatus);

	// ��������url
	res = curl_easy_setopt(curlHandle, CURLOPT_URL, sLsUrl.c_str() );

	res = curl_easy_perform(curlHandle);
	if( CURLE_OK != res )
	{
		QString sLog = QString("%0,%1,%2").arg("webhdfs").arg("op=LISTSTATUS").arg(GB("����ʧ��"));
		m_pLogWriter->debugLog(sLog);
		curl_easy_cleanup(curlHandle);

		return nLayerCount;
	}

	QString qs = arDirectoryStatus;

	curl_easy_cleanup(curlHandle);

	std::string sJson = (const char*)( qs.toLocal8Bit() );

	// ����json�� 
	Json::Reader reader;  
	Json::Value root;  
	if ( reader.parse(sJson, root) )  // reader��Json�ַ���������root��root������Json��������Ԫ��  
	{  
		Json::Value vlFileStatuses = root["FileStatuses"];
		Json::Value vlFileStatus = vlFileStatuses["FileStatus"];
		Json::Value::UInt nFileCount = vlFileStatus.size();
		for (Json::Value::UInt i = 0; i < nFileCount; ++i)
		{
			Json::Value vlTemp = vlFileStatus[i];
			std::string sType = vlTemp["type"].asString();

			// ���������ļ���Ŀ¼����
			if( 0 != sType.compare("FILE") )
				continue;

			std::string spathSuffix = vlTemp["pathSuffix"].asString();
			size_t nDotPos = spathSuffix.rfind('.');
			if( std::string::npos == spathSuffix.rfind('.') )
				continue;

			// ���˲����������ļ�
			std::string sFilesuffix = spathSuffix.substr(nDotPos);
			if( 0 != sFilesuffix.compare(".bdi") )
				continue;

			std::string sBdiPath = sDataDir + spathSuffix;
			//std::string sBdiPath = sDataDir + sFilesuffix;

			std::string sLayerName  = spathSuffix;
			sLayerName.erase(spathSuffix.size() - 4);

			if( _initLayerWithWebhdfs(sLayerName.c_str(), sBaseUrl.c_str(), sBdiPath.c_str()) )
				++nLayerCount;
		}
	}  

	return nLayerCount;
}

bool HdfsWMTSRepository::_initLayerWithWebhdfs(const char* szLayer, const char* szBaseUrl, const char* szBdiPath)
{
	size_t nLen = strlen(szLayer);
	if(0 == nLen || nLen > THP_WMTS_MAX_LAYERLEN)
	{
		QString qsError = QString("hdfs,��ʼ��ʧ��,ͼ��������(·����������%0)").arg(THP_WMTS_MAX_LAYERLEN);
		m_pLogWriter->warnLog(qsError);
		return false;
	}

	QString qsPath = WMTSConfig::Instance()->getDataDir();
	std::string sPath = (const char*)qsPath.toLocal8Bit();

	struct TLayerHashTableNode* pLayerNode = NULL;
	HASH_FIND_STR(m_layers, szLayer, pLayerNode);
	if( NULL != pLayerNode)
	{
		QString qsError = QString("hdfs,��ʼ��ʧ��,ͼ��(%s)�Ѵ���").arg( GB(szLayer) );
		m_pLogWriter->warnLog(qsError);
		return false;
	}

	char szPath[THP_MAX_PATH];
	memset(szPath, 0, THP_MAX_PATH);
	sprintf(szPath, "%s%s/", sPath.c_str(), szLayer);

	WMTSLayer* pNewLayer = WMTSFactory::Instance()->createLayer();
	if( !pNewLayer->setPath(szPath) )
	{
		QString qsError = QString("hdfs,��ʼ��ʧ��,ͼ��(%0)���ò���ʧ��").arg( GB(szLayer) );
		m_pLogWriter->warnLog(qsError);
		delete pNewLayer;
		return false;
	}

	// ͼ�㻺������
	pNewLayer->setCacheMbSize( WMTSConfig::Instance()->getOneLayerMaxCacheMB() );

	// �ڴ�������
	pNewLayer->setMemStrategy( (WMTSLayer::MemStrategy)WMTSConfig::Instance()->getMemStrategy() );

	std::cout << "��ʼ��ͼ��[" << szLayer << "]" << std::endl;

	std::string sBdiUrl = szBaseUrl;
	sBdiUrl += szBdiPath;
	if( 0 != pNewLayer->init(sBdiUrl.c_str()) )
	{
		std::cout << "��ʼ��ͼ��[" << szLayer << "]ʧ��";
		QString qsError = QString("hdfs,��ʼ��ʧ��,ͼ��(%0)��ʼ��ʧ��").arg( GB(szLayer) );
		m_pLogWriter->warnLog("��ʼ��ʧ��");
		delete pNewLayer;
		return false;
	}

	std::cout << "ͼ��[" << szLayer << "]��ʼ���ɹ�" << std::endl;

	struct TLayerHashTableNode* pNewNode = (struct TLayerHashTableNode*)malloc( sizeof(struct TLayerHashTableNode) );
	memset(pNewNode->szName, 0, THP_WMTS_MAX_LAYERLEN);
	memcpy(pNewNode->szName, szLayer, nLen);
	pNewNode->pLayer = pNewLayer;
	HASH_ADD_STR(m_layers, szName, pNewNode);

	return true;
}