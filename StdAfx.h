#ifndef __STDHDR_H__
#define __STDHDR_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ����QtCoreǰ�Ƚ��;��漶�𣬱�����ֺܶ��Qt�ڲ�����
#ifdef _MSC_VER
#pragma warning(push,1)
#endif

// ��������QT��ͷ�ļ�
// ��Ҫ����·��$(QTDIR)\include,$(QTDIR)\mkspecs\$(QMAKESPEC)
//#include <qt.h>
#include <QtCore>
#include <QtGui>
#include <QtXml>


#ifdef Q_OS_WIN32
#include <qt_windows.h>
#endif
#pragma warning(disable:4706)
#pragma warning(disable:4100)
#pragma warning(disable:4127)
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifndef GB
#define GB(text)	(QString::fromLocal8Bit(text))
#endif
#include "WMTSServiceBinding.nsmap"
#endif	//  __STDHDR_H__