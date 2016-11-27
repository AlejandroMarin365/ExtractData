#include "StdAfx.h"
#include "Extract/Oyatu.h"

#include "ArcFile.h"
#include "File.h"

/// Determine if file is supported
///
/// @param archive Archive
///
bool COyatu::IsSupported(CArcFile* archive)
{
	if (!archive->CheckExe(_T("MEBIUS35.DAT")))
		return false;

	// Read MEBIUS35.DAT
	TCHAR dat_path[MAX_PATH];
	lstrcpy(dat_path, archive->GetArcPath());
	PathRemoveFileSpec(dat_path);
	PathAppend(dat_path, _T("MEBIUS35.DAT"));

	CFile dat;
	if (!dat.OpenForRead(dat_path))
		return false;

	dat.SeekHed(4);

	char game_title[15];
	if (dat.Read(game_title, sizeof(game_title)) < sizeof(game_title))
		return false;

	return strcmp(game_title, "おやつのじかん") == 0;
}

/// Mounting 
///
/// @param archive Archive
///
bool COyatu::Mount(CArcFile* archive)
{
	// Check if supported
	if (!IsSupported(archive))
		return false;

	// Mount
	if (archive->GetArcExten() == _T(".BGM"))
		return archive->Mount();

	if (archive->GetArcExten() == _T(".KOE"))
		return archive->Mount();

	if (archive->GetArcExten() == _T(".MSE"))
		return archive->Mount();

	return false;
}

/// Decoding
///
/// @param archive Archive
///
bool COyatu::Decode(CArcFile* archive)
{
	// Check if supported
	if (!IsSupported(archive))
		return false;

	// Decode
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	// BGM
	if (file_info->format == _T("BGM"))
		return DecodeBGM(archive);

	// KOE
	if (file_info->format == _T("KOE"))
		return DecodeKOE(archive);

	// MSE
	if (file_info->format == _T("MSE"))
		return DecodeMSE(archive);

	return false;
}

/// Sound file decoding
///
/// @param archive Archive
/// @param key     Key
///
bool COyatu::DecodeSound(CArcFile* archive, const u8* key)
{
	// Read header section
	u8 header[44];
	archive->Read(header, sizeof(header));

	// Get the size of the data section
	const u32 data_size = *reinterpret_cast<const u32*>(&header[40]);

	// Open the output file
	archive->OpenFile(_T(".wav"));

	// Output the header part
	archive->WriteFile(header, sizeof(header));

	// Output the data part
	size_t buffer_size = archive->GetBufSize();
	std::vector<u8> buffer(buffer_size);

	for (size_t total_written = 0; total_written < data_size; total_written += buffer_size)
	{
		// Adjust the buffer size
		archive->SetBufSize(&buffer_size, total_written);

		// Read
		archive->Read(buffer.data(), buffer_size);

		// Decode
		for (size_t count = 0; count < buffer_size; count++)
		{
			const size_t key_index = (total_written + count) & 0xFF;

			buffer[count] ^= key[key_index];
		}

		// Write
		archive->WriteFile(buffer.data(), buffer_size);
	}

	return true;
}

