/*!
 * \file Tile.h
 * \author Xuebingbing
 * \brief ��Ƭ�����ඨ��
 *
 * TODO: long description
 * \date ���� 2015
 * \copyright	Copyright (c) 2015, �������ƺ�ͨ�����Զ����Ƽ����޹�˾\n
	All rights reserved.
 */

#ifndef TILE_H__
#define TILE_H__

namespace thp
{
	class Bundle;
	class Tile
	{
	public:
		Tile();
		~Tile();

		bool clone(Tile*& pTile);

		// tile ������ռ���ڴ��ֽڴ�С
		unsigned int	m_unSize;

		// ��Դ������ֱ�ӳ���� �����ڴ��
		unsigned char*	m_byteData;

		// ��ʾ���������Ƿ��ͷ� m_byteData
		bool bRelease;
	};
}// namespace thp


#endif // TILE_H__
