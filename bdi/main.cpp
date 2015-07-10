#include <iostream>
#include <fstream>
#include <sstream>
#include <io.h>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include "bdiapi.h"
#include "../BundleReader.h"

using namespace std;

// ����bdi
int genBdi(std::string& sLayersDir);

// ö��ͼ���ͼƬ
// sLayer ͼ���ļ���
// sOut ����Ŀ¼
int enumTiles(const std::string& sLayer, const std::string& sOut, bool bOutTile);
int enumLevelTile(const std::string& sLevelDir, const std::string& sOut, bool bOutTile);
bool isLevel(const char* szLvlFolder);
bool isBundleFile(const char* szBundle);

int main(int argc, char **argv)
{
	if(1 == argc)
	{
		//std::string sFilePath = "D:\\test\\ArcGisParseBoudle\\ParseAGBoundle\\Layers";
		std::cout << "�ڵ�ǰĿ¼���� bdi �ļ�" << std::endl;
		//genBdi( sFilePath );
		genBdi( std::string(".") );
		return 0;
	}

	std::string sCmd = argv[1];
	if( 0 == sCmd.compare("-h") && 2 == argc)
	{
		std::cout << "ʹ��: bdi.exe [OPTION]... [FILE]..." << std::endl;
		std::cout << std::endl;
		std::cout << "����˵��:" << std::endl;
		std::cout << "  none         " << "�ڵ�ǰĿ¼����������" << std::endl;
		std::cout << "  -b (FILE)    " << "���������ļ�, (FILE)ָ��WMTSͼ����Ŀ¼��" << std::endl;
		std::cout << "  -e  [OPTION] (FILE) (FILE)" << std::endl;
		std::cout << "      /t   " << "������Ƭ��Ϣͬʱ������Ƭ�ļ�(*.png)��" << std::endl;
		std::cout << "      /n   " << "������Ƭ��Ϣ����������Ƭ�ļ���" << std::endl;
		std::cout << "           " << "��һ��(FILE)ָ��ͼ��Ŀ¼��" << std::endl;
		std::cout << "           " << "�ڶ���(FILE)ָ����Ϣ����Ŀ¼��" << std::endl;
		return 0;
	}//��ӡ������Ϣ

	if( 0 == sCmd.compare("-e") && 5 == argc )
	{
		std::string sOutPng = argv[2];
		std::string sLayerPath = argv[3];
		std::string sOutPath = argv[4];
		if( 0 == sOutPng.compare("/t") )
		{
			std::cout << "ö���ļ���[" << sLayerPath << "]����Ƭ��[" << sOutPath << "]" << std::endl;
			enumTiles(sLayerPath, sOutPath, true);
		}
		else if( 0 == sOutPng.compare("/n") )
		{
			std::cout << "������Ч������" << std::endl;
			enumTiles(sLayerPath, sOutPath, true);
		}

		std::cout << "������Ч������" << endl;
		return 0;
	}// ö��ͼƬ

	if( 0 == sCmd.compare("-b") && argc == 3)
	{
		// ���� *.bdi
		std::string sFilePath = argv[2];
		genBdi( sFilePath );
		return 0;
	}

	std::cout << "������Ч������" << endl;
	return 0;
}

int genBdi(std::string& sLayersDir)
{
	// �����ļ���Ϣ�Ľṹ��
	struct _finddata64i32_t fileInfo;

	// ���
	long handle;

	// ����nextfile�Ƿ�ɹ�
	int done;

	// Ҫ�������ļ���
	std::cout << "����ɨ��Ŀ¼:[" << sLayersDir << "]" << std::endl;

	std::string sFileFullPath = sLayersDir + "\\*.*";

	handle = _findfirst64i32(sFileFullPath.c_str(), &fileInfo);

	if(-1 == handle)
	{
		std::cout<< "Ŀ¼[" << sLayersDir <<"]����WMTS Server Ŀ¼"<< std::endl;
		return 0;
	}

	do
	{
		if( (strcmp(fileInfo.name, ".")==0) || (strcmp(fileInfo.name,"..")==0) )
			continue;

		if( (fileInfo.attrib&_A_SUBDIR) != _A_SUBDIR )
		{
			//std::string fileNameTure = sLayersDir+"\\"+fileInfo.name;
			//std::cout << "�ļ�["<< fileNameTure << "] ������Ч�� WMTS Server �ļ�" << std::endl;
			continue;
		}// �����ļ�

		// ������Ŀ¼
		{
			std::string filePathSub=sLayersDir+"\\"+fileInfo.name;

			// lv - idx
			std::map<int, TLevelBundleExistStatus*> mapBdlIdx;
			if(-1 == searchLayerFolder(filePathSub, mapBdlIdx) )
			{
				std::cout<< "Ŀ¼[" << sLayersDir <<"]����WMTS Server Ŀ¼"<< std::endl;
				continue;
			}

			// дһ��ͼ��������ļ�
			std::string sBdlIdxName = filePathSub + ".bdi";
			if( writeLayerBdlExistIdx(mapBdlIdx, sBdlIdxName) )
			{
				std::cout << "�ɹ�����ͼ��[" << sBdlIdxName << "] ����" << std::endl;
			}

			for (std::map<int, TLevelBundleExistStatus*>::iterator it = mapBdlIdx.begin(); it != mapBdlIdx.end(); ++it)
			{
				delete it->second;
			}
		}
	}while(!(done=_findnext64i32(handle,&fileInfo)));

	_findclose(handle);	

	std::cout << "���ɨ��"<< std::endl; 
}

