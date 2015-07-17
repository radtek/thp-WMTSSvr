#ifndef BUNDLE_H__
#define BUNDLE_H__

#include <memory>

#include <QByteArray>

#ifdef _DEBUG
#include <Windows.h>
#endif

#include "ParamDef.h"

class CLogWriter;

namespace thp
{
	class Tile;
	class BundleReader;
	class LayerLRUCache;

/*!
 * \class Bundle
 *
 * \brief �Ըö���Ϊ��λ�����ڴ棬ʹ��lru�㷨�����ڴ�
 * \detail һ����Ƭ, 128 * 128 �����е���Ƭλһ��
 * \author Baldwin
 * \date ���� 2015
 */
class Bundle
{
public:
	Bundle(const TBundleIDex& tNoEx);
	~Bundle();

	// �����ļ���ַ eg��.\\L09\\R0080C0180.bundle
	bool open(const char* szFile); 

	// ���bundleʹ�õ���Դ
	void close();

	// ��Ƭ���浽�ڴ�
	// �����Լ����ڴ�
	int cache();
	bool isCached();

	// ��ȡ���ռ���ڴ�
	unsigned int getMaxKB();

	/**
	* @brief 	 getTile
	* @details	 Bundle::getTile
	* @param[in] int nRow Tile ȫ���к�
	* @param[in] int nCol tile ȫ���к�
	* @return 	��ȡ���ǿ�tile����true�����򷵻�false
	* @todo 	
	*/
	unsigned int getTile(int nRow, int nCol, QByteArray& arTile,int &nDetail);

	// ������Դ
	void heating();
	void setTemperature(int nDegree);
	int getTemperature() const;

	// ��ȡBundle�ı��
	const TBundleID& getID() const;

	const char* getPath() const;

#ifdef _DEBUG
	void check();
#endif

private:
	void _getMaxByte();
	bool _initLogWriter();
	unsigned int _getTileFromCache(int nTileIndexInBundle, QByteArray& arTile);

//public:
private:
	// ��ʼ�к� ֻ��128��������
	int					m_nBeginRow;

	// ��ʼ�к� ֻ��128��������
	int					m_nBeginCol;

	// ��ȡ����ռ���ڴ�
	unsigned int		m_unMaxByteSize;

	// ��ʾBundle�Ƿ���������
	bool				m_bCached;

	// bundle�ļ���ַ
	char				m_szFilePath[THP_MAX_PATH];

	// ������������Ϣ
	char				m_szLastErr[256];

	// �����д��
	//pthread_rwlock_t	m_prwMutex;

	// ���
	TBundleIDex			m_tID;

	// ��־��
	CLogWriter*			m_pLogWriter;
	
	// ʹ���ȶ�, 0λ���
	int m_nHeatDegree;

	// bundle��Դ����
	char*				m_pBlx;

	// bundle��Դ����
	char*				m_pBle;

#ifdef _DEBUG
	LONG m_lockReadTimes;
	LONG m_lockWriteTimes;
#endif
};//

}// namespace thp


#endif // BUNDLE_H__
