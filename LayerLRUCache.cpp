#include "StdAfx.h"
#include "LayerLRUCache.h"
#include "Bundle.h"
//#include "glog/logging.h"
//#include "glog/raw_logging.h"
#include <set>
#include "dclist.h"
#include  <CLogThreadMgr.h>
#ifdef _DEBUG
#include <ctime>
#include <Windows.h>
#endif

using namespace thp;
LayerLRUCache::LayerLRUCache(unsigned int unCapacity) 
{
	//pthread_rwlock_init(&m_prwMutex, NULL);

#ifdef _DEBUG
	m_lockReadTimes = 0;
	m_lockWriteTimes = 0;
#endif

	bool bLocked = lockForWrite();

	m_unMaxKBCount = unCapacity;
	m_unUsedKBCount = 0;
	m_unLruHeadBundleKBCount = 0;

	//m_pBundles = NULL;
   _initLogWriter();
	unlock(false);
}

bool LayerLRUCache::_initLogWriter()
{
	// ��ȡд��־����
	m_pLogWriter = CLogThreadMgr::instance()->getLogWriter("LayerLRUCache.log");

	// ��������ڣ�������־�ļ���������־����
	if (m_pLogWriter == NULL)
	{
		CLogAppender * pLogAppender = new CLogAppender("LayerLRUCache", "LayerLRUCache.log", "", "DebugLog"); 

		// ��ȡд��־����
		m_pLogWriter = CLogThreadMgr::instance()->createLogWriter(pLogAppender);
	}
	return true; 
}

thp::LayerLRUCache::~LayerLRUCache()
{
	// �ͷ�hash����Դ
	lockForWrite();

	m_pLogWriter->debugLog("LRU �����ͷ���Դ...");
	//LOG(INFO) << "LRU �����ͷ���Դ...";

	unlock(false);

	//pthread_rwlock_destroy(&m_prwMutex);

	m_pLogWriter->debugLog("LRU ����ͷ���Դ...");
	//LOG(INFO) << "LRU ����ͷ���Դ";
}


void LayerLRUCache::setCapacity(unsigned int unCapacity)
{
	bool bLocked = lockForWrite();
	if( !bLocked )
		return ;

	m_unMaxKBCount = unCapacity;

	unlock(false);
}

unsigned int LayerLRUCache::getCapacity()
{
	return m_unMaxKBCount;
}

bool thp::LayerLRUCache::lockForRead()
{
	m_prwMutex.lockForRead();
	return true;
}

bool thp::LayerLRUCache::lockForWrite()
{
	m_prwMutex.lockForWrite();
	return true;
}

void thp::LayerLRUCache::unlock(bool bRead)
{
	m_prwMutex.unlock();
}

bool thp::LayerLRUCache::isFull()
{
	return m_unUsedKBCount >= m_unMaxKBCount;
}

int thp::LayerLRUCache::add(std::tr1::shared_ptr<Bundle> sp)
{
	lockForWrite();

	if(m_unUsedKBCount < m_unMaxKBCount)
	{
		m_listHotColdResources.push_front(sp);
		
		unlock(false);
		return 0;
	}
	
	while(m_unUsedKBCount > m_unMaxKBCount)
	{
		sPtr sp = m_listHotColdResources.back();
		while( sp->getTemperature() > 2)
		{
			sp->setTemperature(0);
			m_listHotColdResources.move_head_forward();
			continue;
		}

		if( m_unUsedKBCount < sp->getMaxKB() )
		{
			m_pLogWriter->errorLog("LRU����");
			//LOG(ERROR) << "LRU����";
			break;
		}

		m_unUsedKBCount -= sp->getMaxKB();
	
		m_listHotColdResources.pop_back();
	}

	m_unUsedKBCount += sp->getMaxKB();

	unlock(false);
	return 0;
}



