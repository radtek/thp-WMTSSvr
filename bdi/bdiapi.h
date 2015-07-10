#ifndef GENAPI_H__
#define GENAPI_H__
#include <map>
#include <string>
#include "../ParamDef.h"


// �����ļ��̶���С
// �����ļ���ʽ
// 16 byte bom δʹ�� 
// ���� 0 - 21 ��22 ��

// 0     idxcontents
// 1	 idxcontents
// 2  
// ..

// idx contents = calcBunldeExistBit, lv<12= 1byte,lv>=12 = 2^(2*lv-23)byte
// [int] �ȼ�, [...] ����״̬����
//
// 
#define FILE_POSTFIX ".bundlx"
//#define FILE_NAME_LEN  17
#define MAX_LEVLE	22

//struct TNode
//{
//	int nSize ;				// ��С
//	unsigned char* pbyteIndex;	// ����
//
//	~TNode()
//	{
//		delete[] pbyteIndex;
//		pbyteIndex = NULL;
//	}
//};

// �ȼ� tile�к� tile�к�
int calcBundleNo(int nLvl, int nRow, int nCol);


// ��ǵĵ����ֽ�,���λ�� 0 - 7
void tag(unsigned char* pOf, int nTagIdx);

// ���Ƿ��б��
bool taged(unsigned char* pOf, int nTagIdx);

// ����ȼ�����ռ�õĴ�С
int calcBunldeExistStatusOccupyByte(int nLvl);

//IN:�ļ����ڵ�·��,�磺f:\example
//IN:�洢���ļ���
int searchLayerFolder (std::string filePath, std::map<int,TLevelBundleExistStatus*>& pBlEstIdx);

//IN:�ļ����ڵ�·��,�磺f:\example\L03
//��Σ�ֻ�в��Ϊ0ʱ��������������ļ���Ϣ����ʾ�ʹ洢
//IN:�洢���ļ���
int searchLevelFolder (std::string sLayerLevelPath, int nLvl, unsigned char* pBundleExistIdx);

// 16 byte bom,int[�ȼ����],int[������С],��������
// 1 ����,0 ������
bool writeLayerBdlExistIdx(const std::map<int, TLevelBundleExistStatus*>& idxMap, const std::string& sPath);

bool readLayerDbi(const std::string& sPath, std::map<int, TLevelBundleExistStatus*>& idxMap);

bool isExist(unsigned char* pByteDbi, unsigned int unBundleIndex);


#endif // genapi_h__
