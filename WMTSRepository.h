/*!
 * \file WMTS.h
 * \author Xuebingbing
 * \brief wmts���ݲֿ�
 *
 * TODO: long description
 * \date ���� 2015
 * \copyright	Copyright (c) 2015, �������ƺ�ͨ�����Զ����Ƽ����޹�˾\n
	All rights reserved.
 */

#ifndef THP_WMTSREPOSITORY_H__
#define THP_WMTSREPOSITORY_H__
#include <string>
#include "ParamDef.h"
#include <memory>
#include <QMutex>
#include <QByteArray>
#include "uthash/uthash.h"

class CLogWriter;
namespace thp
{
	class Tile;
	class WMTSLayer;

	// ��ϣ��ṹ����
	struct TLayerHashTableNode
	{
		// ͼ����
		char szName[THP_WMTS_MAX_LAYERLEN];

		// ͼ�����ָ��
		thp::WMTSLayer* pLayer;

		// hash ����
		UT_hash_handle hh;
	};

	/*!
	 * \class WMTS
	 *
	 * \brief WMTS��Ƭ���ݲֿ�
	 *
	 * \author Baldwin
	 * \date ���� 2015
	 */
	class WMTSRepository
	{
	public:
		WMTSRepository();
		~WMTSRepository();

		// eg: .\Layers\_alllayers\L18\  ����β'\'
		/**
		* @brief 	 setPath ���÷���Ŀ¼
		* @details	 eg: .\Layers\_alllayers\L18\, Ŀ¼ĩβ������'\'
		* @param[in] const char * szPath ����Ŀ¼��ͼ�������ڵ�Ŀ¼
		* @return 	 bool ���óɹ��Ż� true
		* @todo 	ͼ���������� С�������ɵ� [ͼ����].bdi �ļ�
		û��bdi�ļ���Ϊû��ͼ��
		*/
		bool setPath(const char* szPath);
		
		/**
		* @brief 	 init ��ʼ������
		* @details	 thp::WMTS::init
		* @param[in] int nMode
			0x0001--��������Ŀ¼bdi�ļ���ʽ��ʼ��
			0x0002--ʹ�������ļ���ʽ��ʼ��,δʵ��
		* @return 	 bool ��ʼ���ɹ�����true
		* @todo 	
		*/
		bool init(int nMode);

		/**
		* @brief 	 getTile	��ȡָ����Ϣ����Ƭ
		* @details	 thp::WMTS::getTile
		* @param[in] std::string & strLayer	��Ƭ����ͼ��
		* @param[in] int nLvl	��Ƭ�ȼ�
		* @param[in] int nRow	��Ƭ�к�
		* @param[in] int nCol	��Ƭ�к�
		* @param[in] Tile * pTile  ��ȡ������Ƭ��Ӧ����ָ��ָ���װ
			���ڵ�����ʹ�÷�ʽ��lru��С��ͬʱ����һ�������tile����ʱ�������Ұָ��
		* @return 	 int �Է���ֵ��������ϸ����Ϣ����Ŀǰֵ����
			0	�ɹ�
			1	ʧ��
		* @todo 	����
		*/
		// �����ֽ���
		unsigned int getTile(const std::string& strLayer, int nLvl, int nRow, int nCol, QByteArray& arTile, int& nDetail);
		
		/**
		* @brief 	 setMaxOccupyMemory     ���û������ޣ�
		* @details	 thp::WMTS::setMaxOccupyMemory
		* @param[in] unsigned int nMemByMB
		* @return 	 void
		* @todo 	��λMB
				1������ͼ������
				2��ͼ���ʹ���ڴ����ʹ��Ƶ��ֻ�ܷ���
				3��ÿ��ͼ�㶼ʹ�����ֵ��Ϊ��������
		*/
		void setCacheMbSize(unsigned int nMemByMB);
		int getCacheMbSize() const;

		// ͼ����Դ���Ȳ���
		void setMemStrategy(int nMemStrategy);
		int getMemStrategy() const;

		// װ��ָ��ͼ��ָ���ȼ���bundle 
		// 0-ȫ��װ�� 1-����װ�� 2-û��bundle��װ��
		// ָ��ͼ����lru���˾Ͳ���װ���� 
		int loadData(const std::string& strLayer, int nLvl);

	private:
		int _initByDir();
		bool _initByConfig();
		bool _initLayer(const char* szLayer, const char* szBdiPath);
		void getTile_trans(const std::string& strLayer, int nLvl, int nRow, int nCol, int& nDetail, std::tr1::shared_ptr<Tile>& spRes);
		void getTile_pri(const std::string& strLayer, int nLvl, int nRow, int nCol, int& nDetail, std::tr1::shared_ptr<Tile>& spRes);
		bool _initLogWriter();

	private:
		// ���ռ���ڴ� ��λ MB
		int						m_unMaxOccupyMemMb;

		// �������ݲֿ�Ŀ¼
		char					m_szPath[THP_MAX_PATH];

		// ͼ�� hash ��
		TLayerHashTableNode*	m_layers;

		// ͼ����Դ���Ȳ���
		int						m_nMemStrategy;

		// ��־��
		CLogWriter*				m_pLogWriter;
	};

}

#endif // THP_WMTSREPOSITORY_H__
