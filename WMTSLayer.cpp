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
}

thp::TBundleRecord::~TBundleRecord()
{
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
}

bool WMTSLayer::_initLogWriter()
{
	// ��ȡд��־����
	m_pLogWriter = CLogThreadMgr::instance()->getLogWriter("WMTSLayer.csv");

	// ��������ڣ�������־�ļ���������־����
	if (m_pLogWriter == NULL)
	{
		CLogAppender * pLogAppender = new CLogAppender("WMTSLayer", "WMTSLayer.csv", "", "DebugLog"); 

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

void thp::WMTSLayer::setMemStrategy(MemStrategy eGTS)
{
	m_eGTS = eGTS;
}

thp::WMTSLayer::MemStrategy thp::WMTSLayer::getMemStrategy() const
{
	return m_eGTS;
}

bool thp::WMTSLayer::setPath(const char* szPath)
{
	unsigned int unLen = strlen(szPath);
	if(0 == unLen || unLen > THP_MAX_PATH)
		return false;

	memcpy(m_szPath, szPath, unLen);

	return true;
}

