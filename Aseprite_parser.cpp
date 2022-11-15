#include "Aseprite_parser.h"

using namespace std;
using namespace aseprite;

//int64_t Aseprite::convertEndian(BYTE* buff, size_t cnt)
//{
//	int64_t res{ 0 };
//	for (auto i{ 0 }; i < cnt; i++)
//	{
//		res += buff[i] << 8 * i;
//	}
//	return res;
//}

//class aseprite::FIXED
//{
//	WORD m_IntValue;
//	WORD m_FractValue;
//
//	FIXED()	// reset bytes
//	{
//		m_IntValue   = 0x0000;
//		m_FractValue = 0x0000;
//	}
//};
//
//aseprite::STRING
//{
//	
//	STRING(WORD len)
//	{
//		m_Length = len;
//		m_Chars  = new BYTE[m_Length];
//	}
//	~STRING()
//	{
//		delete[] m_Chars;
//	}
//};




void Frame::readHeader()
{

}

Aseprite::Aseprite(const std::string& filename)
	: m_SrcFilename{ filename }
{
	cout << "Opening file: " << m_SrcFilename << endl;

	m_IFS.open(m_SrcFilename);

	if (!m_IFS.is_open())
		throw runtime_error("Could not open file: " + m_SrcFilename);

	readFileHeader();
	m_Frames.resize(m_Header.m_Frames);
	for (auto& frame : m_Frames)
	{
		frame.readHeader();
		//readFrame();
	}
}

// TODO: extract the rest of file data
void Aseprite::readFileHeader()
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
	skip(sizeof(DWORD) * 2);		// 2x 0 byte
	m_Header.m_TransparencyIdx =	read<BYTE>();
	skip(3);						// ignore
	m_Header.m_NumColors =			read<WORD>();
	m_Header.m_PixelWidth =			read<BYTE>();
	m_Header.m_PixelHeight =		read<BYTE>();
	m_Header.m_GridPosX =			read<SHORT>();
	m_Header.m_GridPosY =			read<SHORT>();
	m_Header.m_GridWidth =			read<WORD>();
	m_Header.m_GridHeight =			read<WORD>();
	skip(84);						// reserved
}

void Aseprite::fileInfo() const
{
	cout << endl << setw(30) << "-- FILE HEADER DETAILS --" << endl;

	DWORD fs = m_Header.m_FileSize;
	cout << left << setw(20) << "Filesize: " << fs << " bytes (" << fs / 1000.0 << " KB)" << endl
		<< setw(20) << "Frames: " << m_Header.m_Frames << endl
		<< setw(20) << "Width: " << m_Header.m_Width << endl
		<< setw(20) << "Height: " << m_Header.m_Height << endl
		<< setw(20) << "ColorDepth " << m_Header.m_ColorDepth << endl
		<< setw(20) << "Flags " << m_Header.m_Flags << endl
		<< setw(20) << "Speed " << m_Header.m_Speed << endl
		<< setw(20) << "TransparencyIdx " << m_Header.m_TransparencyIdx << endl
		<< setw(20) << "NumColors " << m_Header.m_NumColors << endl
		<< setw(20) << "PixelWidth " << m_Header.m_PixelWidth << endl
		<< setw(20) << "PixelHeight " << m_Header.m_PixelHeight << endl
		<< setw(20) << "GridPosX " << m_Header.m_GridPosX << endl
		<< setw(20) << "GridPosY " << m_Header.m_GridPosY << endl
		<< setw(20) << "GridWidth " << m_Header.m_GridWidth << endl
		<< setw(20) << "GridHeight " << m_Header.m_GridHeight << endl << endl;
}

template<typename DataType>
DataType Aseprite::read()
{
	DataType data;

	if (!m_IFS.read(reinterpret_cast<char*>(&data), sizeof(data)))
		throw runtime_error("Failed to read data.");

	return data;
}

void Aseprite::skip(size_t count)
{
	m_IFS.seekg(count, ios_base::cur);
	if (!m_IFS)
		throw runtime_error("Failed to read from file or end-of-file reached sooner than expected.");
}

Aseprite::~Aseprite()
{
	delete[] m_Frames;
	m_IFS.close();
}

string Aseprite::filename() const
{
	return m_SrcFilename;
}