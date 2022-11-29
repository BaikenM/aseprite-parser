// Detailed .aseprite file format specification can be found at 
// https://github.com/aseprite/aseprite/blob/main/docs/ase-file-specs.md#frames

#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include <map>

namespace aseprite
{
	typedef uint8_t BYTE;
	typedef uint16_t WORD;
	typedef int16_t SHORT;
	typedef uint32_t DWORD;
	typedef int32_t LONG;

	struct FIXED
	{
		WORD	m_IntValue;
		WORD	m_FractValue;
	};
	struct PIXEL
	{
		std::vector<BYTE>	m_Data;
	};
	struct STRING
	{
		WORD				m_Length;
		std::vector<BYTE>	m_Chars;
	};
	struct TILE
	{
		DWORD m_Data;
	};

	struct Header
	{
	public:
		void	print() const;

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
	struct Chunk
	{
	public:
		virtual void	read() = 0;
		DWORD			m_ChunkSize;
		
		Chunk() = default;
		Chunk(DWORD size) : m_ChunkSize{ size }{}
		virtual	~Chunk() = default;
	};
	struct Frame
	{
	public:
		struct Header
		{
			DWORD   m_ByteNum;			
			WORD    m_MagicNumber;		// always 0xF1FA
			WORD    m_ChunkNumOld;		// if value 0xFFFF -> more chunks in frame, use new field
			WORD    m_FrameDuration;	
			DWORD   m_ChunkNumNew;		// if 0, use old field
		} m_Header;
		std::vector<std::unique_ptr<Chunk>> m_Chunks;
	};

	struct Color
	{
		BYTE    m_Red;		
		BYTE    m_Green;	
		BYTE    m_Blue;		
	};
	struct Packet	//	for 0x0004 and 0x0011
	{
		BYTE				m_PaletteNum;
		BYTE				m_ColorNum;
		std::vector<Color>	m_Colors;	
	};

