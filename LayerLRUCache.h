#ifndef THP_WMTS_LRUCACHE_H__
#define THP_WMTS_LRUCACHE_H__
#include <Windows.h>
#include <list>

#include <QReadWriteLock>
#include "ParamDef.h"
#include "Bundle.h"
#include "dclist.h"
#include <QHash>
#include <QString>

class CLogWriter;

namespace thp
{
class Bundle;
class BundleFactory;
class CircularList;

// ʹ��QHash����ΪQHash����ӣ�ɾ������ѯʱ��Ƚ�ƽ��
class LayerLRUCache
{
public:
	enum GetType 
	{
		G_SUCCESS = 0,
		G_FAIL,
	};

	typedef std::tr1::shared_ptr<thp::Bundle> sPtr;

	// 2^32 kb = 2^22MB = 2^12 Gb = 2^2 Tb 
	// ��λ KB 
	LayerLRUCache(unsigned int nCapacity);
	~LayerLRUCache();

	// ��λ KB
	unsigned int getCapacity();
	void setCapacity(unsigned int nCapacity); 

	// ����һ�ν���
	// �����߶�lru����ӽ���������ֱ�ӷŵ�ͷ���������г�ջ����
	int add( std::tr1::shared_ptr<Bundle> sp );

	bool lockForRead();
	bool lockForWrite();
	void unlock(bool bRead);

	bool isFull();

	bool _initLogWriter();

	void showStatus();
private:

	// �����쳣��׽
	thp::Bundle* get_pri(const TBundleIDex& key);

private:
	
	// ����ʹ�û����� ʹ�ö�д����������
	// �����������Ա�����Ķ�д��
	QReadWriteLock m_prwMutex;


	// ��λ KB
	unsigned int m_unMaxKBCount ;
	unsigned int m_unUsedKBCount;
	unsigned int m_unLruHeadBundleKBCount;

	dclist<sPtr> m_listHotColdResources;

	// val ����
	QHash<QString,int> m_hmapResources;

	CLogWriter *					m_pLogWriter;
#ifdef _DEBUG
	LONG m_lockReadTimes;
	LONG m_lockWriteTimes;
#endif
};

}
#endif // THP_WMTS_LRU_H__
