#ifndef THP_HDFS_BUNDLEREADER_H__
#define THP_HDFS_BUNDLEREADER_H__
#include "../BundleReader.h"

namespace thp
{
	class HdfsBundleReader : public BundleReader
	{
	public:
		HdfsBundleReader();
		~HdfsBundleReader();

		// �����ļ���ַ eg��.\\L09\\R0080C0180.bundle
		bool open(const char* szFile);

		bool getTileFromFile(int nTileInBundleIndex, unsigned char*& pByteTile, unsigned int& nSize);

		virtual unsigned int getMaxByte();

		// ����ȫbundle����
		virtual int readAll(char*& pBundle);
	private:
		bool _loadBundlx(const char* szFile);
	};
}


#endif // THP_HDFS_BUNDLEREADER_H__