int enumTiles(const std::string& sLayer, const std::string& sOut, bool bOutTile)
{
	// �����ļ���Ϣ�Ľṹ��
	struct _finddata64i32_t fileInfo;

	// ���
	long handle;

	// ����nextfile�Ƿ�ɹ�
	int done;

	// Ҫ�������ļ���
	std::cout << "����ɨ��Ŀ¼:[" << sLayer << "]" << std::endl;

	std::string sFileFullPath = sLayer + "\\*.*";

	handle = _findfirst64i32(sFileFullPath.c_str(), &fileInfo);

	if(-1 == handle)
	{
		std::cout<< "Ŀ¼[" << sLayer <<"]����WMTS Server Ŀ¼"<< std::endl;
		return 0;
	}

	do
	{
		if( (strcmp(fileInfo.name, ".")==0) || (strcmp(fileInfo.name,"..")==0) )
			continue;

		if( (fileInfo.attrib&_A_SUBDIR) != _A_SUBDIR )
		{
			std::string fileNameTure = sLayer+"\\"+fileInfo.name;
			std::cout << "�ļ�["<< fileNameTure << "] ������Ч�� WMTS Server �ļ�" << std::endl;
			continue;
		}// �����ļ�

		// ������Ŀ¼
		{
			// �ȼ�
			if( !isLevel(fileInfo.name) )
				continue;
			
			std::string filePathSub = sLayer+"\\"+fileInfo.name;
			std::string sLevlOutDir = sOut + "\\" + fileInfo.name;
			std::string sMkDir = "mkdir " + sOut + "\\" + fileInfo.name;
			system( sMkDir.c_str() );

			enumLevelTile(filePathSub, sLevlOutDir, bOutTile);
		}
	}while(!(done=_findnext64i32(handle,&fileInfo)));

	_findclose(handle);	

	std::cout << "���ö����Ƭ" << std::endl; 
}

bool isLevel(const char* sLvlFolder)
{
	int nLen = strlen(sLvlFolder);
	if(nLen != 3)
		return false;

	if ( 'L' != sLvlFolder[0] )
		return false;

	int nLevel = 0;
	sscanf( (sLvlFolder+1), "%d", &nLevel);
	if(0 > nLevel || nLevel > 21)
		return false;

	return true;
}

int enumLevelTile(const std::string& sLevelDir, const std::string& sOut, bool bOutTile)
{
	// �����ļ���Ϣ�Ľṹ��
	struct _finddata64i32_t fileInfo;

	// ���
	long handle;

	// ����nextfile�Ƿ�ɹ�
	int done;

	std::string sFileFullPath = sLevelDir + "\\*.*";

	// �ȼ����е�tile��Ϣ����д���ļ�
	std::string sLevelInfo = sOut + ".txt";
	std::ofstream osInfo( sLevelInfo.c_str() );

	handle = _findfirst64i32(sFileFullPath.c_str(), &fileInfo);

	if(-1 == handle)
	{
		return 0;
	}

	std::string sLayerName = "";
	int nLvl = 0;

	size_t nPos = sLevelDir.rfind('\\');
	std::string sLvl = sLevelDir.substr(nPos+2);
	sscanf(sLvl.c_str(), "%d", &nLvl);

	size_t nPos1 = sLevelDir.rfind('\\', nPos - 1);
	sLayerName = sLevelDir.substr(nPos1 + 1, nPos - nPos1 - 1 );

	do
	{
		if( (strcmp(fileInfo.name, ".")==0) || (strcmp(fileInfo.name,"..")==0) )
			continue;

		if( (fileInfo.attrib&_A_SUBDIR) == _A_SUBDIR )
		{
			//std::string fileNameTure = sLevelDir + "\\"+fileInfo.name;
			//std::cout << "�ļ�["<< fileNameTure << "] ������Ч�� WMTS Server �ļ�" << std::endl;
			continue;
		}// �����ļ�

		// ���� *.bundle �ļ�
		{
			// �ȼ�
			if( !isBundleFile(fileInfo.name) )
				continue;

			std::string sBundlePath= sLevelDir + "\\" + fileInfo.name;

			thp::BundleReader reader;
			reader.open( sBundlePath.c_str() );

			int nRow = 0;
			int nCol = 0;
			unsigned char* pTile;
			int nTileSize = 0;
			thp::BundleReader::FetchType eType = thp::BundleReader::FetchType_Success;
			while(thp::BundleReader::FetchType_Success == eType) 
			{
				eType = reader.nextTile(nRow, nCol, pTile, nTileSize);
				if( thp::BundleReader::FetchType_Success != eType )
					break;

				osInfo << sLayerName << "," << nLvl << "," << nRow << "," << nCol << std::endl;

				if( bOutTile )
				{
					std::stringstream ss;
					ss << sOut << "\\R" << nRow << "C" << nCol << ".png";
					std::string sTile = ss.str();
					FILE* fpTile = fopen( sTile.c_str(), "wb");
					if(!fpTile)
						continue;

					fwrite(pTile, 1, nTileSize, fpTile);

					fclose(fpTile);
				}// ���ͼƬ
			
				free(pTile);
				pTile = NULL;
				nTileSize = 0;
			}
		}
	}while(!(done=_findnext64i32(handle,&fileInfo)));

	_findclose(handle);	

	std::cout << "��ɵȼ� " << nLvl << " ��Ƭö��" << std::endl; 
}

bool isBundleFile(const char* szBundle)
{
	int nLen = strlen(szBundle);
	if(nLen != 17)
		return false;

	const char* c = szBundle+11;
	int nLevel = 0;
	if( 0 != strcmp(szBundle+11, "bundle") )
		return false;

	return true;
}

