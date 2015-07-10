
#ifndef WMTSLEVEL_H__
#define WMTSLEVEL_H__
#include "ParamDef.h"
#include <memory>

// bundle ����
#define BUNDLE_STATUS_EXIST		0x01

// bundle �Ѿ������ڴ�
#define BUNDLE_STATUS_LOADED	0x02

namespace thp
{
class Bundle;
class Tile;

class WMTSLevel
{
public:
	WMTSLevel(int nLv);
	~WMTSLevel();

	int level();

	bool exist(const TBundleIDex& tbno);

	// ����ֵ�Ӷ��ϴ���
	std::tr1::shared_ptr<Bundle> getBundle(const TBundleIDex& tbno);

	// ����λ�� eg: ".\Layers\_alllayers\L18\"
	bool setPath(const char* pszPath);

	void setBdi(int nSize, unsigned char* szbdi);
private:
	// ��Ӧ��Ŀ¼
	char    m_szPath[THP_MAX_PATH];	

	// �ȼ� - 0-- (maxlv-1 = 19) 19
	int		m_nLvl;

	// <12 ռ1byte, >=10ռ2^(2*lv-18)byte 
	unsigned char*	m_pszBit; 
};

}// namespace thp


#endif // WMTSLEVEL_H__