#include <iostream>
#include <fstream>
#include <sstream>
#include <io.h>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include "bdiapi.h"
#include "win/WinBundleReader.h"
#include "WMTSConfig.h"
#include <QString>
#include "WMTSFactory.h"
#include "curl/curl.h"
#include "hdfs/HdfsUrl.h"
#include <json/json.h>
#include <map>
#include <QFile>
#include <QByteArray>
#include <QString>

using namespace std;
using namespace thp;

#pragma comment(lib, "libcurl.lib")
#pragma comment(lib, "lib_json.lib")


//����bdi
int createBdi(std::string sOutDir);

// ����bdi ���ش���ͼ��������
int createBdiOnWinSys(std::string& sLayersDir);

// ���������hdfs�Ƿ�����webhdfs
bool testWebhdfs(const std::string& sUrl);
// ���ش���ͼ��������
int createBdiOnWebhdfs(std::string& sLayersUrl, const std::string& sLocalDir);
// ���������ݵ�level��
int createBdiOnWebhdfsLayer(std::string& sLayerUrl, std::map<int,TLevelBundleExistStatus*>& pBlEstIdx);
// ����Bundle����-ʹ��bundlxɨ�裬�����ļ��������ļ�����ͬ�´��ڷ����������
int createBdiOnWebhdfsLevel(std::string& sLevelUrl, int nLvl, unsigned char* pBundleExistIdx);

//////////////ö��ָ��Bundle��Tile
// ����Tile����
int enumBundleTiles(const std::string& sBundle, const std::string& sOutDir);

int main(int argc, char **argv)
{
	if( 1 == argc )
		std::cout << "��ϸʹ����Ϣ��ʹ�� -h ����" << std::endl << std::endl;
	
	if(argc > 1)
	{
		if( 0 == strcmp(argv[1], "-h") )
		{
			std::cout << "ʹ��: bdi.exe [OPTION] [FILE]..." << std::endl;
			std::cout << std::endl;
			std::cout << "����˵��:" << std::endl;
			std::cout << " -b (FILE) : " << "���������ļ�(./data/wmts_conf.ini)��������" << std::endl
					  << "    FILE : ��������ļ�λ��(����bundle�ļ��洢��hdfs����Ч)" << std::endl << std::endl;

			std::cout << " -e (FILE) (FILE) : ö��ָ��bundle�ļ��е���Ƭ��ָ��Ŀ¼,ֻ�Ա����ļ�ϵͳ��Ч" << std::endl;
			std::cout << "    (FILE) : " << "��һ������,ָ��bundle�ļ�λ��" << std::endl;
			std::cout << "    (FILE) : " << "�ڶ�������,ָ����Ƭ�ļ����Ŀ¼" << std::endl;

#ifdef _DEBUG
			getchar();
#endif

			return 0;
		}

		// ����bdi�ļ�
		if( 0 == strcmp(argv[1], "-b") )
		{
			std::string sOutDir(".");
			if(argc >= 3)
				sOutDir = argv[2];

			return createBdi(sOutDir);
		}

		// ö��ָ��bundle����Ƭ
		if( 0 == strcmp(argv[1], "-e") )
		{
			if( argc < 4)
			{
				std::cout << "ȱ�ٲ���"  << std::endl;
				return 0;
			}
			std::string sBundle = argv[2];
			//std::string sBundle = "E:\\WMTS\\nwws\\L17\\Rd400C1a000.bundle";

			std::string sOutDir = argv[3];
			//std::string sOutDir = "./L05/png/";

			return enumBundleTiles(sBundle, sOutDir);
		}
	}

	return 0;
}

