#include "../StdAfx.h"
#include "HdfsBundleReader.h"
#include "HdfsUrl.h"
#include "json/json.h"
#include "../ParamDef.h"

#include <QString>


using namespace thp;

thp::HdfsBundleReader::HdfsBundleReader()
{

}

thp::HdfsBundleReader::~HdfsBundleReader()
{

}

bool thp::HdfsBundleReader::open(const char* szFile)
{
	size_t nLen = strlen(szFile);
	if(nLen >= BUNDLE_MAX_PATH || nLen < 20)
	{
		sprintf(m_szLastErr, "file path is too long!the limits length is %d", BUNDLE_MAX_PATH);
		QString qsLog = QString("hdfs,bundle�ļ�(%s)��ʧ��").arg( GB(szFile) );
		return false;
	}// bundle �ļ�������Ϊ 17 +'\0' 

	memcpy(m_szBundleFile, szFile, nLen);
	m_szBundleFile[nLen] = '\0';

	char szBundlxFile[BUNDLE_MAX_PATH];
	memcpy(szBundlxFile, m_szBundleFile, THP_MAX_PATH);
	szBundlxFile[nLen-1] = 'x';
	if( !_loadBundlx(szBundlxFile) )
		return false;

	size_t nFrom = nLen - 16;	//
	char szNum[5];
	memcpy(szNum, szFile+nFrom, 4/* * sizeof(char) */ );
	szNum[4] = '\0';
	int nRow = 0;
	sscanf(szNum, "%x", &nRow);

	memcpy(szNum, szFile + nFrom + 5, 4 /* * sizeof(char) */);
	int nCol = 0;
	sscanf(szNum, "%x", &nCol);

	m_nBundleBeginRow = nRow;
	m_nBundleBeginCol = nCol;

	return true;
}

// ? �����紫��ĳ����� �ڴ�bundleʱһ�δ�����Ӧ����û�б�Ҫ��-�������л��ⲿ�ִ���
bool thp::HdfsBundleReader::_loadBundlx(const char* szFile)
{
	CURL* curlHandle = curl_easy_init();

	_initCurl(curlHandle);

	QString qsUrl = QString("%0?op=OPEN").arg( GB(szFile) );
	std::string sUrl = (const char*)( qsUrl.toLocal8Bit() );
	
	QByteArray arBundlx;

	// ��ʼ�����Ӳ���
	::_initCurl(curlHandle);

	// ��������д�����
	CURLcode res = curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &arBundlx);

	// ��������url
	res = curl_easy_setopt(curlHandle, CURLOPT_URL, sUrl.c_str() );
	if(CURLE_OK != res)
		return false;

	res = curl_easy_perform(curlHandle);
	if(CURLE_OK != res)
		return false;

	if( CURLE_OK != res )
	{
		QString sLog = QString("%0,%1,%2").arg("webhdfs").arg(GB("����ʧ��")).arg(qsUrl);
		return false;
	}

	if( arBundlx.size() != (BUNDLX_CONTENT_SIZE + BUNDLX_DOMX2) )
	{
		// �����ļ����ݳ���
		return false;
	}

	int nPtr = 0;
	int nTemp = 0;
	// �ļ�ǰ16���ֽ�����
	nPtr += BUNDLX_DOM;

	memcpy(m_szBundlxContents, arBundlx.data() + nPtr, BUNDLX_CONTENT_SIZE);
	nPtr += BUNDLX_CONTENT_SIZE;

	curl_easy_cleanup(curlHandle);
	return true;
}

