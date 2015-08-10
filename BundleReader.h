/*!
 * \file BundleReader.h
 * \author Xuebingbing
 * \date ���� 2015
 * \copyright	Copyright (c) 2015, �������ƺ�ͨ�����Զ����Ƽ����޹�˾\n
	All rights reserved.
 */
#ifndef THP_BOUNDLEREADER_H__
#define THP_BOUNDLEREADER_H__
#include "./BundleParamDef.h"
#include <stdio.h>
#include <fstream>

//class CLogWriter;
namespace thp
{
	class Bundle;
	/*!
	 * \class BundleReader
	 *
	 * \brief ArcGis ��Ƭ����ѹ���ļ���ȡ��
	 * \detail bundle��bundlx������ͬһĿ¼�£��������ļ��������bundle�ļ�����ͬ
	 * \author Baldwin
	 * \date ���� 2015
	 */
	class BundleReader
	{
	public:
		
		/**< ��ȡ��Ƭ�������� */
		enum FetchType
		{
			// ��ȡ�ɹ�
			FetchType_Success = 0, 
			
			// ��ȡʧ��
			FetchType_Fail,

			// ��ȡ�����
			FetchType_EOF,
		};


		BundleReader();
		
		// �ļ���ַ eg��.\\L09\\R0080C0180.bundle
		BundleReader(const char* szBundleFile);

		// ���಻������Ϊ�̳еĸ���
		virtual ~BundleReader();
	
		// �����ļ���ַ eg��.\\L09\\R0080C0180.bundle
		virtual bool open(const char* szFile) = 0;

		void close();

		// ��������
		bool copyBundlx(char*& pBlx, int nSize = BUNDLX_CONTENT_SIZE);
		bool loadBundlx(char* pBlx, int nSize = BUNDLX_CONTENT_SIZE);

		// ����ȫbundle���� ����bundle�ļ���byte��
		virtual int readAll(char*& pBundle) = 0;

		// ��ȡ����������Ч
		// ��ȡ����bundle�ļ���Ҫ���Ļ���
		//unsigned int getMaxCacheSizeKB();

		// ��ȡbundle�ļ���С
		virtual unsigned int getMaxByte() = 0;

		/**
		* @brief 	 getTileFromFile
		* @details	 thp::BundleReader::getTileFromFile
		* @param[in] int nTileInBundleIndex
		* @param[in] unsigned char * & pByteTile	������ʹ�� delte[] �ͷ��ڴ�
		* @param[in] unsigned int & nSize			Tile�Ķ����ƿ��С
		* @return 	 bool							���ڷ���true�����򷵻�false
		* @todo 	���������,��������Ԥ������
		*/
		virtual	bool getTileFromFile(int nTileInBundleIndex, unsigned char*& pByteTile, unsigned int& nSize) = 0;

		/**
		* @brief 	 nextTile
		* @details	 ��ȡ�ӵ�ǰλ�ÿ�ʼ��һ�ſ��õ���Ƭ�������ڲ��������ƶ�����һ��λ��
		* @param[in,out] int & nRow  ��Ƭ���к�
		* @param[in,out] int & nCol  ��Ƭ�к�
		* @param[in,out] unsigned char * & pOut  ��Ƭ�Ķ�����bolb, �ÿ����ͨ�� #releaseMem �����ͷ�\n
		����ͨ�� C-API free() �ͷ�
		* @param[in,out] int & nSize	tile���ֽ���
		* @return 	 thp::BundleReader::FetchType  ��ȡ���ݲ���
		*/
		FetchType nextTile(int& nRow, int& nCol, unsigned char*& pOut, int& nSize/*, std::ofstream* pOsInfo*/);

	protected:
		bool _calcWorldRowCol(int nIdxPos, int& nRow, int& nCol);
		//bool _initLogWriter();

		// 
		//
		// ����tile��bundle�ڲ��ı�� -1 ��������� -2 �ڴ治��
		int _nextTile(unsigned char*& pOut,unsigned int& nSize);

		// ��������ļ�����
		unsigned char		m_szBundlxContents[BUNDLX_CONTENT_SIZE];
		
		char				m_szLastErr[BUNDLE_MAX_PATH];

		// 
		char				m_szBundleFile[BUNDLE_MAX_PATH];

		// tile ��bundle�ڵı�� ����λ�� 0---16383
		int					m_nIdxPos;

		// ���ļ����õ��� bundle ������
		int					m_nBundleBeginRow;
		int					m_nBundleBeginCol;

		int					m_nMAx;
		//CLogWriter *		m_pLogWriter;
#ifdef _DEBUG
		// ���õ�
		int					m_nValdTileCount;
#endif// _DEBUG
	};


}// ���ƺ�ͨ


#endif // THP_BOUNDLEREADER_H__