#ifndef _SSOAPCALLBACKFUNC_H_
#define _SSOAPCALLBACKFUNC_H_


//struct TGetHandler
//{
//	TGetHandler(void* p);
//
//	int operator()(struct soap*);
//};

int http_get_handler(struct soap*);	/* HTTP get handler */

int http_fpost_handler(struct soap*, const char*, const char*, int, const char*, const char*, size_t);

/// gsoap�Ļص�������д�����Ŀ����Ϊ��ʵ��js�ͻ��˵Ŀ������
/// if (err = soap->fposthdr(soap, "Access-Control-Allow-Origin", "*")) :��ʾ����
/// if ((err = soap->fposthdr(soap, "Server", "WebGISServer")): ָ����������ĳ���
int http_set_response(struct soap *soap, int status, size_t count);

#endif
