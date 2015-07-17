#include "StdAfx.h"
#include <io.h>
#include "WMTSRepository.h"
#include "WMTSLayer.h"
#include "Tile.h"
#include "WMTSLevel.h"
#include <iostream>
#include <algorithm>
#include <CLogThreadMgr.h>

#pragma warning(once:4996)

using namespace thp;

WMTSRepository::WMTSRepository()
{
	// Ĭ�����ռ�� 1GB �ڴ�
	m_unMaxOccupyMemMb = THP_WMTS_DEFAULT_MEM_OCCUPY;
	memset(m_szPath, 0, THP_MAX_PATH);

	m_layers = NULL;

	_initLogWriter();
}


bool WMTSRepository::_initLogWriter()
{
	// ��ȡд��־����
	m_pLogWriter = CLogThreadMgr::instance()->getLogWriter("WMTSRepository.log");

	// ��������ڣ�������־�ļ���������־����
	if (m_pLogWriter == NULL)
	{
		CLogAppender * pLogAppender = new CLogAppender("WMTSRepository", "WMTSRepository.log", "", "DebugLog"); 

		// ��ȡд��־����
		m_pLogWriter = CLogThreadMgr::instance()->createLogWriter(pLogAppender);
	}
	return true; 
}

WMTSRepository::~WMTSRepository()
{
	struct TLayerHashTableNode* s = NULL;
	struct TLayerHashTableNode* tmp = NULL;

	HASH_ITER(hh, m_layers, s, tmp) 
	{
		std::cout << "�ͷ�ͼ��[" << s->szName << "]��Դ";

		HASH_DEL(m_layers, s);
		WMTSLayer* pLayer = s->pLayer;
		delete pLayer;
		free(s);
	}
}

bool WMTSRepository::setPath(const char* szPath)
{
	unsigned int unLen = strlen(szPath);
	if(0 == unLen || unLen > THP_MAX_PATH)
		return false;

	memcpy(m_szPath, szPath, unLen);
	return true;
}

bool WMTSRepository::init(int nMode)
{
	// ���ļ��ṹ��ʼ��
	bool bSuccess = false;
	int nLayerCount = 0;
	if( 0x0001 == (nMode&0x0001) )
		nLayerCount = _initByDir();

	if(nLayerCount > 0)
		return true;

	// ͨ�������ļ���ʼ��
	//if( 0x0002 == (nMode&0x0002) )
	//	bSuccess = _initByConfig();
	
	return bSuccess;
}

int WMTSRepository::_initByDir()
{
	// ����bdi�ļ�
	struct _finddata64i32_t fileInfo;
	long handle;
	int done;
	std::string sFileName = m_szPath; //Ҫ�������ļ���
	sFileName += "*";
	sFileName += THP_WMTS_BUNDLE_EXIST_IDXFILE_POSFIX;
	int nLayerCount = 0;

	//���ҵ�һ���ļ������ؾ��
	handle=_findfirst64i32(sFileName.c_str(), &fileInfo);
	if(handle==-1)
	{
		std::cerr << "Ŀ¼["<< m_szPath << "] ������Ч�� WMTS Ŀ¼" << std::endl;
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
			std::string sFileFullPath = m_szPath;
			sFileFullPath += fileInfo.name;

			int nLen = strlen(fileInfo.name);

			//��bdi�ļ���ȡͼ����
			std::string sLayerName  = fileInfo.name;
			sLayerName.erase(nLen-4);
			
			// ��ʼ��ͼ��
			std::transform(sLayerName.begin(), sLayerName.end(), sLayerName.begin(), toupper);
			if( _initLayer(sLayerName.c_str(), sFileFullPath.c_str()) )
				++nLayerCount;
		}

	}while( 0 == (done=_findnext64i32(handle,&fileInfo)) );

	_findclose(handle);

	return nLayerCount;
}

bool WMTSRepository::_initLayer(const char* szLayer, const char* szBdiPath)
{
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
	sprintf(szPath, "%s%s\\", m_szPath, szLayer);

	if( -1 == _access(szPath, 0) )
		return false;

	WMTSLayer* pNewLayer = new WMTSLayer;
	if( !pNewLayer->setPath(szPath) )
	{
		m_pLogWriter->warnLog("ͼ�����ò���ʧ��");
		delete pNewLayer;
		return false;
	}
	
	pNewLayer->setCacheMbSize(m_unMaxOccupyMemMb);
	pNewLayer->setGetTileStrategy( (WMTSLayer::GetTileStrategy)m_nMemStrategy );

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

unsigned int thp::WMTSRepository::getTile(const std::string& strLayer, int nLvl, int nRow, int nCol, QByteArray& arTile, int& nDetail)
{
	TLayerHashTableNode* pLyrNode = NULL;
	HASH_FIND_STR(m_layers, strLayer.c_str(), pLyrNode);
	if(NULL == pLyrNode)
		return 0;

	WMTSLayer* pLayer = pLyrNode->pLayer;
	if(NULL == pLayer)
	{	
		m_pLogWriter->errorLog("��Դ����");
		return 0;
	}

	return pLayer->getTile(nLvl, nRow, nCol, arTile, nDetail);
}

void thp::WMTSRepository::setCacheMbSize(unsigned int nMemByMB)
{
	m_unMaxOccupyMemMb = nMemByMB;
}

int thp::WMTSRepository::getCacheMbSize() const
{
	return m_unMaxOccupyMemMb;
}

int thp::WMTSRepository::loadData(const std::string& strLayer, int nLvl)
{
	TLayerHashTableNode* pLyrNode = NULL;
	HASH_FIND_STR(m_layers, strLayer.c_str(), pLyrNode);
	if(NULL == pLyrNode)
	{
		return 2;
	}

	WMTSLayer* pLayer = pLyrNode->pLayer;
	if(NULL == pLayer)
	{	
		return 2;
	}

	return pLayer->loadData(nLvl);
}

void thp::WMTSRepository::setMemStrategy(int nMemStrategy)
{
	m_nMemStrategy = nMemStrategy;
}

int thp::WMTSRepository::getMemStrategy() const
{
	return m_nMemStrategy;
}

std::string thp::WMTSRepository::getCapabilities()
{
	std::string sPath(m_szPath);
	sPath.append(WMTS_REQUEST_VL_CAPABILITIES);
	sPath.append(".xml");

	return sPath;
}


