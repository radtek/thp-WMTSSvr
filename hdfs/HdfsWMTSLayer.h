#ifndef HDFS_WMTSLAYER_H__
#define HDFS_WMTSLAYER_H__
#include "../WMTSLayer.h"

namespace thp
{
	class HdfsWMTSLayer : public WMTSLayer
	{
	public:
		~HdfsWMTSLayer();

		virtual int init(const char* szBdiPath);

		virtual unsigned int getTile(int nLvl, int nRow, int nCol, QByteArray& arTile, int& nDetail);

	private:
		bool readBdiWithWebhdfs(const std::string& sPath, std::map<int, TLevelBundleExistStatus*>& idxMap);

		//// ��ʼ��һ��ͼ��ĵȼ�, ���س�ʼ���ɹ��ĵȼ�����
		int _initLevels(const char* szLayerUrl, const std::map<int, TLevelBundleExistStatus*>& mapBdi);

		//// ��ʼ��һ���ȼ�
		int _initLevel(const char* szLvlPath , int nLvl, const TLevelBundleExistStatus* pNode);

		void _clear();

	};
}



#endif // HDFS_WMTSLAYER_H__
