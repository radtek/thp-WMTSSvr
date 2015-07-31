#include "../StdAfx.h"
#include "HdfsWMTSLayer.h"
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <io.h>
#include <QString>
#include <CLogThreadMgr.h>
#include <CLogWriter.h>
#include "../WMTSLevel.h"
#include "../LayerLRUCache.h"
#include "../Tile.h"
#include "../WMTSLayer.h"
#include "../Bundle.h"
#include "../bdi/bdiapi.h"
#include "./HdfsUrl.h"
#include "../WMTSFactory.h"

using namespace thp;

thp::HdfsWMTSLayer::~HdfsWMTSLayer()
{
	_clear();
}

int HdfsWMTSLayer::init(const char* szBdiPath)
{
	std::map<int, TLevelBundleExistStatus*> mapBdi;

	if( !readBdiWithWebhdfs(szBdiPath, mapBdi) )
		return 1;

	// �����ͼ��Ŀ¼
	std::string sLayerPath = szBdiPath;
	size_t nPos = sLayerPath.rfind(".bdi");
	sLayerPath.erase(nPos, 4);
	sLayerPath.append("/");

	int nRes = 0;
	nRes = _initLevels( sLayerPath.c_str(), mapBdi);

	if(  !mapBdi.empty() )
	{
		std::map<int, TLevelBundleExistStatus*>::iterator it = mapBdi.begin();
		for( ;it != mapBdi.end(); ++it)
			delete it->second;
	}

	return nRes>0?0:1;
}

unsigned int HdfsWMTSLayer::getTile(int nLvl, int nRow, int nCol, QByteArray& arTile, int& nDetail)
{
#ifdef _THP_TJ
	InterlockedIncrement( (LONG*)(&m_nCount) );
#endif// _THP_TJ

	if( nLvl > THP_MAX_LEVEL)
	{
		QString qsInfo = QString( GB("%0,%1,%2,%3,�������ȼ�") ).arg( GB("info") ).arg(nLvl).arg(nRow).arg(nCol);
		m_pLogWriter->debugLog(qsInfo);
		return 0;
	}// �����������Χ

	if( nRow < 0 || nRow > (1 << (nLvl-1)) )
	{
		QString qsInfo = QString( GB("%0,%1,%2,%3,����Խ��") ).arg( GB("info") ).arg(nLvl).arg(nRow).arg(nCol);
		m_pLogWriter->debugLog(qsInfo);
		return 0;
	}// ������Χ�������Ƭ����

	if( nCol < 0 || nCol > (1 << nLvl) )
	{
		QString qsInfo = QString( GB("%0,%1,%2,%3,����Խ��") ).arg( GB("info") ).arg(nLvl).arg(nRow).arg(nCol);
		m_pLogWriter->debugLog(qsInfo);
		return 0;
	}// ������Ƭ����

	// û�ж�Ӧ�ĵȼ�
	WMTSLevel* pLv = m_pLvl[nLvl];
	if( NULL == pLv )
	{
		QString qsInfo = QString( GB("%0,%1,%2,%3,û�ж�Ӧ�ȼ�") ).arg( GB("info") ).arg(nLvl).arg(nRow).arg(nCol);
		m_pLogWriter->debugLog(qsInfo);
		return 0;
	}

	TBundleIDex tbnNo;
	_calcBundleNo(nLvl, nRow, nCol, tbnNo);

	// ���Ի���
	std::tr1::shared_ptr<Bundle> spBundle; 

	// ������Դ
	TBundleRecord* pRecord = NULL;
	HASH_FIND(hh, m_pBundleRecords, &(tbnNo.tID), sizeof(TBundleID), pRecord);

	// ��ȡ�����ɹ�����û�ж�Ӧ���������Դ���
	if( NULL == pRecord )
	{
		// bundle ������
		if( !pLv->exist(tbnNo) )
		{
			QString qsInfo = QString( GB("%0,%1,%2,%3,û�ж�Ӧbundle") ).arg( GB("info") ).arg(nLvl).arg(nRow).arg(nCol);
			m_pLogWriter->debugLog(qsInfo);
			return 0;
		}

		{
			QMutexLocker locker(&m_pRecordsMutex);

			HASH_FIND(hh, m_pBundleRecords, &(tbnNo.tID), sizeof(TBundleID), pRecord);
			if(NULL == pRecord)
			{
				pRecord = new TBundleRecord;

				pRecord->tID = tbnNo.tID;
				pRecord->nBundleRow = tbnNo.nBundleRow;
				pRecord->nBundleCol = tbnNo.nBundleCol;

				HASH_ADD(hh, m_pBundleRecords, tID, sizeof(TBundleID), pRecord);

#ifdef _THP_TJ
				// ��¼�����ڴ��bundle���ļ���С
				spBundle = pRecord->loadBundle(pLv);
				unsigned int nbKb = spBundle->getMaxKB();

				QString qsInfo = QString( GB("%0,%1,%2") ).arg( GB("FILE") ).arg( spBundle->getPath() ).arg( nbKb );
				m_pLogWriter->debugLog(qsInfo);
#endif// _THP_TJ

			}
		}
	}// ����bundle

	spBundle = pRecord->loadBundle(pLv);

	// ��Դ�޷��������أ�û�ж�Ӧ����Ƭ�ļ�,����û�ж�Ӧ��bundle��Դ
	if( NULL == spBundle.get() )
	{
		QString sErrInfo = QString("none tile resource, %0,%1,%2,%3");
		m_pLogWriter->errorLog( GB("IO��Դ����") );
		return 0;
	}

	// ��֤�� 8 �����ϵ�bundle���Ի���
	if( (GTS_MEM == m_eGTS) && (spBundle->getMaxKB() < (m_pLyrLRU->getCapacity() >> 3)) )
	{
		if( !spBundle->isCached() )
		{
			spBundle->cache();
		}

		m_pLyrLRU->add(spBundle);
	}

	if(spBundle->getID().unLv < 0 || spBundle->getID().unLv > 21)
	{
		m_pLogWriter->errorLog( GB("IO��Դ����") );
		return 0;
	}

	return spBundle->getTile(nRow, nCol, arTile, nDetail);
}

