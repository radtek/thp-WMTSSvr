#ifndef THP_WMTSPARAMDEF_H__
#define THP_WMTSPARAMDEF_H__

#define THP_WMTS_MAX_LAYERLEN	30

#define THP_MAX_PATH	512

// �����Ƭ�ȼ�
#define THP_MAX_LEVEL	22

// ��λMB
#define THP_WMTS_DEFAULT_MEM_OCCUPY		512//1024

// ��λkb
#define THP_WMTS_DEFAULT_LRU_CACHE      1048576 // 1024*1024 1GB

#define THP_WMTS_BUNDLE_EXIST_IDXFILE_POSFIX	".bdi"

//localhost:9092/WMTS?service=WMTS&request=GetTile&version=1.0.0&
//layer=img&style=default&format=tiles&TileMatrixSet=c&TileMatrix=3&TileRow=2&TileCol=2
#define WMTS_SERVICE		"SERVICE"
#define WMTS_REQUEST		"REQUEST"
#define WMTS_VERSION		"VERSION"
#define WMTS_LAYER			"LAYER"
#define WMTS_LAYER_STYLE	"STYLE"
#define WMTS_TILE_FORMAT	"FORMAT"
#define WMTS_TILEMATRIXSET	"TILEMATRIXSET"
#define WMTS_TILEMATRIX		"TILEMATRIX"
#define WMTS_TILEROW		"TILEROW"
#define WMTS_TILECOL		"TILECOL"

#define WMTS_SERVICE_VALUE				"WMTS"

#define WMTS_REQUEST_VL_GETTILE			"GetTile"
#define WMTS_REQUEST_VL_CAPABILITIES	"Capabilities"

#define WMTS_VERSION_VL					"1.0.0"

// LRU��������Դ����
#define LAYLRU_MAX_COLDBUNDLE_NUM		10 

#ifndef NULL
#define NULL 0
#endif// #ifndef NULL

// ����ĳ�ȼ���bundle������ 
struct TLevelBundleExistStatus
{
	// pbyteIndex �ֽڴ�С
	int nSize ;

	// 1 bit ��ʾһ��bundle�Ĵ�����
	// �ȼ�lv���� 2^(2-8)��2^(2-7)��bundle,��ʾ�����¶�bundle����
	// 0  ......
	// 1 
	// 2 
	// .........
	unsigned char* pbyteIndex;

	~TLevelBundleExistStatus()
	{
		delete[] pbyteIndex;
		pbyteIndex = NULL;
	}
};

namespace thp
{
	struct TBundleID
	{
		// �����ڴ�ռ�ÿ�����1char
		unsigned int unLv;

		// �ڵȼ��ڵı��
		// nLv < 8 nBundleID = 0; nLv> 8  nBundleID = [0, 2^(2*nLv-15))
		// ��Ŷ�Ӧ
		// nLv >= 8  �� 2^(nLv-8) row, ��2^(nLv-7) ��
		// ���±�ʾ��Ŷ�Ӧ��bundle��λ��
		// 0   1*(2^(nLv-8))   ...
		// 1   .
		// .   .
		// .   .
		// .   .
		unsigned int unBundleIDinLv;

		bool operator==(const TBundleID& tNoNew) const;
		bool operator<(const TBundleID& tNoNew) const;

		TBundleID();
	};

	// bundle ���
	struct TBundleIDex
	{
		TBundleID tID;

		// bundle ���ڵȼ������кſ���ͨ��unBunldeID���㣬Ϊ�˼��ټ������������������ʵ��bundle���
		// �������б�Ų���
		// bundle ���ڵȼ����к� 
		int nBundleRow;

		// bundle ���ڵȼ����к� 
		int nBundleCol;
	};


}


#endif // THP_WMTSPARAMDEF_H__