int createBdi(std::string sOutDir)
{
	// ͨ�������в�����ȡ������Ϣ
	QString sConfig( ".\\data\\wmts_conf.ini" );
	WMTSConfig::Instance()->initConfigData( sConfig );

	thp::FST eFsType = (thp::FST)WMTSConfig::Instance()->getFileSysType();

	int nLayers = 0;
	switch(eFsType)
	{
	case WIN32_FILE_SYS:
		{
			//std::string sFilePath = "D:\\test\\ArcGisParseBoudle\\ParseAGBoundle\\Layers";
			std::cout << "�ļ�ϵͳ: Windows file system" << std::endl;
			QString qsDataDir = WMTSConfig::Instance()->getDataDir();
			std::string sDataDir = (const char*)qsDataDir.toLocal8Bit();
			nLayers = createBdiOnWinSys(sDataDir);
		}
		break;

		// ����webhdfs�ϴ������ļ���Ŀ¼Ȩ�����������ڱ���Ŀ¼����
	case HDFS_SYS:
		{
			std::cout << "�ļ�ϵͳ: HDFS" << std::endl;
			QString qsHdfsServer = WMTSConfig::Instance()->getHdfsServer();
			int nHdfsPort = WMTSConfig::Instance()->getHdfsNameNodeWebPort();
			QString qsDataDir = WMTSConfig::Instance()->getDataDir();

			QString qsUrl = QString("http://%0:%1/webhdfs/v1%2").arg(qsHdfsServer).arg(nHdfsPort).arg(qsDataDir);

			std::string sUrl = (const char*)qsUrl.toLocal8Bit();

			std::cout << "�������Ŀ¼: " << sOutDir << std::endl;

			nLayers = createBdiOnWebhdfs(sUrl, sOutDir);
		}

		break;

	case UNIX_FILE_SYS:
		std::cout << "�ļ�ϵͳ: Unix file system" << std::endl 
			<< "��֧��" << std::endl;
		break;

	default:
		{
			std::cout << "δ֪�ļ�ϵͳ:���޸������ļ�" << std::endl;
		}
		break;
	}

	return nLayers;
}

int createBdiOnWinSys(std::string& sLayersDir)
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

	int nCount = 0;
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
			if(-1 == searchWinSysLayerFolder(filePathSub, mapBdlIdx) )
			{
				std::cout<< "Ŀ¼[" << filePathSub <<"]����WMTS Server Ŀ¼"<< std::endl;
				continue;
			}

			// дһ��ͼ��������ļ�
			std::string sBdlIdxName = filePathSub + ".bdi";
			if( write_bdi(mapBdlIdx, sBdlIdxName) )
			{
				std::cout << "�ɹ�����ͼ��[" << sBdlIdxName << "] ����" << std::endl;
			}

			for (std::map<int, TLevelBundleExistStatus*>::iterator it = mapBdlIdx.begin(); it != mapBdlIdx.end(); ++it)
			{
				delete it->second;
			}

			++nCount;
		}
	}while(!(done=_findnext64i32(handle,&fileInfo)));

	_findclose(handle);	

	std::cout << "���ɨ��"<< std::endl; 
	return nCount;
}

bool testWebhdfs(const std::string& sUrl)
{
	CURL* curlHandle = curl_easy_init();
	if(NULL == curlHandle)
		return false;

	std::string sTotalUrl = sUrl + "?op=LISTSTATUS";

	// ��ʼ�����Ӳ���
	::_initCurl(curlHandle);

	QByteArray arDirectoryStatus;

	// ��������д�����
	CURLcode res = curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &arDirectoryStatus);
	if(CURLE_OK != res)
		return false;

	res = curl_easy_setopt(curlHandle, CURLOPT_URL, sTotalUrl.c_str() );
	if(CURLE_OK != res)
		return false;

	res = curl_easy_perform(curlHandle);
	if(CURLE_OK != res)
		return false;

	curl_easy_cleanup(curlHandle);
	return true;
}

int createBdiOnWebhdfs(std::string& sLayersUrl, const std::string& sLocalDir)
{
	if( !testWebhdfs(sLayersUrl) )
	{
		std::cout << "����hdfsʧ�ܣ�����ԭ��: 1 ��������; 2 hdfsδ����webhdfs" << std::endl;
		return 0;
	}

	int nLayerCount = 0;
	CURL* curlHandle = curl_easy_init();

	QByteArray arDirectoryStatus;

	// ��ʼ�����Ӳ���
	::_initCurl(curlHandle);

	// ��������д�����
	CURLcode res = curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &arDirectoryStatus);

	// ��������url
	std::string sUrl = sLayersUrl + "?op=LISTSTATUS";

	res = curl_easy_setopt(curlHandle, CURLOPT_URL, sUrl.c_str() );

	res = curl_easy_perform(curlHandle);
	curl_easy_cleanup(curlHandle);
	if( CURLE_OK != res )
	{
		std::cout << "url:" << sUrl << "����ʧ��" << std::endl;
		return nLayerCount;
	}

	QString qs = arDirectoryStatus;

	std::string sJson = (const char*)( qs.toLocal8Bit() );

	// ����json�� 
	Json::Reader reader;  
	Json::Value root;  
	if ( reader.parse(sJson, root) )  // reader��Json�ַ���������root��root������Json��������Ԫ��  
	{  
		Json::Value vlFileStatuses = root["FileStatuses"];
		Json::Value vlFileStatus = vlFileStatuses["FileStatus"];
		Json::Value::UInt nFileCount = vlFileStatus.size();
		for (Json::Value::UInt i = 0; i < nFileCount; ++i)
		{
			Json::Value vlTemp = vlFileStatus[i];
			std::string sType = vlTemp["type"].asString();

			// ��������Ŀ¼����
			if( 0 != sType.compare("DIRECTORY") )
				continue;

			std::string sDirName = vlTemp["pathSuffix"].asString();

			std::string sLayerUrl = sLayersUrl + sDirName + "/";
		
			// lv - idx
			std::map<int, TLevelBundleExistStatus*> mapBdlIdx;
			if( createBdiOnWebhdfsLayer(sLayerUrl, mapBdlIdx) )
				++nLayerCount;

			// дһ��ͼ��������ļ�
			std::string sBdlIdxName = sLocalDir + "/" + sDirName + ".bdi";

			if( write_bdi(mapBdlIdx, sBdlIdxName) )
			{
				// �ϴ���hdfs
				std::cout << "�ɹ�����ͼ��[" << sDirName << ".bdi] ����" << std::endl;
			}

			for (std::map<int, TLevelBundleExistStatus*>::iterator it = mapBdlIdx.begin(); it != mapBdlIdx.end(); ++it)
			{
				delete it->second;
			}
		}
	}  

	return nLayerCount;
}

