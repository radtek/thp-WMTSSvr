/**
 * @file     
 * @brief    ���������ļ����صĶ�����
             

 * @author   
 * @date     
 */
#ifndef LOADCONFIGDATA_H
#define LOADCONFIGDATA_H

#include "QString"

class LoadConfigData
{

public:

	static LoadConfigData * getInstance();

	static void release();

	/// ��ʼ�������ļ���Ϣ
	/// @param fileName[in] �����ļ�����
	/// @rreturn ��ʼ���ɹ����� true ;ʧ�ܷ��� false
	bool initConfigData(const QString& fileName);

	/// ��ȡ�����˿ں�
	int getPort() const;

	/// ��ȡ����߳���
	int getMaxThreadCount() const;

	int getOneLayerMaxCacheMB() const;

	int getMemStrategy() const;

	/// ��ȡ����Ŀ¼ eg: "D:\\thp\\data\\wmts\\",���÷���Ŀ¼ʱע������������"\\"
	QString getServerDir() const;

	QString getPreLoadLayerName() const;
	int getBeginLevel() const;
	int getEndLevel() const;

private:
	LoadConfigData();
	~LoadConfigData();
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
	static LoadConfigData     *s_pLoadConfigData;

	///< �����ü���ͼ����������ͼ������ͼ����ʼ�ȼ��������ȼ�
	QString		m_strPreLoadLayerName;
	int			m_nBeginLv;
	int			m_nEndLv;	
};

#endif