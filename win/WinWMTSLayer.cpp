#include "../StdAfx.h"
#include "WinWMTSLayer.h"
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

using namespace thp;

WinWMTSLayer::~WinWMTSLayer()
{
	_clear();
}

int WinWMTSLayer::init(const char* szBdiPath)
{
	if( -1 == _access(m_szPath, 0) )
	{
		// write log WMTS ����ͼ�����ݳ�ʼ��ʧ��,ԭ��ͼ��Ŀ¼������
		return 1;
	}

	if( -1 == _access(szBdiPath, 0) )
	{
		// write log WMTS ����ͼ�����ݳ�ʼ��ʧ��,ԭ��ͼ��Ŀ¼������
		printf("ָ��bdi�ļ�[%s]������", szBdiPath);
		return 1;
	}

	std::map<int, TLevelBundleExistStatus*> mapBdi;
	readLayerDbi(szBdiPath, mapBdi);

	int nRes = _initLevels(mapBdi);

	if(  !mapBdi.empty() )
	{
		std::map<int, TLevelBundleExistStatus*>::iterator it = mapBdi.begin();
		for( ;it != mapBdi.end(); ++it)
			delete it->second;
	}

	return nRes>0?0:1;
}

unsigned int WinWMTSLayer::getTile(int nLvl, int nRow, int nCol, QByteArray& arTile, int& nDetail)
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

int thp::WinWMTSLayer::_initLevels(const std::map<int, TLevelBundleExistStatus*>& mapBdi)
{
	char szLvlPath[THP_MAX_PATH];
	memset(szLvlPath, 0, THP_MAX_PATH);
	std::map<int, TLevelBundleExistStatus*>::const_iterator it = mapBdi.begin();
	int nSuccessLv = 0;
	for (int i=0; i < THP_MAX_LEVEL; ++i)
	{
		it = mapBdi.find(i);
		if(mapBdi.end() == it)
		{
			//std::cerr<<"��������"<<std::endl;
			return -1;
		}

		const TLevelBundleExistStatus* pNode = it->second;
		if( NULL == pNode || 0 == pNode->nSize )
		{
			m_pLvl[i] = NULL;
			continue;
		}

		if( 0 != _initLevel(szLvlPath, i, pNode) )
		{
			m_pLvl[i] = NULL;
		}

		++nSuccessLv;
	}

	return nSuccessLv;
}

int thp::WinWMTSLayer::_initLevel(char* szLvlPath , int nLvl, const TLevelBundleExistStatus* pNode)
{
	sprintf(szLvlPath, "%s\L%02d\\", m_szPath, nLvl);
	// ��ʼ�� level
	if( 0 == _access(szLvlPath, 0) )
	{
		m_pLvl[nLvl] = new WMTSLevel(nLvl);
		m_pLvl[nLvl]->setPath(szLvlPath);

		m_pLvl[nLvl]->setBdi(pNode->nSize, pNode->pbyteIndex);

		return 0;
	}// success
	else
	{
		return 1;
	}// fail
}

void thp::WinWMTSLayer::_clear()
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


