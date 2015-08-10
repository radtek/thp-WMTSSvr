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
#endif// _DEBUG

#ifdef _THP_TJ
#include <QString>
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
	m_pLogWriter = CLogThreadMgr::instance()->getLogWriter("LayerLRUCache.csv");

	// ��������ڣ�������־�ļ���������־����
	if (m_pLogWriter == NULL)
	{
		CLogAppender * pLogAppender = new CLogAppender("LayerLRUCache", "LayerLRUCache.csv", "", "DebugLog"); 

		// ��ȡд��־����
		m_pLogWriter = CLogThreadMgr::instance()->createLogWriter(pLogAppender);
	}
	return true; 
}

thp::LayerLRUCache::~LayerLRUCache()
{
	// �ͷ�hash����Դ
	lockForWrite();

	std::cout << "LRU �����ͷ���Դ..." << std::endl;

	unlock(false);

	std::cout << "LRU ����ͷ���Դ" << std::endl;
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

	// �����Դ�Ѿ�����
	QString qsName = sp->getName();
	QHash<QString,int>::iterator it = m_hmapResources.find(qsName);
	if(it != m_hmapResources.end())
	{
		unlock(false);
		return 0;
	}

	if(m_unUsedKBCount < m_unMaxKBCount)
	{
		// ������Դ��lruͷ��
		m_hmapResources.insert(qsName, 0);
		m_listHotColdResources.push_front(sp);

#ifdef _THP_TJ
		unsigned int nbKb = sp->getMaxKB();
		QString qsInfo = QString( "%0,%1,%2" ).arg( GB("LRU") ).arg( GB(sp->getPath()) ).arg( nbKb );
		m_pLogWriter->debugLog(qsInfo);
#endif

		m_unUsedKBCount += sp->getMaxKB();
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
			break;
		}

		m_unUsedKBCount -= sp->getMaxKB();
	
		m_listHotColdResources.pop_back();
		m_hmapResources.remove(qsName);
	}

	// ������Դ
	m_listHotColdResources.insert_n(m_listHotColdResources.size()/2, sp);
	m_hmapResources.insert(qsName, 0);

	// ά��ռ��ֵ
	m_unUsedKBCount += sp->getMaxKB();

#ifdef _THP_TJ
	// CLASS, MAX, CURRENT, BUNDLE����
	QString qsInfo = QString( GB("%0,%1,%2,%3") ).arg( GB("LRUSTATUS") ).arg( m_unMaxKBCount ).arg( m_unUsedKBCount ).arg( m_listHotColdResources.size() );
	m_pLogWriter->debugLog(qsInfo);
#endif

	unlock(false);
	return 0;
}

void thp::LayerLRUCache::showStatus()
{
	std::cout << "max kb:" << m_unMaxKBCount << std::endl;

	std::cout << "occ kb:" << m_unUsedKBCount << std::endl;

	std::cout << "list size:"<< m_listHotColdResources.size() << std::endl;

	dclist<sPtr>::iterator it = m_listHotColdResources.begin();
	for (int i=0;it != m_listHotColdResources.end();++it,++i)
	{
		sPtr sp = *it;
		std::cout << i << " " << sp->getPath() << std::endl;
	}
}