/// BGM Decoding
///
/// @param archive Archive
///
bool COyatu::DecodeBGM(CArcFile* archive)
{
	static const u8 key[256] = {
		0xB0, 0x6F, 0xA4, 0xD7, 0x8B, 0x81, 0xBD, 0xF3, 0x82, 0xAF, 0x95, 0x6B, 0x9D, 0x3E, 0x88, 0x73,
		0xB8, 0xF9, 0xD8, 0x09, 0x31, 0xF3, 0x84, 0xDA, 0xCC, 0xAF, 0x54, 0x60, 0xFD, 0x97, 0x04, 0xA6,
		0x05, 0x65, 0x20, 0x9A, 0xA7, 0x62, 0xD9, 0xD7, 0x5C, 0x98, 0x6F, 0x2D, 0x3A, 0x6E, 0x07, 0xF8,
		0x86, 0x34, 0xF9, 0x05, 0xAB, 0x25, 0xF5, 0x70, 0x79, 0x64, 0x03, 0x7C, 0xF2, 0xF6, 0xBF, 0x9B,
		0x91, 0xBB, 0x6B, 0x2A, 0xB3, 0xEB, 0xF2, 0x42, 0x39, 0x27, 0xD2, 0xF0, 0xEA, 0x00, 0x7A, 0x57,
		0xAF, 0xB2, 0xCE, 0xEE, 0xBE, 0xCE, 0x1B, 0x87, 0x4E, 0x1F, 0xA4, 0xB0, 0xD1, 0x8E, 0x79, 0x9D,
		0x6E, 0xC4, 0x26, 0xBF, 0x26, 0xDD, 0x39, 0x2D, 0x54, 0x49, 0x0B, 0xF6, 0x19, 0xDF, 0x3E, 0x19,
		0x2F, 0xA2, 0x6A, 0x2A, 0x66, 0xAC, 0x68, 0x60, 0xA6, 0xEB, 0xB7, 0x24, 0xC0, 0x85, 0x5E, 0x40,
		0xF8, 0x8B, 0xD8, 0x68, 0x78, 0xE8, 0x66, 0xA1, 0xAA, 0xA7, 0xC7, 0x4F, 0x2C, 0x6D, 0xD1, 0x5F,
		0xC4, 0xE6, 0x46, 0x6F, 0x1B, 0x69, 0xF7, 0xD1, 0x23, 0x89, 0x1C, 0x53, 0x6E, 0x75, 0x41, 0xAF,
		0x52, 0xCB, 0x6D, 0x03, 0x28, 0x38, 0xBE, 0x41, 0xFE, 0x99, 0x9D, 0xDF, 0x96, 0x7C, 0xD8, 0xDE,
		0xF3, 0x95, 0x3E, 0x47, 0x5E, 0xA7, 0x05, 0x43, 0xA8, 0x6B, 0x96, 0x8C, 0x89, 0x6D, 0x16, 0x29,
		0x60, 0x74, 0x31, 0x47, 0x36, 0xE2, 0x91, 0x3D, 0x57, 0xAD, 0x81, 0x63, 0xC8, 0xD5, 0x9A, 0x5F,
		0x03, 0x7A, 0x14, 0x10, 0x32, 0x7B, 0xF1, 0x33, 0xDE, 0xBA, 0x52, 0x74, 0xC7, 0x6E, 0xF8, 0x7E,
		0x4C, 0x2C, 0x58, 0x3B, 0xA9, 0x7A, 0x51, 0x5C, 0xFD, 0xA5, 0xCF, 0x67, 0xB8, 0x34, 0x85, 0x3D,
		0x7D, 0x93, 0xE9, 0x7E, 0x9E, 0x6E, 0xC3, 0xB2, 0xB1, 0xD0, 0x5C, 0x83, 0x61, 0x6F, 0x27, 0x18
	};

	return DecodeSound(archive, key);
}

/// KOE Decoding
///
/// @param archive Archive
///
bool COyatu::DecodeKOE(CArcFile* archive)
{
	static const u8 key[256] = {
		0x15, 0xEE, 0x1F, 0x83, 0x32, 0x20, 0xF8, 0x17, 0x53, 0xE3, 0x7B, 0xC0, 0x6A, 0x75, 0x93, 0xA5,
		0x79, 0x32, 0x36, 0x7A, 0x76, 0xC5, 0xF4, 0x06, 0xC5, 0x08, 0xF5, 0x1E, 0xE4, 0xD5, 0xED, 0x72,
		0x0B, 0xEC, 0x2A, 0x52, 0x6D, 0x87, 0xC3, 0x55, 0xD9, 0xC0, 0x07, 0x7A, 0x5E, 0x84, 0x35, 0x38,
		0xB7, 0x0C, 0x17, 0x8A, 0x22, 0xB4, 0x17, 0xFB, 0xEE, 0xA1, 0x57, 0xAE, 0x51, 0x09, 0xF3, 0xE9,
		0x65, 0x0F, 0x66, 0x3B, 0xD1, 0x91, 0x51, 0x0F, 0x08, 0x58, 0xC3, 0x75, 0x0D, 0x69, 0x3C, 0x65,
		0xC4, 0x92, 0x1E, 0x27, 0x32, 0x69, 0x93, 0xD3, 0x19, 0xBA, 0xAF, 0x00, 0x87, 0x38, 0x79, 0xFB,
		0x24, 0xEA, 0xAE, 0x4E, 0x4C, 0x1C, 0x06, 0xCF, 0xD9, 0xD7, 0x4E, 0x80, 0x2C, 0x27, 0xBF, 0x07,
		0x38, 0xA6, 0x48, 0xF9, 0x43, 0x2E, 0x32, 0xD4, 0x13, 0x09, 0x7B, 0xBB, 0xAC, 0x92, 0x99, 0xF8,
		0x70, 0xAC, 0xA1, 0xD0, 0x2A, 0x59, 0x8F, 0x17, 0xEF, 0xFE, 0x85, 0x9B, 0x53, 0x15, 0xDA, 0xE9,
		0xC7, 0xBD, 0xD4, 0x64, 0x55, 0x9C, 0x42, 0x38, 0x4E, 0x55, 0x7D, 0x3D, 0xCB, 0x96, 0xF0, 0xA8,
		0x14, 0x92, 0x21, 0x3E, 0xA1, 0xCC, 0xF0, 0xD9, 0x0F, 0xA1, 0x0B, 0x00, 0xFD, 0x5C, 0xAE, 0x4E,
		0x53, 0x61, 0xC6, 0xF6, 0xCE, 0xA5, 0x91, 0x2C, 0x62, 0x01, 0x3A, 0x17, 0x53, 0x1A, 0xA1, 0x47,
		0xFE, 0xF1, 0xD1, 0x42, 0x48, 0xD3, 0xBB, 0x7F, 0x1D, 0xA8, 0xC7, 0x96, 0x8E, 0xFC, 0x5E, 0xEA,
		0x5A, 0xAD, 0xE8, 0xFB, 0x78, 0x8B, 0x76, 0xD2, 0x86, 0x7B, 0x79, 0x0B, 0x96, 0xC4, 0x51, 0x04,
		0x43, 0x30, 0x20, 0x3F, 0x19, 0x19, 0x88, 0xE3, 0x27, 0x10, 0x65, 0xFE, 0xC8, 0x4A, 0x11, 0x67,
		0x01, 0x55, 0x46, 0xEE, 0x80, 0x68, 0xC9, 0xC1, 0x1B, 0x4C, 0x49, 0x14, 0xC9, 0x95, 0xA9, 0x7F
	};

	return DecodeSound(archive, key);
}

