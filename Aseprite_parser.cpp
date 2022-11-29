// NOTE: tests were run on 0x0004, 0x2004, 0x2005, 0x2007, 0x2018, 0x2019
#include "Aseprite_parser.h"

using namespace std;
using namespace aseprite;

Header Aseprite::m_Header;
ifstream Aseprite::m_IFS;

unique_ptr<Chunk> Aseprite::createFrameChunk(WORD type) const
{
	switch (type)
	{
	case 0x0004:	return unique_ptr<Chunk>(make_unique<OldPaletteChunk_256>());
	case 0x0011:	return unique_ptr<Chunk>(make_unique<OldPaletteChunk_64>());	
	case 0x2004:	return unique_ptr<Chunk>(make_unique<LayerChunk>());
	case 0x2005:	return unique_ptr<Chunk>(make_unique<CelChunk>());
	case 0x2006:	return unique_ptr<Chunk>(make_unique<CelExtraChunk>());
	case 0x2007:	return unique_ptr<Chunk>(make_unique<ColorProfileChunk>());
	case 0x2008:	return unique_ptr<Chunk>(make_unique<ExternalFilesChunk>());
	case 0x2016:	return unique_ptr<Chunk>(make_unique<MaskChunk>());
	case 0x2017:	return unique_ptr<Chunk>(make_unique<PathChunk>());
	case 0x2018:	return unique_ptr<Chunk>(make_unique<TagsChunk>());
	case 0x2019:	return unique_ptr<Chunk>(make_unique<PaletteChunk>());
	case 0x2020:	return unique_ptr<Chunk>(make_unique<UserDataChunk>());
	case 0x2022:	return unique_ptr<Chunk>(make_unique<SliceChunk>());
	case 0x2023:	return unique_ptr<Chunk>(make_unique<TilesetChunk>());
	default:		return nullptr;
	}
}
Aseprite::Aseprite(const std::string& filename)
	: m_SrcFilename{ filename }
{
	cout << "Opening file: " << m_SrcFilename << endl;
	try
	{
		m_IFS.open(m_SrcFilename, ios::binary);
		if (!m_IFS.is_open())
			throw runtime_error("Could not open file: " + m_SrcFilename);

		readHeader();
		m_Header.print();
		readFrames();
	}
	catch (const std::exception&)
	{
		throw;
	}
}
void Aseprite::readHeader()
{
	m_Header.m_FileSize = read<DWORD>();
	
	// check if we're dealing with .aseprite file
	if (read<WORD>() != 0xA5E0)
		throw runtime_error("This file is not an .aseprite file.");
	
	m_Header.m_Frames =				read<WORD>();
	m_Header.m_Width =				read<WORD>();
	m_Header.m_Height =				read<WORD>();
	m_Header.m_ColorDepth =			read<WORD>();
	m_Header.m_Flags =				read<DWORD>();
	m_Header.m_Speed =				read<WORD>();
	skip(sizeof(DWORD) * 2);	
	m_Header.m_TransparencyIdx =	read<BYTE>();
	skip(3);					
	m_Header.m_NumColors =			read<WORD>();
	m_Header.m_PixelWidth =			read<BYTE>();
	m_Header.m_PixelHeight =		read<BYTE>();
	m_Header.m_GridPosX =			read<SHORT>();
	m_Header.m_GridPosY =			read<SHORT>();
	m_Header.m_GridWidth =			read<WORD>();
	m_Header.m_GridHeight =			read<WORD>();
	skip(84);					
}
void Aseprite::readFrames()
{
	m_Frames.resize(m_Header.m_Frames);
	for (auto& frame : m_Frames)
	{
		// read frame header
		frame.m_Header.m_ByteNum = read<DWORD>();
		frame.m_Header.m_MagicNumber = read<WORD>();
		if (frame.m_Header.m_MagicNumber != 0xF1FA)
			throw runtime_error("Bad frame magic number!");
		frame.m_Header.m_ChunkNumOld = read<WORD>();
		frame.m_Header.m_FrameDuration = read<WORD>();
		skip(2);
		frame.m_Header.m_ChunkNumNew = read<DWORD>();

		auto chunkCount{ frame.m_Header.m_ChunkNumOld == 0xFFFF
				? frame.m_Header.m_ChunkNumNew
				: frame.m_Header.m_ChunkNumOld };

		// read frame chunks
		frame.m_Chunks.reserve(chunkCount);
		for (auto i{ 0 }; i < frame.m_Chunks.capacity(); i++)
		{
			DWORD size = read<DWORD>();
			WORD type = read<WORD>();

			auto chunk = createFrameChunk(type);
			if (!chunk)
				cout << "Unknown chunk type: " << typeid(chunk).name() << endl;
			cout << "Reading chunk: " << hex << type << ", size: " << size << endl;
			chunk.get()->m_ChunkSize = size;
			chunk.get()->read();
			frame.m_Chunks.push_back(move(chunk));
		}
	}
}
template<typename DataType>
DataType Aseprite::read()
{
	DataType data;
	if (!m_IFS.read(reinterpret_cast<char*>(&data), sizeof(data)))
		throw runtime_error("Failed to read data: " + string{ typeid(DataType).name() });
	return data;
}
template<>
FIXED Aseprite::read()
{
	FIXED fxd;
	fxd.m_IntValue = read<WORD>();
	fxd.m_FractValue = read<WORD>();
	return fxd;
}
template<>
STRING Aseprite::read()
{
	STRING str;
	str.m_Length = read<WORD>();
	str.m_Chars.resize(str.m_Length);
	for (auto& ch : str.m_Chars)
		ch = read<BYTE>();
	return str;
}
void Aseprite::skip(size_t count)
{
	m_IFS.seekg(count, ios_base::cur);
	if (!m_IFS)
		throw runtime_error("Failed to read from file or end-of-file reached sooner than expected.");
}
Aseprite::~Aseprite()
{
	m_IFS.close();
}
string Aseprite::filename() const
{
	return m_SrcFilename;
}
WORD Aseprite::getPixelFormat()
{
	return m_Header.m_ColorDepth;
}

