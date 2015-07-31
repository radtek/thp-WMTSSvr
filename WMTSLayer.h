/*!
 * \file WMTSLayer.h
 * \author Xuebingbing
 * \brief һ��WMTSͼ�����
 *
 * TODO: 1.�����ⲿ��ȡTile; 2.���𴴽�bundle���������ͷ�
 * \date ���� 2015
 * \copyright	Copyright (c) 2015, �������ƺ�ͨ�����Զ����Ƽ����޹�˾\n
	All rights reserved.
 */

#ifndef THP_WMTS_H__
#define THP_WMTS_H__

#include <memory>
#include <QByteArray>
#include "ParamDef.h"
#include "bdi/bdiapi.h"
#include "uthash/uthash.h"
#include <QMutex>
#include <QReadWriteLock>
#include "Bundle.h"

#ifdef _THP_TJ
#include <Windows.h>
#endif// _THP_TJ

class CLogWriter;
namespace thp
{
	class Tile;
	class WMTSLevel;
	class LayerLRUCache;

	// Bundle��Դhash���¼
	struct TBundleRecord : public TBundleIDex
	{
		// ��Դ������
		std::tr1::weak_ptr<Bundle> wpBundle;

		// ��Դ��д��
		QReadWriteLock rwLocker;

		// hash����
		UT_hash_handle hh;

		// ������Դ
		std::tr1::shared_ptr<Bundle> loadBundle(WMTSLevel* pLv);

		TBundleRecord();
		~TBundleRecord();
	};

	class WMTSLayer 
	{
	public:
		/**< ��ȡtile���� */
		enum MemStrategy 
		{
			/**< ֱ��IO�����л��� */
			GTS_IO = 0,

			/**< ��IO�����뻺�� */
			GTS_MEM,
		};

	public:
		WMTSLayer();
		virtual ~WMTSLayer();

		bool setPath(const char* szPath);

		// 0 - success 1 - no dir
		// bdi �ļ�λ��
		virtual int init(const char* szBdiPath) = 0;

		// ��ȡ��Ӧ�ĵȼ�
		WMTSLevel* getLevel(int nLvl);

		/**
		* @brief 	 getTile ��ȡ��Ƭ
		* @details	 thp::WMTSLayer::getTile
		* @param[in] int nLvl   0...21
		* @param[in] int nRow	��Ƭ�к�
		* @param[in] int nCol	��Ƭ�к�
		* @param[in] int & nDetail	��ȡ����ϸ��Ϣ ����
		* @return 	 const Tile*
		* @todo 	
		*/
		virtual unsigned int getTile(int nLvl, int nRow, int nCol, QByteArray& arTile, int& nDetail) = 0;

		// ���û�������
		void setCacheMbSize(unsigned int nMemByMB);

		// �ڴ�������
		void setMemStrategy(MemStrategy eGTS);
		MemStrategy getMemStrategy() const;

		// װ������,����ʹ��
		int loadData(int nLvl);

	protected:
		// ����ָ���ȼ���bundle���
		void _calcBundleNo(int nLvl, int nRow, int nCol, TBundleIDex& tNo);

		// ����ȼ���bundle���������
		int _clacBundleCount(int nLvl);

		bool _initLogWriter();

	protected:
		int						nID;

		// ����ͼ�㷶Χ�Է�����Ϣ��ˢ�� �Ǳ��� ʵ��һ�Ż��������ٶ�
		double					m_dLeft;
		double					m_dBottom;
		double					m_dRight;
		double					m_dTop;

		// ���������
		LayerLRUCache*			m_pLyrLRU;

		// Bundle��Դ�б�
		TBundleRecord*			m_pBundleRecords;

		// Bundle��Դ�б�����,ֻ��add��Դ��ʱ������
		QMutex						m_pRecordsMutex;

		// Bundleװ�ڲ��� 
		MemStrategy			m_eGTS;

		// ��־��д��
		CLogWriter *			m_pLogWriter;

		char m_szPath[THP_MAX_PATH];

		WMTSLevel* m_pLvl[THP_MAX_LEVEL];

#ifdef _THP_TJ
		// �ܷ��ʴ���
		LONG			m_nCount;

		// ��Ч���ʴ���
		LONG			m_nENum;

		// ���ڴ��ȡ�Ĵ���
		LONG			m_nMNum;
#endif// _THP_TJ
	};
}


#endif // THP_WMTS_H__