bool thp::HdfsWMTSLayer::readBdiWithWebhdfs(const std::string& sUrl, std::map<int, TLevelBundleExistStatus*>& idxMap)
{
	QByteArray qaBdi;

	// ��ȡbdi
	CURL* curlHandle = curl_easy_init();

	// ���û�ȡ��ϸ����ϵ��Ϣ ��ǰΪ��״̬���Թرյ� 
	CURLcode res;

	// ��ʼ�����Ӳ���
	::_initCurl(curlHandle);

	// ��������д�����
	res = curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &qaBdi);

	// ��������url
	std::string sOpenBdiUrl = sUrl + "?op=OPEN";
	res = curl_easy_setopt(curlHandle, CURLOPT_URL, sOpenBdiUrl.c_str() );

	res = curl_easy_perform(curlHandle);
	if( CURLE_OK != res )
	{
		QString sLog = QString("%0,%1,%2").arg("webhdfs").arg("op=LISTSTATUS").arg(GB("����ʧ��"));
		m_pLogWriter->debugLog(sLog);
	}

	curl_easy_cleanup(curlHandle);

	// ��������
	if( qaBdi.size() < 16 )
		return false;
	
	int nPtr = 16;
	int i = 0;

	int nSize = qaBdi.size();
	bool bSuccess = true;
	int nByteCount = 0;
	int nLv = 0;
	for (i=0; i < MAX_LEVLE; ++i)
	{
		if(nPtr + sizeof(int) > nSize)
			break;
		memcpy(&nLv, qaBdi.data() + nPtr, sizeof(int));
		nPtr += sizeof(int);

		if(nPtr + sizeof(int) > nSize)
			break;
		memcpy(&nByteCount, qaBdi.data() + nPtr, sizeof(int));
		nPtr += sizeof(int);

		if(0 == nByteCount)
		{
			idxMap.insert( std::make_pair(i, (TLevelBundleExistStatus*)NULL) );
			continue;
		}

		TLevelBundleExistStatus* pNode = new TLevelBundleExistStatus;
		pNode->nSize = nByteCount;

		pNode->pbyteIndex = new unsigned char[nByteCount];
		memcpy((void*)(pNode->pbyteIndex), qaBdi.data() + nPtr, nByteCount);
		nPtr += nByteCount;
		idxMap.insert( std::make_pair(i, pNode) );
	}

	return bSuccess;
}

int HdfsWMTSLayer::_initLevels(const char* szLayerUrl, const std::map<int, TLevelBundleExistStatus*>& mapBdi)
{
	std::map<int, TLevelBundleExistStatus*>::const_iterator it = mapBdi.begin();
	int nSuccessLv = 0;
	for (int i=0; i < THP_MAX_LEVEL; ++i)
	{
		it = mapBdi.find(i);
		if(mapBdi.end() == it)
		{
			std::cerr << "��������" << std::endl;
			return -1;
		}

		const TLevelBundleExistStatus* pNode = it->second;
		if( NULL == pNode || 0 == pNode->nSize )
		{
			m_pLvl[i] = NULL;
			continue;
		}

		if( 0 != _initLevel(szLayerUrl, i, pNode) )
		{
			m_pLvl[i] = NULL;
		}

		++nSuccessLv;
	}

	return nSuccessLv;
}

int HdfsWMTSLayer::_initLevel(const char* szPath , int nLvl, const TLevelBundleExistStatus* pNode)
{
	char szLvlPath[THP_MAX_PATH];
	memset(szLvlPath, 0, THP_MAX_PATH);
	sprintf(szLvlPath, "%sL%02d/", szPath, nLvl);
	
	// ��ʼ�� level
	m_pLvl[nLvl] = WMTSFactory::Instance()->createLevel(nLvl); 
	m_pLvl[nLvl]->setPath(szLvlPath);
	m_pLvl[nLvl]->setBdi(pNode->nSize, pNode->pbyteIndex);

	return 0;
}

void HdfsWMTSLayer::_clear()
{
	try
	{
		int i = 0;
		for (; i < THP_MAX_LEVEL; ++i)
		{
			if(NULL == m_pLvl[i])
				continue;

			delete m_pLvl[i];
			m_pLvl[i] = NULL;
		}
	}
	catch (...)
	{

	}
}

