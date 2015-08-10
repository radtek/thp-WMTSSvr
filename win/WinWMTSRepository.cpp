#include "../StdAfx.h"
#include "WinWMTSRepository.h"
#include <io.h>
#include <sstream>
#include "../Tile.h"
#include "WinWMTSLevel.h"
#include <iostream>
#include <algorithm>
#include "../WMTSConfig.h"
#include <CLogThreadMgr.h>
#include "../WMTSFactory.h"
#include "../WMTSLayer.h"

using namespace thp;

thp::WinWMTSRepository::WinWMTSRepository()
{

}

thp::WinWMTSRepository::~WinWMTSRepository()
{

}

bool thp::WinWMTSRepository::init(int nMode)
{
	// ���ļ��ṹ��ʼ��
	bool bSuccess = false;
	int nLayerCount = 0;
	int nFileSysType = WMTSConfig::Instance()->getFileSysType();
	if( 0x0001 == (nMode&0x0001) )
	{

		if( 0 == nFileSysType )
			nLayerCount = _initByDir();
		else
			nLayerCount = 0;
	}

	if(nLayerCount > 0)
		return true;

	return bSuccess;
}

int thp::WinWMTSRepository::_initByDir()
{
	// ����bdi�ļ�
	struct _finddata64i32_t fileInfo;
	long handle;
	int done;
	QString qsFileName = WMTSConfig::Instance()->getDataDir();
	std::string sFileName = (const char*)qsFileName.toLocal8Bit(); //Ҫ�������ļ���
	std::string sFileFullPath = sFileName + "*";
	sFileFullPath += THP_WMTS_BUNDLE_EXIST_IDXFILE_POSFIX;
	int nLayerCount = 0;

	//���ҵ�һ���ļ������ؾ��
	handle=_findfirst64i32(sFileFullPath.c_str(), &fileInfo);
	if(handle==-1)
	{
		std::cerr << "Ŀ¼["<< sFileName << "] ������Ч�� WMTS Ŀ¼" << std::endl;
		return false;
	}

	do
	{
		//������ļ���".",����".."��������ж���һ���ļ�
		if( (strcmp(fileInfo.name,".")==0) || (strcmp(fileInfo.name,"..") == 0))
			continue;

		// �����ļ���
		if((fileInfo.attrib&_A_SUBDIR)==_A_SUBDIR)
		{
			// log
			continue;
		}
		else
		{
			std::string sFileFullPath = sFileName;
			sFileFullPath += fileInfo.name;

			int nLen = strlen(fileInfo.name);

			//��bdi�ļ���ȡͼ����
			std::string sLayerName  = fileInfo.name;
			sLayerName.erase(nLen-4);

			// windows·�������ִ�Сд
			if( 0 == WMTSConfig::Instance()->getFileSysType() )
				std::transform(sLayerName.begin(), sLayerName.end(), sLayerName.begin(), toupper);

			// ��ʼ��ͼ��
			if( _initLayer(sLayerName.c_str(), sFileFullPath.c_str()) )
				++nLayerCount;
		}
	}while( 0 == (done=_findnext64i32(handle,&fileInfo)) );

	_findclose(handle);

	return nLayerCount;
}

bool WinWMTSRepository::_initLayer(const char* szLayer, const char* szBdiPath)
{
	QString qsPath = WMTSConfig::Instance()->getDataDir();
	std::string sPath = (const char*)qsPath.toLocal8Bit();

	size_t nLen = strlen(szLayer);
	if(0 == nLen || nLen > THP_WMTS_MAX_LAYERLEN)
	{
		m_pLogWriter->warnLog("ͼ��������");
		return false;
	}

	struct TLayerHashTableNode* pLayerNode = NULL;
	HASH_FIND_STR(m_layers, szLayer, pLayerNode);
	if( NULL != pLayerNode)
	{
		m_pLogWriter->warnLog("ͼ���Ѵ���");
		return false;
	}

	char szPath[THP_MAX_PATH];
	memset(szPath, 0, THP_MAX_PATH);
	sprintf(szPath, "%s%s\\", sPath.c_str(), szLayer);

	if( -1 == _access(szPath, 0) )
		return false;

	WMTSLayer* pNewLayer = WMTSFactory::Instance()->createLayer();
	if( !pNewLayer->setPath(szPath) )
	{
		m_pLogWriter->warnLog("ͼ�����ò���ʧ��");
		delete pNewLayer;
		return false;
	}

	// ͼ�㻺������
	pNewLayer->setCacheMbSize( WMTSConfig::Instance()->getOneLayerMaxCacheMB() );

	// �ڴ�������
	pNewLayer->setMemStrategy( (WMTSLayer::MemStrategy)WMTSConfig::Instance()->getMemStrategy() );

	std::cout << "���ڳ�ʼ��ͼ��[" << szLayer << "]" << std::endl;
	m_pLogWriter->warnLog("���ݴ���");
	if( 0 != pNewLayer->init(szBdiPath) )
	{
		std::cout << "��ʼ��ͼ��[" << szLayer << "]ʧ��";
		m_pLogWriter->warnLog("��ʼ��ʧ��");
		delete pNewLayer;
		return false;
	}

	std::cout << "ͼ���ʼ���ɹ�" << std::endl;

	struct TLayerHashTableNode* pNewNode = (struct TLayerHashTableNode*)malloc( sizeof(struct TLayerHashTableNode) );
	memset(pNewNode->szName, 0, THP_WMTS_MAX_LAYERLEN);
	memcpy(pNewNode->szName, szLayer, nLen);
	pNewNode->pLayer = pNewLayer;
	HASH_ADD_STR(m_layers, szName, pNewNode);

	return true;
}