/// MSE Decoding
///
/// @param archive Archive
///
bool COyatu::DecodeMSE(CArcFile* archive)
{
	static const u8 key[256] = {
		0x06, 0xDE, 0xEF, 0x76, 0xD2, 0xDA, 0xE7, 0x95, 0x7A, 0x87, 0x6D, 0x7C, 0xF6, 0x17, 0x44, 0x9F,
		0x08, 0xD2, 0xC5, 0x89, 0xDC, 0xDE, 0xA1, 0x0F, 0x2D, 0xCB, 0xCA, 0xB8, 0x6E, 0xBB, 0x7F, 0x8A,
		0x9E, 0x63, 0x70, 0x58, 0xCC, 0xA8, 0x61, 0x34, 0x68, 0x98, 0xD8, 0xB3, 0x74, 0x18, 0x2C, 0x9B,
		0x1F, 0x64, 0xFD, 0x28, 0xF4, 0x56, 0x8E, 0x80, 0x98, 0x06, 0x5C, 0x13, 0x38, 0x31, 0xFE, 0x62,
		0xBD, 0x11, 0xBC, 0x05, 0x58, 0xF6, 0x38, 0xDD, 0x6A, 0x5A, 0x01, 0x4C, 0x03, 0xC1, 0x0A, 0xE0,
		0x58, 0xA2, 0x88, 0x4E, 0xF6, 0x96, 0x1A, 0x2A, 0xA0, 0xA1, 0xA5, 0xAF, 0x04, 0x4E, 0x08, 0x99,
		0xC8, 0xDA, 0x9B, 0x4C, 0xA4, 0xD2, 0x82, 0x51, 0xE2, 0xBB, 0x33, 0xFC, 0x23, 0xAF, 0xAD, 0x22,
		0xB7, 0x98, 0x5D, 0x36, 0xD3, 0xE7, 0x8C, 0x54, 0x0C, 0x6A, 0xE6, 0x6D, 0x53, 0x28, 0xFA, 0xAE,
		0xE7, 0x66, 0x36, 0x50, 0xE6, 0x40, 0xBA, 0xDF, 0xFA, 0xE2, 0xA3, 0xCC, 0xDB, 0x70, 0x89, 0x27,
		0x88, 0x0A, 0x59, 0x6D, 0x81, 0x06, 0xD1, 0x5A, 0x65, 0x5E, 0xC3, 0x7F, 0x2F, 0xC7, 0x5D, 0xB2,
		0x86, 0x91, 0x19, 0x0A, 0xD7, 0x33, 0xA8, 0xF0, 0x21, 0xA7, 0xE7, 0x19, 0xB5, 0x07, 0xB4, 0xCA,
		0x59, 0xEA, 0xB8, 0xD4, 0xFB, 0x21, 0xF8, 0xAC, 0x7C, 0x2F, 0x45, 0xEA, 0x22, 0x2B, 0x58, 0x4A,
		0x55, 0xEA, 0xB6, 0x45, 0x34, 0x96, 0xAE, 0xFD, 0x86, 0x97, 0xF9, 0x93, 0xBE, 0x6A, 0x6A, 0xFB,
		0x7B, 0x65, 0x21, 0x24, 0x42, 0x5C, 0x37, 0x4F, 0x64, 0x45, 0x58, 0x0C, 0xBC, 0xC1, 0xB7, 0xAD,
		0xC7, 0xB6, 0xE3, 0x21, 0xBB, 0xC8, 0xD2, 0x15, 0x1F, 0xF1, 0x39, 0x3F, 0x87, 0x86, 0x88, 0xBE,
		0x84, 0xD7, 0x1A, 0x63, 0xD5, 0x51, 0x63, 0xDB, 0x74, 0x39, 0x4C, 0x12, 0x12, 0xF1, 0x6E, 0x2C
	};

	return DecodeSound(archive, key);
}
