#include "StdAfx.h"
#include "Extract/paz/ef_first.h"

#include "ArcFile.h"

/// Dealing with files
///
/// @param archive Archive
///
bool Cef_first::IsSupported(const CArcFile* archive)
{
	// Get header
	u8 header[32];
	memcpy(header, archive->GetHeader(), sizeof(header));

	// Get decryption key
	m_key = header[8];

	// Decrypt
	Decrypt(header, sizeof(header));

	return memcmp(header, "ef_first", 8) == 0;
}

/// Mount
///
/// @param archive Archive
///
bool Cef_first::Mount(CArcFile* archive)
{
	if (!IsSupported(archive))
		return false;

	// Skip 32 bytes
	archive->Seek(32, FILE_BEGIN);

	return CPaz::Mount(archive);
}

/// Decode
///
/// @param archive Archive
///
bool Cef_first::Decode(CArcFile* archive)
{
	if (!IsSupported(archive))
		return false;

	return CPaz::Decode(archive);
}

/// Initialize mount key
///
/// @param archive Archive
///
void Cef_first::InitMountKey(CArcFile* archive)
{
	static const KeyInfo key_info[] =
	{
		{ _T("scr"),    "\xEC\xA1\x5A\x7A\xD0\x79\x7A\xFF\x06\xD4\x30\xCF\xB8\x04\x80\x37\x3E\x86\x1E\xB2\x7C\x08\x44\x14\x57\x42\x0B\xC8\x2B\x0C\xF4\xC7" },
		{ _T("bg"),     "\x15\xED\xC3\xCA\xC7\x7B\x6D\xD6\x38\xF5\x1A\x91\x50\x3D\x94\x85\x39\x85\xBE\x56\x1E\xA0\xF0\x1B\xA3\x1D\x78\x92\xD1\x0D\xB6\xBD" },
		{ _T("st"),     "\x90\x80\x72\xD3\x07\x2E\x3D\xB9\x5C\xC8\x99\xC8\x12\xB5\x58\xA3\xC8\x8E\xB2\xFF\x5A\xC7\x8C\x62\x98\x65\xFA\x84\xD4\x0A\x9E\xFA" },
		{ _T("sys"),    "\x13\x29\x55\x49\xBB\x67\x35\x17\xD3\x43\x4F\x47\xEE\x5E\x64\x14\x4E\x03\x8A\x12\x1F\xED\x0E\xBF\x62\x6D\x96\xF7\xCE\xD9\xF0\x30" },
		{ _T("bgm"),    "\xF6\x69\xD5\xA1\x0E\x21\xCC\xA0\x18\x63\xC3\xB3\x43\xDF\xCF\x0F\xE3\x14\x27\xC6\x55\x7B\x9F\xD4\x78\xEB\x3C\x5B\x1F\x9F\x6D\xB6" },
		{ _T("voice"),  "\x22\xBF\x45\xD9\x43\x0A\xEF\x7D\xBA\x3D\x95\x5D\x42\xAF\x39\x90\x0B\xC2\x4C\x8C\x72\x0D\x08\x81\x08\x5A\x30\x64\xC7\x0B\x8F\x8D" },
		{ _T("se"),     "\x73\x9E\xB8\xFD\x20\x1A\x9C\xFC\x1B\x2D\x44\x83\x15\x3C\x06\x65\x79\x88\x8C\xCD\x75\x26\xB3\xE5\x9C\x35\x60\x85\x98\x14\x90\x53" },
		{ _T("mov"),    "\x75\xE3\xE5\x40\xC9\x5B\x77\x17\x34\x4A\xD9\xC7\x12\xE3\x5A\xBB\x31\x60\x8D\xDB\x04\x57\xFD\xC2\x95\xC4\x0A\x7A\x14\xB5\xC0\x08" }
	};

	SetKey(archive, key_info);
}

