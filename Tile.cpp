#include "Tile.h"
#include <memory.h>
#include "Bundle.h"

using namespace thp;

Tile::Tile()
{
	m_unSize	= 0;
	m_byteData	= 0;
	bRelease = true;
}

Tile::~Tile()
{
	// ��Դ�ͷŴ���
	if(bRelease)
		delete [] m_byteData;
}

bool Tile::clone(Tile*& pTile)
{
	if(0 == m_unSize)
		return false;

	pTile = new Tile;
	if(0 == pTile)
	{
		// log �ڴ����ʧ��
		return false;
	}

	pTile->m_unSize = this->m_unSize;
	pTile->m_byteData = new unsigned char[m_unSize];
	if( 0 == pTile->m_byteData)
	{
		delete pTile;
		// log �ڴ����ʧ��
		return false;
	}

	memcpy(pTile->m_byteData, this->m_byteData, this->m_unSize);

	return true;
}