#ifndef THP_WMTS_LRUCACHE_H__
#define THP_WMTS_LRUCACHE_H__
#include <map>
#include <list>
#include <QReadWriteLock>
#include "pthread/pthread.h"
#include <Windows.h>

namespace thp
{
class Bundle;

// Bundle ���
struct TBundleNo
{
	// �����ڴ�ռ�ÿ�����1char
	unsigned int unLv;

	// �ڵȼ��ڵı��
	// nLv < 8 nBundleID = 0; nLv> 8  nBundleID = [0, 2^(2*nLv-15))
	// ��Ŷ�Ӧ
	// nLv >= 8  �� 2^(nLv-8) row, ��2^(nLv-7) ��
	// ���±�ʾ��Ŷ�Ӧ��bundle��λ��
	// 0   1*(2^(nLv-8))   ...
	// 1   .
	// .   .
	// .   .
	// .   .
	unsigned int unBunldeID;

	bool operator==(const TBundleNo& tNoNew) const;
	bool operator<(const TBundleNo& tNoNew) const;

	TBundleNo();
};

class LayerLRUCache
{
public:

	// 2^32 kb = 2^22MB = 2^12 Gb = 2^2 Tb 
	// ��λ KB 
	LayerLRUCache(unsigned int nCapacity);

	// ��λ KB
	unsigned int getCapacity();
	void setCapacity(unsigned int nCapacity); 

	// ����ʹ�ö���
	thp::Bundle* get(const TBundleNo& key);
	void set(const TBundleNo& key, thp::Bundle* pBundle);

	// �������ʹ�õ� bundle ռ���ڴ���Ϣ ����һ��lruռ���ڴ����
	void saveUsedCapacityStatus();
	void updateUsedCapacity();

	bool lockForRead();
	bool lockForWrite();
	void unlock(bool bRead);

private:

	// �����쳣��׽
	thp::Bundle* get_pri(const TBundleNo& key);
	void set_pri(const TBundleNo& key, thp::Bundle* pBundle);

private:
	// ��λ KB
	unsigned int m_unMaxKBCount ;
	unsigned int m_unUsedKBCount;
	unsigned int m_unLruHeadBundleKBCount;

	//QMutex m_qmutex;
	//QReadWriteLock m_qrwMutex;
	pthread_rwlock_t m_prwMutex;

	pthread_mutex_t m_plock;

	std::list< std::pair<TBundleNo, thp::Bundle*> > m_listCache;
	std::map<TBundleNo, std::list< std::pair<TBundleNo, thp::Bundle*> >::iterator > m_mp;

	LONG m_lockReadTimes;
	LONG m_lockWriteTimes;
};

}
#endif // THP_WMTS_LRU_H__
