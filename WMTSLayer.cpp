#include "StdAfx.h"
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <io.h>

#include <CLogThreadMgr.h>

#include "WMTSLevel.h"
#include "LayerLRUCache.h"
#include "Tile.h"
#include "WMTSLayer.h"
#include "Bundle.h"
#include "bdi/bdiapi.h"

#pragma warning(once:4996)

using namespace thp;

thp::TBundleRecord::TBundleRecord()
{
	//pthread_rwlock_init(&rwLocker, NULL);
}

thp::TBundleRecord::~TBundleRecord()
{
	//pthread_rwlock_destroy(&rwLocker);
}

std::tr1::shared_ptr<Bundle> thp::TBundleRecord::loadBundle(WMTSLevel* pLv)
{
	try
	{
		{
			QReadLocker locker(&rwLocker);
			if( !wpBundle.expired() )
			{
				std::tr1::shared_ptr<Bundle> sp = wpBundle.lock();
				return sp;
			}
		}

		QWriteLocker locker(&rwLocker);

		std::tr1::shared_ptr<Bundle> sp = pLv->getBundle( *this );

		wpBundle = sp;

		return sp;
	}
	catch(...)
	{
		//LOG(ERROR) << "������Դ����";
		std::cerr << "������Դ����";
		std::tr1::shared_ptr<Bundle> sp;
		return sp;
	}
}

thp::WMTSLayer::WMTSLayer()
{
	memset(m_pLvl, 0, THP_MAX_LEVEL);
	memset(m_szPath, 0, THP_MAX_PATH);

	m_pLyrLRU = new LayerLRUCache(THP_WMTS_DEFAULT_LRU_CACHE);

	m_pBundleRecords = NULL;
	m_eGTS = GTS_IO;

	_initLogWriter();
	//pthread_mutex_init(&m_pRecordsMutex, NULL);
}

bool WMTSLayer::_initLogWriter()
{
	// ��ȡд��־����
	m_pLogWriter = CLogThreadMgr::instance()->getLogWriter("WMTSLayer.log");

	// ��������ڣ�������־�ļ���������־����
	if (m_pLogWriter == NULL)
	{
		CLogAppender * pLogAppender = new CLogAppender("WMTSLayer", "WMTSLayer.log", "", "DebugLog"); 

		// ��ȡд��־����
		m_pLogWriter = CLogThreadMgr::instance()->createLogWriter(pLogAppender);
	}
	return true; 
}

thp::WMTSLayer::~WMTSLayer()
{
	delete m_pLyrLRU;

	TBundleRecord* p = NULL, *pTemp = NULL;
	HASH_ITER(hh, m_pBundleRecords, p, pTemp) 
	{
		HASH_DEL(m_pBundleRecords, p);

		// �ͷ�bundle��Դ
		delete p;
	}
}

void thp::WMTSLayer::setCacheMbSize(unsigned int nMemByMB)
{
	// �����kb
	m_pLyrLRU->setCapacity( nMemByMB<<10 );
}

thp::WMTSLevel* thp::WMTSLayer::getLevel(int nLvl)
{
	if(nLvl<0 || nLvl >= THP_MAX_LEVEL)
		return 0;

	return m_pLvl[nLvl];
}

unsigned int thp::WMTSLayer::getTile(int nLvl, int nRow, int nCol, QByteArray& arTile, int& nDetail)
{
	if( nLvl > THP_MAX_LEVEL)
	{
		// log
		return 0;
	}// �����������Χ

	if( nRow < 0 || nRow > (1 << (nLvl-1)) )
	{
		// log
		return 0;
	}// ������Χ�������Ƭ����

	if( nCol < 0 || nCol > (1 << nLvl) )
	{
		// log
		return 0;
	}// ������Ƭ����
	
	// û�ж�Ӧ�ĵȼ�
	WMTSLevel* pLv = m_pLvl[nLvl];
	if( NULL == pLv )
		return 0;

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
			return 0;

		{
			//pthread_mutex_lock(&m_pRecordsMutex);
			QMutexLocker locker(&m_pRecordsMutex);

			HASH_FIND(hh, m_pBundleRecords, &(tbnNo.tID), sizeof(TBundleID), pRecord);
			if(NULL == pRecord)
			{
				pRecord = new TBundleRecord;

				pRecord->tID = tbnNo.tID;
				pRecord->nBundleRow = tbnNo.nBundleRow;
				pRecord->nBundleCol = tbnNo.nBundleCol;

				HASH_ADD(hh, m_pBundleRecords, tID, sizeof(TBundleID), pRecord);
			}

			//pthread_mutex_unlock(&m_pRecordsMutex);
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

int thp::WMTSLayer::loadData(int nLvl)
{
	if( nLvl > THP_MAX_LEVEL)
	{
		// log
		return 0;
	}// �����������Χ
	
	int nCount = _clacBundleCount(nLvl);
	int i = 0;
	TBundleIDex tId;
	
	return 0;
}

void WMTSLayer::_calcBundleNo(int nLvl, int nRow, int nCol, TBundleIDex& tNo)
{
	tNo.tID.unLv = (unsigned int)nLvl;

	if(nLvl < 8)
	{
		tNo.tID.unBundleIDinLv = 0;
		tNo.nBundleRow = 0;
		tNo.nBundleCol = 0;
		return ;
	}

	// �õȼ���Bundle������
	int nBundleRowNum = 1 << (nLvl - 8);
	tNo.nBundleRow = nRow >> 7;
	
	// ��Ƭ����Bundle���ڵ�������
	tNo.nBundleCol = nCol >> 7;
	
	tNo.tID.unBundleIDinLv = (unsigned int)( (tNo.nBundleCol * nBundleRowNum) + tNo.nBundleRow );
}

int thp::WMTSLayer::_clacBundleCount(int nLvl)
{
	if(nLvl < 8)
		return 1;

	// �õȼ���Bundle������
	return 1 << (2*nLvl - 15);
}

void thp::WMTSLayer::setGetTileStrategy(GetTileStrategy eGTS)
{
	m_eGTS = eGTS;
}

thp::WMTSLayer::GetTileStrategy thp::WMTSLayer::getGetTileStrategy() const
{
	return m_eGTS;
}

