#include "WMTSLevel.h"
#include <math.h>
#include <memory.h>
#include <io.h>
#include "Bundle.h"
#include "BundleReader.h"
#include "bdi/bdiapi.h"

#pragma warning(once:4996)

using namespace thp;

WMTSLevel::WMTSLevel(int nLv)
{
	m_nLvl = nLv;

	// �������쳣
	//m_nBundleRowCount = 1;
	//m_nBundleColCount = 1;
	//if( nLv > 7)
	//{
	//	// 8��֮ǰ�ĵȼ�ֻ��һ�� bundle(һ��һ��)
	//	m_nBundleRowCount = 1 << (nLv - 8); 
	//	m_nBundleColCount = 1 << (nLv - 7);;
	//}

	memset(m_szPath, 0, THP_MAX_PATH);

	m_pszBit = NULL;
}
//
//bool WMTSLevel::_initLogWriter()
//{
//	// ��ȡд��־����
//	m_pLogWriter = CLogThreadMgr::instance()->getLogWriter("WMTSLevel.log");
//
//	// ��������ڣ�������־�ļ���������־����
//	if (m_pLogWriter == NULL)
//	{
//		CLogAppender * pLogAppender = new CLogAppender("WMTSLevel", "WMTSLevel.log", "", "General"); 
//
//		// ��ȡд��־����
//		m_pLogWriter = CLogThreadMgr::instance()->createLogWriter(pLogAppender);
//	}
//	return true; 
//}

WMTSLevel::~WMTSLevel()
{
	if(NULL != m_pszBit)
	{
		delete[] m_pszBit;
		m_pszBit = NULL;
	}
}

bool WMTSLevel::setPath(const char* szPath)
{
	unsigned int unLen = strlen(szPath);
	if(0 == unLen || unLen > THP_MAX_PATH)
		return false;

	memcpy(m_szPath, szPath, unLen);
	return true;
}

bool thp::WMTSLevel::exist(const TBundleIDex& tbno)
{
	return ::isExist(m_pszBit, tbno.tID.unBundleIDinLv);
}

std::tr1::shared_ptr<Bundle> thp::WMTSLevel::getBundle(const TBundleIDex& tbno)
{
	// �����������bundle�ļ��Ƿ����
	if( !::isExist(m_pszBit, tbno.tID.unBundleIDinLv) )
	{
		std::tr1::shared_ptr<Bundle> sp;
		return sp;
	}

	// �����µ� bundle ��ȡ bundle
	char szBundleFile[THP_MAX_PATH];
	memset(szBundleFile, 0, THP_MAX_PATH);

	int nXR = 128 * tbno.nBundleRow;
	int nXC = 128 * tbno.nBundleCol;

	sprintf(szBundleFile, "%sR%04xC%04x.bundle", m_szPath, nXR, nXC);

	std::tr1::shared_ptr<Bundle> sp(new Bundle(tbno));
	sp->open( szBundleFile );

	return sp;
}

void WMTSLevel::setBdi(int nSize, unsigned char* szbdi)
{
	if( NULL != m_pszBit)
	{
		delete[] m_pszBit;
		m_pszBit = NULL;
	}

	m_pszBit = new unsigned char[nSize];
	memcpy(m_pszBit, szbdi, nSize);
}



