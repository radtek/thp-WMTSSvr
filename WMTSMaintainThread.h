/*!
 * \file WMTSMaintainThread.h
 * \author Xuebingbing
 * \brief WMTS ά���߳� û����Сʱ�Ͳ鿴ÿ��ͼ���м�¼��bundle����,����¼��bundle�����ڵ���1000��ʱ
 ɾ����ǰ�����ڴ��еļ�¼
 *
 * TODO: long description
 * \date ���� 2015
 * \copyright	Copyright (c) 2015, �������ƺ�ͨ�����Զ����Ƽ����޹�˾\n
	All rights reserved.
 */

#ifndef THP_WMTSMAINTAINTHREAD_H__
#define THP_WMTSMAINTAINTHREAD_H__

#include <QThread>

class WMTSMaintainThread : public QThread
{
	//Q_OBJECT
public:
	explicit WMTSMaintainThread(QObject *parent = 0);

	void run();

	void stop();

	void maintain();

signals:

	public slots:

	bool m_bStop;
};

#endif // THP_WMTSMAINTAINTHREAD_H__
