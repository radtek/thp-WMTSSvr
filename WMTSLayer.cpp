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

#ifdef _THP_TJ
	m_nCount = 0;
	m_nENum = 0;
	m_nMNum = 0;
#endif// _THP_TJ

	// ��ϣ������Դ����1000��(��)ʱ������ʱ���������Դ
	m_nMaintainLine = 100000000;
	m_nSpan = 200;

	_initLogWriter();
}

thp::WMTSLayer::~WMTSLayer()
{
	delete m_pLyrLRU;
	m_pLyrLRU = NULL;

	int nCount = HASH_COUNT(m_pBundleRecords);

	std::cout << "delete hash..." << std::endl;
	std::cout << "bundle count:" << nCount << std::endl;

	TBundleRecord* p = NULL, *pTemp = NULL;
	HASH_ITER(hh, m_pBundleRecords, p, pTemp) 
	{
		HASH_DEL(m_pBundleRecords, p);

		// �ͷ�bundle��Դ
		delete p;
	}
	std::cout << "hash deleted" << std::endl;

	std::cout << "delete level..." << std::endl;
	for (int i = 0; i < THP_MAX_LEVEL; ++i)
	{
		WMTSLevel* pLv = m_pLvl[i];
		delete pLv;
		pLv = NULL;
	}
	std::cout << "level deleted" << std::endl;
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
	unsigned int unLen = (unsigned int)strlen(szPath);
	if(0 == unLen || unLen > THP_MAX_PATH)
		return false;

	memcpy(m_szPath, szPath, unLen);

	return true;
}

void thp::WMTSLayer::showStatus()
{
	QMutexLocker locker(&m_pRecordsMutex);

	TBundleRecord* p = NULL, *pTemp = NULL;

	std::cout << "-----------hash status---------" << std::endl;
	int nCount = HASH_COUNT(m_pBundleRecords);
	std::cout << "hash count :" << nCount << std::endl;
	std::cout << "--in memory--" << std::endl;
	int nCountInMem = 0;
	HASH_ITER(hh, m_pBundleRecords, p, pTemp) 
	{
		if( !p->wpBundle.expired() )
		{
			std::tr1::shared_ptr<Bundle> spBundle = p->wpBundle.lock();

			if( spBundle->isCached() )
			{
				std::cout << spBundle->getPath() << std::endl;
				++nCountInMem;
			}
		}
	}
	std::cout << "memory count :" << nCountInMem << std::endl;

	std::cout << "-----------hash END-------------------" << std::endl;

	std::cout << "-----------LRU status-------------------" << std::endl;

	m_pLyrLRU->showStatus();

	std::cout << "-----------LRU END-------------------" << std::endl;
}

void thp::WMTSLayer::maintain(bool bForce)
{
	QMutexLocker locker(&m_pRecordsMutex);

	int nRecordCount = HASH_COUNT(m_pBundleRecords);
	if(nRecordCount < m_nMaintainLine)
		return ;

	TBundleRecord* p = NULL, *pTemp = NULL;

	std::string sLayerName = getName();
	std::cout << "����ά��ͼ��["<< sLayerName << "]..." << std::endl;

	bool bMaintainedData = false;

	if(bForce)
	{
		// ��¼Ҫɾ���ļ�¼
		HASH_ITER(hh, m_pBundleRecords, p, pTemp) 
		{
			// ɾ�������ڴ��е����м�¼
			if( p->wpBundle.expired() )
			{
				HASH_DEL(m_pBundleRecords, p);

				// �ͷ�bundle��Դ
				delete p;
				bMaintainedData = true;
			}// �Ѿ������ڴ���ֱ��ɾ��
			else
			{
				std::tr1::shared_ptr<Bundle> spBundle = p->wpBundle.lock();

				if( !spBundle->isCached() )
				{
					HASH_DEL(m_pBundleRecords, p);

					// �ͷ�bundle��Դ
					delete p;
					bMaintainedData = true;
				}
			}// ������ڻ����¼�е� 
		}
	}
	else
	{
		clock_t tBegin = clock();
		clock_t tMsBase = CLOCKS_PER_SEC / 1000;

		// ��¼Ҫɾ���ļ�¼
		HASH_ITER(hh, m_pBundleRecords, p, pTemp) 
		{
			// ɾ�������ڴ��е����м�¼
			if( p->wpBundle.expired() )
			{
				HASH_DEL(m_pBundleRecords, p);

				// �ͷ�bundle��Դ
				delete p;
				bMaintainedData = true;
			}// �Ѿ������ڴ���ֱ��ɾ��
			else
			{
				std::tr1::shared_ptr<Bundle> spBundle = p->wpBundle.lock();

				if( !spBundle->isCached() )
				{
					HASH_DEL(m_pBundleRecords, p);

					// �ͷ�bundle��Դ
					delete p;
					bMaintainedData = true;
				}
			}// ������ڻ����¼�е� 
			
			if ( (clock() - tBegin)/tMsBase > m_nSpan )
				break;
		}
	}

	if(bMaintainedData)
	{
		std::cout << "ͼ��["<< sLayerName << "]ά�����" << std::endl;
	}
}

std::string thp::WMTSLayer::getName() const
{
	std::string sPath( m_szPath );

	if(sPath.empty())
		return "";

	size_t nSize = sPath.size();
	size_t nPos0 = sPath.rfind('\\');
	if(std::string::npos == nPos0)
		nPos0 = sPath.rfind('/');

	std::string sName = sPath.substr(nPos0+1, (nSize - nPos0));

	return sName;
}

