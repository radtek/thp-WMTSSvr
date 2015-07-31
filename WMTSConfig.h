/**
 * @file     
 * @brief    ���������ļ����صĶ�����
             

 * @author   
 * @date     
 */
#ifndef LOADCONFIGDATA_H
#define LOADCONFIGDATA_H

#include "QString"

class WMTSConfig
{

public:

	static WMTSConfig * Instance();

	static void release();

	/// ��ʼ�������ļ���Ϣ
	/// @param fileName[in] �����ļ�����
	/// @rreturn ��ʼ���ɹ����� true ;ʧ�ܷ��� false
	bool initConfigData(const QString& fileName);

	/// ��ȡ�����˿ں�
	int getPort() const;

	/// ��ȡ����߳���
	int getMaxThreadCount() const;

	// һ��ͼ��ʹ�õ��ڴ�����
	int getOneLayerMaxCacheMB() const;

	int getMemStrategy() const;

	// �ļ�ϵͳ���� 0:windows�ļ�ϵͳ 1:��unix�ļ�ϵͳ 2: HDFS
	// eg 0: #ServerDir=D:\\thp\\data\\wmts\\
	// eg 1: #ServerDir=/thp/data/wmts/
	// eg 2: #ServerDir=/thp/data/wmts/
	int getFileSysType() const;

	//0:webhdfs,��������չ�����ӷ�ʽ��libhdfs��libhdfs3
	int getHdfsClient() const;

	QString getHdfsServer() const;

	int getHdfsNameNodeWebPort() const;

	/// ��ȡ����Ŀ¼ eg: "D:\\thp\\data\\wmts\\",���÷���Ŀ¼ʱע������������"\\"
	QString getDataDir() const;

	QString getPreLoadLayerName() const;
	int getBeginLevel() const;
	int getEndLevel() const;

private:
	WMTSConfig();
	~WMTSConfig();
private:
	///< �̸߳���
	int			m_nThreadCount;

	///< �˿ں�
	int			m_nPort;

	///< ���ݲֿ�Ŀ¼ 
	QString		m_strRepositoryDir;

	///< ÿ��ͼ��ʹ�õĻ�������
	int			m_nMaxOneLayerCacheMb;

	///< ��Դ����
	int			m_nMemStrategy;

	///< ����ָ��
	static WMTSConfig     *s_pLoadConfigData;

	///< �����ü���ͼ����������ͼ������ͼ����ʼ�ȼ��������ȼ�
	QString		m_strPreLoadLayerName;
	int			m_nBeginLv;
	int			m_nEndLv;	

	// �ļ�ϵͳ
	int			m_nFileSysType;

	// ����hdfsƽ̨
	int			m_nHdfsClient;

	// hdfs ��ַ
	QString		m_strHdfsServer;

	// hdfs NameNode�˿ں�
	int			m_nHdfsNameNodePort;
};

#endif