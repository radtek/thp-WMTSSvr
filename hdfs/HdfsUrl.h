#ifndef HDFS_URL_H_
#define HDFS_URL_H_
#include "curl/curl.h"

// д������
size_t writeData(void *pIn, size_t nSize, size_t nItems, void *pOut);

bool _initCurl(CURL* handle);


#endif // HDFS_URL_H_