// ���������ݵ�lv ����
int createBdiOnWebhdfsLayer(std::string& sLayerUrl, std::map<int,TLevelBundleExistStatus*>& pBlEstIdx)
{
	CURL* curlHandle = curl_easy_init();

	QByteArray arDirectoryStatus;

	// ��ʼ�����Ӳ���
	::_initCurl(curlHandle);

	// ��������д�����
	CURLcode res = curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &arDirectoryStatus);

	// ��������url
	std::string sUrl = sLayerUrl + "?op=LISTSTATUS";

	res = curl_easy_setopt(curlHandle, CURLOPT_URL, sUrl.c_str() );

	res = curl_easy_perform(curlHandle);
	curl_easy_cleanup(curlHandle);
	if( CURLE_OK != res )
	{
		std::cout << "url:" << sUrl << "����ʧ��" << std::endl;
		return 0;
	}

	QString qs = arDirectoryStatus;

	std::string sJson = (const char*)( qs.toLocal8Bit() );

	// ����json�� 
	Json::Reader reader;  
	Json::Value root;  
	if ( reader.parse(sJson, root) )  // reader��Json�ַ���������root��root������Json��������Ԫ��  
	{  
		Json::Value vlFileStatuses = root["FileStatuses"];
		Json::Value vlFileStatus = vlFileStatuses["FileStatus"];
		Json::Value::UInt nFileCount = vlFileStatus.size();
		for (Json::Value::UInt i = 0; i < nFileCount; ++i)
		{
			Json::Value vlTemp = vlFileStatus[i];
			std::string sType = vlTemp["type"].asString();

			// ��������Ŀ¼����
			if( 0 != sType.compare("DIRECTORY") )
				continue;

			std::string sName = vlTemp["pathSuffix"].asString();

			if( 3 != sName.size() )
			{
				// log
				continue;
			}

			if( 'L' != sName[0] )
			{
				// log
				continue;
			}

			char szNum[4];
			memset(szNum, 0, 4);
			memcpy(szNum, sName.c_str()+1, 3 );
			szNum[3] = '\0';
			int nLv = 0;
			if( -1 == sscanf(szNum, "%d", &nLv) )
			{
				// 
				continue;
			}

			TLevelBundleExistStatus* pNode = new TLevelBundleExistStatus;
			pNode->nSize = calcBunldeExistStatusOccupyByte(nLv);
			pNode->pbyteIndex = new unsigned char[pNode->nSize];
			memset(pNode->pbyteIndex, 0, pNode->nSize);

			std::string sLevelUrl = sLayerUrl + sName + "/";
			if( createBdiOnWebhdfsLevel(sLevelUrl, nLv, pNode->pbyteIndex) )
			{
				pBlEstIdx.insert( std::make_pair(nLv, pNode) );
			}
		}
	}  

	return (int)pBlEstIdx.size();
}