	struct OldPaletteChunk_256 : public Chunk
	{
		void				read() override;
		WORD				m_PacketNum;
		std::vector<Packet> m_Packets;
	};
	struct OldPaletteChunk_64	: public Chunk
	{
		void				read() override;
		WORD				m_PacketNum;
		std::vector<Packet> m_Packets;
	};
	struct LayerChunk			: public Chunk
	{
		void		read() override;
		WORD        m_Flags;
		WORD        m_LayerType;
		WORD        m_LayerChildLevel;
		WORD        m_Width;
		WORD        m_Height;
		WORD		m_BlendMode;
		BYTE        m_Opacity;		// valid if file header flag is set
		STRING      m_LayerName;
		// if m_LayerType = 2
		DWORD		m_TilesetIdx;	
	};
	struct CelChunk				: public Chunk
	{
		void				read() override;
		WORD				m_LayerIdx;
		SHORT				m_X;
		SHORT				m_Y;
		BYTE				m_Opacity;
		WORD				m_CelType;
		WORD				m_Width;
		WORD				m_Height;
		// m_Type = 0 (Raw Image Data)
		std::vector<PIXEL>	m_Pixels;
		// m_Type = 1 (Linked Cel)
		WORD				m_FramePos;
		// m_Type = 2 (Compressed Image)
		std::vector<BYTE>	m_CompressedPixels;		// compressed with ZLIB
		// m_Type = 3 (Compressed Tilemap)
		WORD				m_Bits;				
		DWORD				m_BitmaskID;		
		DWORD				m_BitmaskFlipX;		
		DWORD				m_BitmaskFlipY;		
		DWORD				m_BitmaskRotation;	
		std::vector<TILE>	m_Tiles;			// Row by row, top to bottom, ZLIB
	};
	struct CelExtraChunk		: public Chunk
	{
		void		read() override;
		DWORD       m_Flags;
		// If m_Flags = 1
		FIXED       m_PreciseX;
		FIXED       m_PreciseY;
		FIXED       m_CelWidth; 
		FIXED       m_CelHeight;	
	};
	struct ColorProfileChunk	: public Chunk
	{
		void				read() override;
		WORD				m_Type;
		WORD				m_Flags;
		FIXED				m_FixedGamma;	// (1.0 = linear)
		// If m_Type = ICC
		DWORD				m_ICCDataLen;
		std::vector<BYTE>	m_ICCData;		// http://www.color.org/ICC1V42.pdf
	};
	struct ExternalFilesChunk	: public Chunk
	{
		void				read() override;
		DWORD				m_EntryNum;
		struct Entry 
		{
			DWORD	m_ID;					// referenced by tilesets or palettes
			STRING	m_ExternalFilename;
		};
		std::vector<Entry>	m_Entries;
	};
	struct MaskChunk			: public Chunk
	{
		void				read() override;
		SHORT				m_X;
		SHORT				m_Y;
		WORD				m_Width;
		WORD				m_Height;
		STRING				m_Name;
		std::vector<BYTE>	m_BitMapData;
	};
	struct PathChunk : public Chunk	// never used
	{
		void read() override {}		// do nothing
	};
	struct TagsChunk			: public Chunk
	{
		void		read() override;
		WORD        m_TagNum;
		struct Tag
		{
			WORD		m_From;
			WORD		m_To;
			BYTE		m_AnimationDirection;	
			WORD		m_AnimationRepeatNum; 
			BYTE		m_TagColorRGB[3];
			STRING		m_Name;
		};
		std::vector<Tag> m_Tags;
	};
	struct PaletteChunk			: public Chunk
	{
		void		read() override;
		DWORD       m_PaletteSize;
		DWORD       m_FirstColorIdx;
		DWORD       m_LastColorIdx;
		struct PaletteEntry
		{
			WORD    m_Flags;		
			BYTE    m_Red;			
			BYTE    m_Green;		
			BYTE    m_Blue;			
			BYTE    m_Alpha;		
			// If m_Flags = 1
			STRING	m_ColorName;	

		};
		std::vector<PaletteEntry>	m_Entries;
	};
	struct UserDataChunk		: public Chunk
	{
		void		read() override;
		DWORD       m_Flags;
		// If m_Flags = 1 
		STRING		m_Text;		
		// If m_Flags = 2
		BYTE		m_Red;		
		BYTE		m_Green;	
		BYTE		m_Blue;		
		BYTE		m_Alpha;	
	};
	struct SliceChunk			: public Chunk
	{
		void		read() override;
		DWORD       m_SliceKeysNum;
		DWORD       m_Flags;	
		DWORD       m_Reserved;
		STRING      m_Name;
		struct SliceKey
		{
			DWORD	m_FrameNumber;
			LONG	m_X;
			LONG	m_Y;
			DWORD	m_Width;
			DWORD	m_Height;
			// if m_Flags = 1
			LONG	m_CenterX;		
			LONG	m_CenterY;	
			DWORD   m_CenterWidth;
			DWORD   m_CenterHeight;
			// if m_Flags = 2
			LONG    m_PivotX;		
			LONG    m_PivotY;	
		};
		std::vector<SliceKey> m_SliceKeys;
	};
	struct TilesetChunk			: public Chunk
	{
		void				read() override;
		DWORD				m_TilesetID;
		DWORD				m_Flags;
		DWORD				m_TileNum;
		WORD				m_TileWidth;
		WORD				m_TileHeight;
		SHORT				m_BaseIdx;
		STRING				m_Name;
		// If m_Flags = 1
		DWORD				m_ExtFileID;			
		DWORD				m_ExtTilesetID;
		// If m_Flags = 2
		DWORD				m_ComprDataLen;			
		std::vector<PIXEL>	m_ComprTilesetImage;
	};

	class Aseprite
	{
	public:
		// read file data 
		void				readHeader();
		void				readFrames();

		// READ next sizeof(DataType) bytes from file
		template<typename DataType>
		static DataType		read();

		std::unique_ptr<Chunk> createFrameChunk(WORD type) const;

		// SKIP next count bytes from file
		static void skip(size_t count);

		// getters
		std::string filename() const;
		static WORD getPixelFormat();

		// misc

		Aseprite(const std::string& filename);
		~Aseprite();

	private:
		static std::ifstream		m_IFS;
		// basic file manipulation data
		const std::string	m_SrcFilename;
		static Header		m_Header;
		std::vector<Frame>	m_Frames;
	};
}