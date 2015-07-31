#ifndef WIN_WMTSLAYER_H__
#define WIN_WMTSLAYER_H__
#include "../WMTSLayer.h"

namespace thp
{
	class WinWMTSLayer : public WMTSLayer
	{
	public:
		~WinWMTSLayer();

		virtual int init(const char* szBdiPath);

		virtual unsigned int getTile(int nLvl, int nRow, int nCol, QByteArray& arTile, int& nDetail);

	private:
		// ��ʼ��һ��ͼ��ĵȼ�, ���س�ʼ���ɹ��ĵȼ�����
		int _initLevels(const std::map<int, TLevelBundleExistStatus*>& mapBdi);

		// ��ʼ��һ���ȼ�
		int _initLevel(char* szLvlPath , int nLvl, const TLevelBundleExistStatus* pNode);

		void _clear();

	};
}



#endif // WIN_WMTSLAYER_H__