int createBdiOnWebhdfsLevel(std::string& sLevelUrl, int nLvl, unsigned char* pBundleExistIdx)
{
	int nBundleCount = 0;
	CURL* curlHandle = curl_easy_init();

	QByteArray arDirectoryStatus;

	// ��ʼ�����Ӳ���
	::_initCurl(curlHandle);

	// ��������д�����
	CURLcode res = curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &arDirectoryStatus);

	// ��������url
	std::string sUrl = sLevelUrl + "?op=LISTSTATUS";

	res = curl_easy_setopt(curlHandle, CURLOPT_URL, sUrl.c_str() );

	res = curl_easy_perform(curlHandle);
	curl_easy_cleanup(curlHandle);
	if( CURLE_OK != res )
	{
		std::cout << "url:" << sUrl << "����ʧ��" << std::endl;
		return nBundleCount;
	}

	// ����json�� 
	QString qs = arDirectoryStatus;
	std::string sJson = (const char*)( qs.toLocal8Bit() );

	Json::Reader reader;  
	Json::Value root;
	// reader��Json�ַ���������root��root������Json��������Ԫ�� 
	if ( reader.parse(sJson, root) )   
	{  
		Json::Value vlFileStatuses = root["FileStatuses"];
		Json::Value vlFileStatus = vlFileStatuses["FileStatus"];
		Json::Value::UInt nFileCount = vlFileStatus.size();

		for (Json::Value::UInt i = 0; i < nFileCount; ++i)
		{
			Json::Value vlTemp = vlFileStatus[i];
			std::string sType = vlTemp["type"].asString();

			// ���������ļ�����
			if( 0 != sType.compare("FILE") )
				continue;

			std::string sName = vlTemp["pathSuffix"].asString();

			// �������������ļ����ļ�
			if ( std::string::npos == sName.find(FILE_POSTFIX) )
				continue;

			size_t pos0 = sName.find('R');
			size_t pos1 = sName.find('C');
			size_t pos2 = sName.find('.');

			std::string sNum = sName.substr(pos0+1, pos1 - pos0 - 1);
			unsigned int nRow = 0;
			if( -1 == sscanf(sNum.c_str(), "%x", &nRow) )
				continue;

			unsigned int nCol = 0;
			sNum = sName.substr(pos1+1, pos2 - pos1 -1);
			if( -1 == sscanf(sNum.c_str(), "%x", &nCol) )
				continue;

			// ����bundle���
			unsigned int nBundleIndex = calcBundleNo(nLvl, nRow, nCol);

			// �õ��ֽڵ�ƫ��
			unsigned int nByteOffset = nBundleIndex >> 3;  // <==> nBundleIndex / 3
			unsigned char* pOf = pBundleExistIdx + nByteOffset;

			// ���λ�� 0-7
			unsigned int nTagIdx = nBundleIndex - (nByteOffset << 3);
			tag(pOf, nTagIdx);
			++nBundleCount;
		}
	}

	return nBundleCount;
}

int enumBundleTiles(const std::string& sBundle, const std::string& sOutDir)
{
	int nTileCount = 0;
	thp::WinBundleReader reader;
	reader.open( sBundle.c_str() );

	int nRow = 0;
	int nCol = 0;
	unsigned char* pTile;
	int nTileSize = 0;
	thp::BundleReader::FetchType eType = thp::BundleReader::FetchType_Success;

	size_t nPos = sBundle.rfind('\\');
	if(nPos == std::string::npos)
		nPos = sBundle.rfind('/');

	size_t nPos2 = sBundle.rfind('.');
	std::string sBundleName = sBundle.substr(nPos, (nPos2 - nPos));

	std::stringstream ss;
	ss << sOutDir << sBundleName << ".txt";
	std::string sTxt = ss.str();
	std::ofstream of(sTxt.c_str());

	// ������ʵ���к�
	//nPos = sBundleName.find('R');
	//nPos2 = sBundleName.find('C');
	//int nBeginRow = 0;
	//int nBeginCol = 0;

	//std::string sBeginRow = sBundleName.substr(nPos+1, nPos2 - nPos -1);
	//std::string sBeginCol = sBundleName.substr(nPos2+1, sBundleName.size() - nPos2);

	//sscanf(sBeginRow.c_str(), "%x", &nBeginRow);
	//sscanf(sBeginCol.c_str(), "%x", &nBeginCol);

	while(thp::BundleReader::FetchType_Success == eType) 
	{
		eType = reader.nextTile(nRow, nCol, pTile, nTileSize);
		if( thp::BundleReader::FetchType_Success != eType )
			break;

		QString sFile = QString("%0R%1C%2.png").arg( QString::fromLocal8Bit(sOutDir.c_str())).arg(nRow).arg(nCol);

		of << nRow << "," << nCol << std::endl;

		std::string s = sFile.toLocal8Bit();

		QFile file( sFile );
		file.open(QIODevice::WriteOnly);

		QByteArray bydata;
		bydata.append((const char*)pTile, nTileSize);
		file.write(bydata);
		file.close();
		++nTileCount;
	}

	std::cout << "���Ŀ¼:" << sOutDir << std::endl;
	std::cout << "һ��ö��:" << nTileCount << " ��Ƭ" << std::endl;
	return nTileCount;
}
