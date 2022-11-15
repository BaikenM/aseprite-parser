// Detailed .aseprite file format specification can be found at 
// https://github.com/aseprite/aseprite/blob/main/docs/ase-file-specs.md#frames

#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <fstream>

namespace aseprite
{
	typedef uint8_t BYTE;
	typedef uint16_t WORD;
	typedef int16_t SHORT;
	typedef uint32_t DWORD;
	typedef int32_t LONG;
	//class FIXED;				// represent a fixed point (16.16) value
	//class STRING
	//{
	//	WORD	m_Length;
	//	BYTE* m_Chars;
	//	STRING();
	//	~STRING();
	//};
	
	class Header
	{
	public:
		DWORD	m_FileSize;
		WORD	m_Frames;
		WORD	m_Width;
		WORD	m_Height;
		WORD	m_ColorDepth;
		DWORD	m_Flags;
		WORD	m_Speed;
		BYTE	m_TransparencyIdx;
		WORD	m_NumColors;
		BYTE	m_PixelWidth;
		BYTE	m_PixelHeight;
		SHORT	m_GridPosX;
		SHORT	m_GridPosY;
		WORD	m_GridWidth;
		WORD	m_GridHeight;
	};

	class Frame
	{
	public:
		DWORD   m_ByteNum;			// number of bytes in this frame
		WORD    m_MagicNumber;		// always 0xF1FA
		WORD    m_ChunkNumOld;		// if value 0xFFFF -> more chunks in frame, use new field
		WORD    m_FrameDuration;	// in milliseconds
		DWORD   m_ChunkNumNew;		// if 0, use old field
		Chunk*	m_Chunks;

	private:
		void	readHeader();
	};

	// base class for different chunk types
	class Chunk
	{
		DWORD m_ChunkSize;
		WORD  m_ChunkType;

		//void read();	// read/init this chunk

		//virtual ~Chunk();
	};

	// types of chunks
	class Chunk;
	class OldPaletteChunk_256;	// 0x0004
	class OldPaletteChunk_64;	// 0x0011
	class LayerChunk;			// 0x2004
	class CelChunk;				// 0x2005
	class CelExtraChunk;		// 0x2006
	class ColorProfileChunk;	// 0x2007
	class ExternalFilesChunk;	// 0x2008
	class MaskChunk;			// 0x2016
	class PathChunk;			// 0x2017
	class TagsChunk;			// 0x2018
	class PaletteChunk;			// 0x2019
	class UserDataChunk;		// 0x2020
	class SliceChunk;			// 0x2022
	class TilesetChunk;			// 0x2023
	
	// TODO: PIXEL, TILE
	typedef std::vector<std::pair<std::string, int>> Tags;

	class Aseprite
	{
	public:
		// read file's info
		void				readFileHeader();

		// read frames' info
		//void				readFrame();

			// swap the byte-order (endianness)
		//int64_t convertEndian(BYTE* buff, size_t cnt);

		// read next sizeof(DataType) bytes from file
		template<typename DataType>
		DataType read();

		// skip next count bytes from file
		void skip(size_t count);

		// getters
		std::string filename() const;

		// misc
		void		fileInfo() const;	// get file's basic detail summary

		Aseprite(const std::string& filename);
		~Aseprite();

	private:
		// basic file manipulation data
		std::ifstream		m_IFS;
		const std::string	m_SrcFilename;
		Header				m_Header;
		std::vector<Frame>	m_Frames;
	};
}