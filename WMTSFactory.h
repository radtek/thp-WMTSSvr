/*!
 * \file WMTSFactory.h
 * \author Xuebingbing
 * \brief ������
 *
 * TODO: long description
 * \date ���� 2015
 * \copyright	Copyright (c) 2015, �������ƺ�ͨ�����Զ����Ƽ����޹�˾\n
	All rights reserved.
 */
#ifndef THP_WMTSFACTORY_H_
#define THP_WMTSFACTORY_H_
#include "ParamDef.h"


namespace thp
{
	class WMTSRepository;
	class WMTSLayer;
	class WMTSLevel;
	class Bundle;
	class BundleReader;

	class WMTSFactory
	{
	public:

		static WMTSFactory* Instance();
		static void DeInstance();

		void setFileSys(FST efs);
		FST getFileSys();

		WMTSRepository* createRepository();
		WMTSLayer*		createLayer();
		WMTSLevel*		createLevel(int nLv);
		Bundle*			createBundle(const thp::TBundleIDex& tNoEx);
		BundleReader*	createBundleReader();

	private:
		WMTSFactory();
		~WMTSFactory();

		static WMTSFactory* _ins;
		FST m_eType;
	};
}

#endif//THP_WMTSFACTORY_H_