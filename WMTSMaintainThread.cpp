#include "WMTSMaintainThread.h"
#include "WMTSRepository.h"
#include "WMTSLayer.h"
#include <iostream>
#include <ctime>

using namespace thp;

extern WMTSRepository* g_pWMTSDataCache;

// ����ʱ���� second
#define WM_WAKEUP_SPAN 7200 // 2h

// ǿ�ƽ�����Դ�����ʱ��� 1���е� [0-24),����ʱ���賿1�㵽�賿4�����һ��ǿ�Ƶ���Դ����
#define FORCE_MAINTAIN_BEGIN 1
#define FORCE_MAINTAIN_END	 4


WMTSMaintainThread::WMTSMaintainThread(QObject *parent /*= 0*/) :
	QThread(parent)
{
	m_bStop = false;
}

void WMTSMaintainThread::run()
{
	if(m_bStop)
		return ;

	for(;;)
	{
		if ( m_bStop )
			break;

		if(NULL == g_pWMTSDataCache)
			QThread::sleep(WM_WAKEUP_SPAN);

		time_t ttNow = time(NULL);
		tm* tNow = localtime(&ttNow);

		int nH = tNow->tm_hour;
		
		// �賿1�㵽�賿4���ڽ���һ��ǿ�Ƶ���������
		if(nH > FORCE_MAINTAIN_BEGIN && nH < FORCE_MAINTAIN_END)
			maintain(true);
		else
			maintain(false);

		QThread::sleep(WM_WAKEUP_SPAN);
	}
}

void WMTSMaintainThread::stop()
{
	m_bStop = true;
}

void WMTSMaintainThread::maintain(bool bForce)
{
	int nLayerCount = g_pWMTSDataCache->getLayerCount();
	for (int i = 0; i < nLayerCount; ++i)
	{
		WMTSLayer* pLyr = g_pWMTSDataCache->getLayer(i);
		pLyr->maintain(bForce);
	}
}
