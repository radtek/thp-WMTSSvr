#ifndef THP_WMTS_LRUCACHE_H__
#define THP_WMTS_LRUCACHE_H__
#include <map>
#include "pthread/pthread.h"
#include <Windows.h>
#include <set>
#include <list>
#include "ParamDef.h"

namespace thp
{
class Bundle;
class BundleFactory;
class CircularList;

class LayerLRUCache
{
public:

	// 2^32 kb = 2^22MB = 2^12 Gb = 2^2 Tb 
	// ��λ KB 
	LayerLRUCache(unsigned int nCapacity);
	~LayerLRUCache();

	// ��λ KB
	unsigned int getCapacity();
	void setCapacity(unsigned int nCapacity); 

	// ����ʹ�ö���
	// ����NULLʱ�������д��
	thp::Bundle* get(const TBundleNo& key);

	// ����һ�ν���
	// �����߶�lru����ӽ���������ֱ�ӷŵ�ͷ���������г�ջ����
	thp::Bundle* addAndGet(const TBundleNo& key, BundleFactory* pFactory);

	bool lockForRead();
	bool lockForWrite();
	void unlock(bool bRead);

private:

	// �����쳣��׽
	thp::Bundle* get_pri(const TBundleNo& key);

private:
	
	// ����ʹ�û����� ʹ�ö�д����������
	// �����������Ա�����Ķ�д��
	pthread_rwlock_t m_prwMutex;

	//pthread_mutex_t m_ptMutex;

	// ��λ KB
	unsigned int m_unMaxKBCount ;
	unsigned int m_unUsedKBCount;
	unsigned int m_unLruHeadBundleKBCount;
	// std::list< std::pair<TBundleNo, thp::Bundle*> > m_listCache;
	// ʹ��hash_map�滻Ч�ʿ��Ը��� 
	typedef std::map<TBundleNo, thp::Bundle*> TbnoBdlMap;
	typedef TbnoBdlMap::iterator	TbnoBdlMapIt;

	// bundle ��Դ��
	TbnoBdlMap m_mapKeyValue;

	// ��Դ�ȶ�ѭ������ 
	CircularList* m_pList;

	LONG m_lockReadTimes;
	LONG m_lockWriteTimes;
};

}
#endif // THP_WMTS_LRU_H__