// OVERRIDEN READ() FUNCTIONS
void OldPaletteChunk_256::read()
{
	try
	{
		m_PacketNum = Aseprite::read<WORD>();
		m_Packets.resize(m_PacketNum);
		for (auto& packet : m_Packets)
		{
			packet.m_PaletteNum = Aseprite::read<BYTE>();
			packet.m_ColorNum = Aseprite::read<BYTE>();
			packet.m_Colors.resize(packet.m_ColorNum);
			for (auto& color : packet.m_Colors)
			{
				color.m_Red = Aseprite::read<BYTE>();
				color.m_Green = Aseprite::read<BYTE>();
				color.m_Blue = Aseprite::read<BYTE>();
			}
		}
	}
	catch (const std::exception& e)
	{
		cerr << e.what() << endl;
		throw runtime_error("Error reading OldPaletteChunk_256");
	}
}
void OldPaletteChunk_64::read()
{
	try
	{
		m_PacketNum = Aseprite::read<WORD>();
		m_Packets.resize(m_PacketNum);
		for (auto& packet : m_Packets)
		{
			packet.m_PaletteNum = Aseprite::read<BYTE>();
			packet.m_ColorNum = Aseprite::read<BYTE>();
			packet.m_Colors.resize(packet.m_ColorNum);
			for (auto& color : packet.m_Colors)
			{
				color.m_Red = Aseprite::read<BYTE>();
				color.m_Green = Aseprite::read<BYTE>();
				color.m_Blue = Aseprite::read<BYTE>();
			}
		}
	}
	catch (const std::exception& e)
	{
		cerr << e.what() << endl;
		throw runtime_error("Error reading OldPaletteChunk_64");
	}
}
void LayerChunk::read()
{
	try
	{
		m_Flags = Aseprite::read<WORD>();
		m_LayerType = Aseprite::read<WORD>();
		m_LayerChildLevel = Aseprite::read<WORD>();
		m_Width = Aseprite::read<WORD>();
		m_Height = Aseprite::read<WORD>();
		m_BlendMode = Aseprite::read<WORD>();
		m_Opacity = Aseprite::read<BYTE>();
		Aseprite::skip(3);
		m_LayerName = Aseprite::read<STRING>();
		if (m_LayerType == 2)
			m_TilesetIdx = Aseprite::read<DWORD>();
	}
	catch (const std::exception& e)
	{
		cerr << e.what() << endl;
		throw runtime_error("Error reading LayerChunk");
	}
}
void CelChunk::read()
{
	m_LayerIdx = Aseprite::read<WORD>();
	m_X = Aseprite::read<SHORT>();
	m_Y = Aseprite::read<SHORT>();
	m_Opacity = Aseprite::read<BYTE>();
	m_CelType = Aseprite::read<WORD>();
	Aseprite::skip(7);

	size_t chunkHeaderSize = sizeof(DWORD) + sizeof(WORD);
	size_t celChunkHeaderSize = sizeof(WORD) * 2 + sizeof(SHORT) * 2 + sizeof(BYTE) + sizeof(BYTE) * 7;
	auto chunkSizeLeft = this->m_ChunkSize - chunkHeaderSize - celChunkHeaderSize;
	switch (m_CelType)
	{
	case 0:		
	{
		m_Width = Aseprite::read<WORD>();
		m_Height = Aseprite::read<WORD>();
		m_Pixels.resize(m_Width * m_Height);

		auto colorDepth = Aseprite::getPixelFormat();
		for (auto& pixel : m_Pixels)
		{
			pixel.m_Data.resize(colorDepth);
			for (auto& byte : pixel.m_Data)
				byte = Aseprite::read<BYTE>();
		}
		break;
	}
	case 1:		
		m_FramePos = Aseprite::read<WORD>();
		break;
	case 2:
		m_Width = Aseprite::read<WORD>();
		m_Height = Aseprite::read<WORD>();

		m_CompressedPixels.resize(chunkSizeLeft - sizeof(WORD) * 2);
		for (auto i = 0; i < m_CompressedPixels.size(); i++)
			m_CompressedPixels[i] = Aseprite::read<BYTE>();
		//decompress(); // TODO
		break;
	case 3:
		m_Width = Aseprite::read<WORD>();
		m_Height = Aseprite::read<WORD>();
		m_Bits = Aseprite::read<WORD>();
		m_BitmaskID = Aseprite::read<DWORD>();
		m_BitmaskFlipX = Aseprite::read<DWORD>();
		m_BitmaskFlipY = Aseprite::read<DWORD>();
		m_BitmaskRotation = Aseprite::read<DWORD>();
		Aseprite::skip(10);
	
		m_Tiles.resize(chunkSizeLeft - sizeof(WORD) * 3 - sizeof(DWORD) * 4 - sizeof(BYTE) * 10);
		for (auto& tile : m_Tiles)
			tile.m_Data = Aseprite::read<DWORD>();
		break;
	default:		throw runtime_error("Unknown cel type of CelChunk");
	}
}
void CelExtraChunk::read()
{
	m_Flags = Aseprite::read<DWORD>();
	m_PreciseX = Aseprite::read<FIXED>();
	m_PreciseY = Aseprite::read<FIXED>();
	m_CelWidth = Aseprite::read<FIXED>();
	m_CelHeight = Aseprite::read<FIXED>();
	Aseprite::skip(16);
}
void ColorProfileChunk::read()
{
	m_Type = Aseprite::read<WORD>();
	m_Flags = Aseprite::read<WORD>();
	m_FixedGamma = Aseprite::read<FIXED>();
	Aseprite::skip(8);
	if (m_Type == 2)
	{
		m_ICCDataLen = Aseprite::read<DWORD>();
		m_ICCData.resize(m_ICCDataLen);
		for (auto& byte : m_ICCData)
			byte = Aseprite::read<BYTE>();
	}
	else
		m_ICCDataLen = 0;
}
void ExternalFilesChunk::read()
{
	m_EntryNum = Aseprite::read<DWORD>();
	Aseprite::skip(8);
	m_Entries.resize(m_EntryNum);
	for (auto& entry : m_Entries)
	{
		entry.m_ID = Aseprite::read<DWORD>();
		Aseprite::skip(8);
		entry.m_ExternalFilename = Aseprite::read<STRING>();
	}
}
void MaskChunk::read()
{
	m_X = Aseprite::read<SHORT>();
	m_Y = Aseprite::read<SHORT>();
	m_Width = Aseprite::read<WORD>();
	m_Height = Aseprite::read<WORD>();
	Aseprite::skip(8);
	m_Name = Aseprite::read<STRING>();
	m_BitMapData.resize(m_Height * ((m_Width + 7) / 8));
	for (auto& bmData : m_BitMapData)
		bmData = Aseprite::read<BYTE>();
}
void TagsChunk::read()
{
	m_TagNum = Aseprite::read<WORD>();
	Aseprite::skip(8);
	m_Tags.resize(m_TagNum);
	for (auto& tag : m_Tags)
	{
		tag.m_From = Aseprite::read<WORD>();
		tag.m_To = Aseprite::read<WORD>();
		tag.m_AnimationDirection = Aseprite::read<BYTE>();
		tag.m_AnimationRepeatNum = Aseprite::read<WORD>();
		Aseprite::skip(6);
		for (auto& rgb : tag.m_TagColorRGB)
			rgb = Aseprite::read<BYTE>();
		Aseprite::skip(1);
		tag.m_Name = Aseprite::read<STRING>();
	}
}
void PaletteChunk::read()
{
	m_PaletteSize = Aseprite::read<DWORD>();
	m_FirstColorIdx = Aseprite::read<DWORD>();
	m_LastColorIdx = Aseprite::read<DWORD>();
	Aseprite::skip(8);
	m_Entries.resize(m_LastColorIdx - m_FirstColorIdx + 1);
	for (auto& entry : m_Entries)
	{
		entry.m_Flags = Aseprite::read<WORD>();
		entry.m_Red = Aseprite::read<BYTE>();
		entry.m_Green = Aseprite::read<BYTE>();
		entry.m_Blue = Aseprite::read<BYTE>();
		entry.m_Alpha = Aseprite::read<BYTE>();
		if (entry.m_Flags == 1)
			entry.m_ColorName = Aseprite::read<STRING>();
	}
}
void UserDataChunk::read()
{
	m_Flags = Aseprite::read<DWORD>();
	if (m_Flags == 1)
		m_Text = Aseprite::read<STRING>();
	else if (m_Flags == 2)
	{
		m_Red = Aseprite::read<BYTE>();
		m_Red = Aseprite::read<BYTE>();
		m_Red = Aseprite::read<BYTE>();
		m_Red = Aseprite::read<BYTE>();
	}
}
void SliceChunk::read()
{
	m_SliceKeysNum = Aseprite::read<DWORD>();
	m_Flags = Aseprite::read<DWORD>();
	Aseprite::skip(sizeof(DWORD));
	m_Name = Aseprite::read<STRING>();
	m_SliceKeys.resize(m_SliceKeysNum);
	for (auto& sliceKey : m_SliceKeys)
	{
		sliceKey.m_FrameNumber = Aseprite::read<DWORD>();
		sliceKey.m_X = Aseprite::read<LONG>();
		sliceKey.m_Y = Aseprite::read<LONG>();
		sliceKey.m_Width = Aseprite::read<DWORD>();
		sliceKey.m_Height = Aseprite::read<DWORD>();
		if (m_Flags == 1)
		{
			sliceKey.m_CenterX = Aseprite::read<LONG>();
			sliceKey.m_CenterY = Aseprite::read<LONG>();
			sliceKey.m_CenterWidth = Aseprite::read<DWORD>();
			sliceKey.m_CenterHeight = Aseprite::read<DWORD>();
		}
		else if (m_Flags == 2)
		{
			sliceKey.m_PivotX = Aseprite::read<LONG>();
			sliceKey.m_PivotY = Aseprite::read<LONG>();
		}
	}
}
void TilesetChunk::read()
{
	m_TilesetID = Aseprite::read<DWORD>();
	m_Flags = Aseprite::read<DWORD>();
	m_TileNum = Aseprite::read<DWORD>();
	m_TileWidth = Aseprite::read<WORD>();
	m_TileHeight = Aseprite::read<WORD>();
	m_BaseIdx = Aseprite::read<SHORT>();
	Aseprite::skip(14);
	m_Name = Aseprite::read<STRING>();
	if (m_Flags == 1)
	{
		m_ExtFileID = Aseprite::read<DWORD>();
		m_ExtTilesetID = Aseprite::read<DWORD>();
	}
	else if (m_Flags == 2)
	{
		m_ComprDataLen = Aseprite::read<DWORD>();
		m_ComprTilesetImage.resize(m_ComprDataLen);
		auto colorDepth = Aseprite::getPixelFormat();
		for (auto& pixel : m_ComprTilesetImage)
		{
			pixel.m_Data.resize(colorDepth);
			for (auto& byte : pixel.m_Data)
				byte = Aseprite::read<BYTE>();
		}
	}
}
void Header::print() const
{
	cout << endl << setw(30) << "-- FILE HEADER DETAILS --" << endl;

	DWORD fs = m_FileSize;
	cout << left << setw(20) << "Filesize: " << fs << " bytes (" << fs / 1000.0 << " KB)" << endl
		<< setw(20) << "Frames: " << m_Frames << endl
		<< setw(20) << "Width: " << m_Width << endl
		<< setw(20) << "Height: " << m_Height << endl
		<< setw(20) << "ColorDepth " << m_ColorDepth << endl
		<< setw(20) << "Flags " << m_Flags << endl
		<< setw(20) << "Speed " << m_Speed << endl
		<< setw(20) << "TransparencyIdx " << m_TransparencyIdx << endl
		<< setw(20) << "NumColors " << m_NumColors << endl
		<< setw(20) << "PixelWidth " << (unsigned int)m_PixelWidth << endl
		<< setw(20) << "PixelHeight " << (unsigned int)m_PixelHeight << endl
		<< setw(20) << "GridPosX " << m_GridPosX << endl
		<< setw(20) << "GridPosY " << m_GridPosY << endl
		<< setw(20) << "GridWidth " << m_GridWidth << endl
		<< setw(20) << "GridHeight " << m_GridHeight << endl;
}