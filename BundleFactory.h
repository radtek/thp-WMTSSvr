/*!
 * \file BundleFactory.h
 * \author Xuebingbing
 * \brief Bundle ������
 *
 * TODO: long description
 * \date ���� 2015
 * \copyright	Copyright (c) 2015, �������ƺ�ͨ�����Զ����Ƽ����޹�˾\n
	All rights reserved.
 */


#ifndef THP_BUNDLEFACTORY_H__
#define THP_BUNDLEFACTORY_H__
#include <map>
#include "ParamDef.h"

namespace thp
{
	class Bundle;
	class WMTSLevel;

	// ��Ҫʹ�ø����ָ��ɾ���������
	class BundleFactory
	{
	public:
		~BundleFactory();

		// eg: .\Layers\_alllayers\L18\  ����β'\'
		bool setPath(const char* szPath);

		// 0 - success 1 - no dir
		// bdi �ļ�λ��
		int init(const char* szBdiPath);

		Bundle* createBundle(const TBundleIDex& key);

	private:
		// ��ʼ��һ��ͼ��ĵȼ�, ���س�ʼ���ɹ��ĵȼ�����
		int _initLevels(const std::map<int, TLevelBundleExistStatus*>& mapBdi);

		// ��ʼ��һ���ȼ�
		int _initLevel(char* szLvlPath , int nLvl, const TLevelBundleExistStatus* pNode);

		void _clear();

	protected:
		WMTSLevel* m_pLvl[THP_MAX_LEVEL];
		char m_szPath[THP_MAX_PATH];
	};
}

#endif // BundleFactory_h__