bool thp::HdfsBundleReader::getTileFromFile(int nTileInBundleIndex, unsigned char*& pByteTile, unsigned int& nSize)
{
	unsigned int* pOffSet = (unsigned int*)(m_szBundlxContents + nTileInBundleIndex * BUNDLX_NODE_SIZE);

	CURL* curlHandle = curl_easy_init();

	// ��ʼ�����Ӳ���
	::_initCurl(curlHandle);

	// ��ʼ����ѯTile��С�Ĳ�ѯ����
	QString qsUrl = QString("%0?op=OPEN&offset=%1&length=%2").arg(m_szBundleFile).arg(*pOffSet).arg( BUNDLE_TILE_SIZE );
	std::string sUrl = (const char*)( qsUrl.toLocal8Bit() );

	QByteArray arTileSize;

	// ��������д�����
	CURLcode res = curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &arTileSize);

	// ��������url
	res = curl_easy_setopt(curlHandle, CURLOPT_URL, sUrl.c_str() );
	if(CURLE_OK != res)
		return false;

	res = curl_easy_perform(curlHandle);
	if(CURLE_OK != res)
		return false;

	// �������ڴ濽�� ��QByteArray ��ת�������õ�����0ֵ
	memcpy(&nSize, arTileSize.data(), BUNDLE_TILE_SIZE);

	if(0 == nSize)
	{
		// û�ж�Ӧ��Tile����
		return false;
	}

	// ��ȡTile
	qsUrl = QString("%0?op=OPEN&offset=%1&length=%2").arg( GB(m_szBundleFile) ).arg((*pOffSet) + BUNDLE_TILE_SIZE).arg( nSize );
	sUrl = (const char*)( qsUrl.toLocal8Bit() );

	QByteArray arTile;

	// ��������д�����
	res = curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &arTile);

	// ��������url
	res = curl_easy_setopt(curlHandle, CURLOPT_URL, sUrl.c_str() );
	if(CURLE_OK != res)
		return false;

	// ����
	res = curl_easy_perform(curlHandle);
	if(CURLE_OK != res)
		return false;

	// ��ȡ����
	pByteTile = new unsigned char[nSize];
	if(!pByteTile)
	{
		sprintf(m_szLastErr, "memory Error");
		delete[] pByteTile;
		return false;
	}

	memcpy(pByteTile, arTile.data(), nSize);

	curl_easy_cleanup(curlHandle);

	return true;
}

unsigned int thp::HdfsBundleReader::getMaxByte()
{
	unsigned int nSize = 0;

	CURL* curlHandle = curl_easy_init();
	
	// ��ʼ�����Ӳ���
	::_initCurl(curlHandle);

	QString qsUrl = QString("%0?op=LISTSTATUS").arg(GB(m_szBundleFile));
	std::string sUrl = (const char*)( qsUrl.toLocal8Bit() );

	QByteArray arBundleStatus;

	// ��������д�����
	CURLcode res = curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &arBundleStatus);

	// ��������url
	res = curl_easy_setopt(curlHandle, CURLOPT_URL, sUrl.c_str() );
	if(CURLE_OK != res)
		return false;

	res = curl_easy_perform(curlHandle);
	if(CURLE_OK != res)
		return false;

	curl_easy_cleanup(curlHandle);

	QString qsBundleStatus = arBundleStatus;
	std::string sJson = (const char*)( qsBundleStatus.toLocal8Bit() );

	// ����json�� 
	Json::Reader reader;  
	Json::Value root;  
	if ( reader.parse(sJson, root) )  // reader��Json�ַ���������root��root������Json��������Ԫ��  
	{  
		Json::Value vlFileStatuses = root["FileStatuses"];
		Json::Value vlFileStatus = vlFileStatuses["FileStatus"];
		Json::Value::UInt nFileCount = vlFileStatus.size();

		if( 1 != nFileCount)
		{
			// �ļ��ṹ����
			return 0;
		}

		Json::Value vlTemp = vlFileStatus[ Json::Value::UInt(0) ];
		std::string sType = vlTemp["type"].asString();

		// ���������ļ���Ŀ¼����
		if( 0 != sType.compare("FILE") )
		{
			// �ļ��ṹ����
			return 0;
		}
		
		// �õ��ļ���С
		Json::Value::UInt nSizeByte = vlTemp["length"].asUInt();
		nSize = (unsigned int)nSizeByte;
	}

	return nSize;
}

int HdfsBundleReader::readAll(char*& pBundle)
{
	// TODO: ���������Ż� ��һ�ζ���Ĵ��ڴ����
	// ��ȡBundle�ļ���С
	unsigned int nSize = getMaxByte();

	pBundle = new char[nSize]; 
	if(!pBundle)
	{
		// log
		std::cerr << "�ڴ治��" << std::endl;
		sprintf(m_szLastErr, "memory full");
		delete[] pBundle;
		pBundle = NULL;
		return 0;
	}

	CURL* curlHandle = curl_easy_init();

	// ��ʼ�����Ӳ���
	::_initCurl(curlHandle);

	QString qsUrl = QString("%0?op=OPEN").arg(m_szBundleFile);
	std::string sUrl = (const char*)( qsUrl.toLocal8Bit() );

	QByteArray arBundle;

	// ��������д�����
	CURLcode res = curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &arBundle);

	// ��������url
	res = curl_easy_setopt(curlHandle, CURLOPT_URL, sUrl.c_str() );
	if(CURLE_OK != res)
	{
		delete[] pBundle;
		pBundle = NULL;
		return 0;
	}

	res = curl_easy_perform(curlHandle);
	if(CURLE_OK != res)
	{
		delete[] pBundle;
		pBundle = NULL;
		return false;
	}

	memcpy(pBundle, arBundle.data(), nSize);

	return nSize;
}