/// Initialize decode key
///
/// @param archive Archive
///
void Cef_first::InitDecodeKey(CArcFile* archive)
{
	m_movie_table_id = 0;

	static const KeyInfo key_info[] =
	{
		{ _T("scr"),    "\x2A\x96\xDA\xE6\xCF\xB3\xD5\x9C\x77\x57\x8E\xE3\x9E\x03\xE4\x2A\x49\x38\x2F\xDD\x12\x83\xBA\x7A\x66\x8A\x03\xF3\xAB\xCA\x8E\xA7" },
		{ _T("bg"),     "\xD6\x36\xFD\x61\xEA\xDD\x22\x11\x36\xD0\x10\x94\x67\x85\xF7\x84\x5A\xD6\x5F\x5F\xFD\xC5\xA1\xD2\x72\x7B\xBC\x6A\x81\xF6\x53\xD5" },
		{ _T("st"),     "\xBC\x25\x69\x24\x43\x3F\x7B\x23\x82\x26\x3D\x9C\x7B\x5C\xD7\x35\x5F\xC6\x26\xC3\x66\xF6\x48\x14\x37\x6C\x6C\xF8\xEA\x73\x36\x93" },
		{ _T("sys"),    "\x6C\xAA\x90\x3D\x57\x25\x57\x99\xEB\x65\xC5\xCF\xB1\xB9\xE2\x27\x7A\xF1\x30\x93\xBD\xEF\xE9\x79\xAF\x5D\xAC\x91\xBE\xD0\x53\x15" },
		{ _T("bgm"),    "\x6B\x99\xAD\x59\xCF\xB0\xED\x3D\x3D\x83\xE1\x08\xD7\x8E\xD3\x3C\xA2\x22\x0E\x01\x23\x71\x00\x59\x8B\x83\xCF\x77\x6B\x97\x45\xCE" },
		{ _T("voice"),  "\xA1\x3B\xBA\x88\x3A\xA3\xDD\x0C\x30\x92\xA8\xAD\xFE\x25\x80\x14\xCD\x5B\x0D\x57\x9D\x8A\x21\x9C\xED\x2D\xB8\xD6\x82\x32\x0D\x3A" },
		{ _T("se"),     "\xCC\x17\x29\x26\x10\xA9\x20\x18\x17\xB8\xDD\xF0\xD2\x62\xCD\x44\x53\xCE\x38\x98\x81\xB6\xB4\x31\x3F\x08\x1C\xA0\xAA\xF3\x86\x6E" },
		{ _T("mov"),    "\x40\x28\x70\x54\x8C\x92\xDD\xBF\xF2\xC8\xEC\x43\x2F\x42\x0F\xC9\x54\x0F\x65\x31\xF4\x7B\xE0\xDA\x35\x21\x1C\x8D\x96\x4E\x73\x61" }
	};

	SetKey(archive, key_info);
}

/// Initialize Movie Table
///
/// @param table Table
///
/// @return table size
///
u32 Cef_first::InitMovieTable(const u8* table)
{
	for (size_t i = 0; i < 256; i++)
	{
		for (size_t j = 0; j < 256; j++)
		{
			m_movie_table[i][*table++] = static_cast<u8>(j);
		}
	}

	return 65536;
}

/// Decode table 1
void Cef_first::DecodeTable1()
{
	u32*      table = GetTable();
	const u8* key = GetKey();
	size_t    key_ptr = 0;

	static constexpr std::array<u8, 6> key_table{{
		0x6D, 0x69, 0x6E, 0x6F, 0x72, 0x69
	}};

	for (size_t i = 0; i < 18; i++)
	{
		u32 value = 0;

		for (size_t j = 0; j < 4; j++)
		{
			value <<= 8;
			value |= key[key_ptr] ^ key_table[key_ptr % 6];

			key_ptr = (key_ptr + 1) & 0x1F;
		}

		table[i] ^= value;
	}
}

/// Decode movie table
///
/// @param target Data to be decoded
/// @param size   Decoding size
///
void Cef_first::DecodeMovieData(u8* target, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		target[i] = m_movie_table[m_movie_table_id][target[i]];
	}

	m_movie_table_id = (m_movie_table_id + 1) & 0xFF;
}

/// Cipher-specific Decryption
///
/// @parma target Data to be decoded
/// @param size   Decoding size
///
void Cef_first::Decrypt(u8* target, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		target[i] ^= m_key;
	}
}

/// Get movie buffer size
///
/// @param archive Archive
///
size_t Cef_first::GetMovieBufSize(CArcFile* archive)
{
	return 65536;
}
