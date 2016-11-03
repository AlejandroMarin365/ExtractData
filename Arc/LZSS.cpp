#include "stdafx.h"
#include "../ArcFile.h"
#include "../Image.h"
#include "LZSS.h"

#include <vector>

/// Decode
///
/// @param archive Archive
///
bool CLZSS::Decode(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format != _T("LZ"))
		return false;

	return Decomp(archive, 4096, 4078, 3);
}

/// Extract the file
///
/// @param archive       Archive
/// @param dic_size      Dictionary size
/// @param dic_ptr       Initial address to the dictionary position (dictionary pointer)
/// @param length_offset Length offset
///
bool CLZSS::Decomp(CArcFile* archive, size_t dic_size, size_t dic_ptr, size_t length_offset)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	// Read
	std::vector<u8> src(file_info->sizeCmp);
	archive->Read(src.data(), src.size());

	// Buffer allocation for extraction
	std::vector<u8> dst(file_info->sizeOrg);

	// Decompression
	Decomp(dst.data(), dst.size(), src.data(), src.size(), dic_size, dic_ptr, length_offset);

	// Bitmap
	if (lstrcmp(PathFindExtension(file_info->name), _T(".bmp")) == 0)
	{
		CImage clImage;
		clImage.Init(archive, dst.data());
		clImage.Write(dst.size());
		clImage.Close();
	}
	else // Other
	{
		archive->OpenFile();
		archive->WriteFile(dst.data(), dst.size());
		archive->CloseFile();
	}

	return true;
}

/// Extract from memory
///
/// @param dst          Destination
/// @param dst_size     Destination size
/// @param src          Compressed data
/// @param src_size     Compressed data size
/// @param dic_size     Dictionary size
/// @param dic_ptr      Initial address to the dictionary position (dictionary pointer)
/// @param length_offset Length offset
///
bool CLZSS::Decomp(u8* dst, size_t dst_size, const u8* src, size_t src_size, size_t dic_size, size_t dic_ptr, size_t length_offset)
{
	// Allocate dictionary buffer
	std::vector<u8> dictionary(dic_size);

	// Decompression
	size_t src_ptr = 0;
	size_t dst_ptr = 0;
	u8 flags = 0;
	u32 bit_count = 0;

	while (src_ptr < src_size && dst_ptr < dst_size)
	{
		if (bit_count == 0)
		{
			// Finished reading 8-bits
			flags = src[src_ptr++];
			bit_count = 8;
		}

		if (flags & 1)
		{
			// Non-compressed data
			dst[dst_ptr] = dictionary[dic_ptr] = src[src_ptr];

			dst_ptr++;
			src_ptr++;
			dic_ptr++;

			dic_ptr &= (dic_size - 1);
		}
		else
		{
			// Compressed data
			const u8 low = src[src_ptr++];
			const u8 high = src[src_ptr++];

			size_t back = ((high & 0xF0) << 4) | low;
			size_t length = (high & 0x0F) + length_offset;

			if ((dst_ptr + length) > dst_size)
			{
				// Exceeds the output buffer
				length = (dst_size - dst_ptr);
			}

			for (size_t j = 0; j < length; j++)
			{
				dst[dst_ptr] = dictionary[dic_ptr] = dictionary[back];

				dst_ptr++;
				dic_ptr++;
				back++;

				dic_ptr &= (dic_size - 1);
				back &= (dic_size - 1);
			}
		}

		flags >>= 1;
		bit_count--;


/*
		for( DWORD i = 0 ; (i < 8) && (dwSrcPtr < dwSrcSize) && (dwDstPtr < dwDstSize) ; i++ )
		{
			if( btFlags & 1 )
			{
				// Non-compressed data

				pbtDst[dwDstPtr] = clmbtDic[dwDicPtr] = pbtSrc[dwSrcPtr];

				dwDstPtr++;
				dwSrcPtr++;
				dwDicPtr++;

				dwDicPtr &= (dwDicSize - 1);
			}
			else
			{
				// Compressed data

				BYTE  btLow = pbtSrc[dwSrcPtr++];
				BYTE  btHigh = pbtSrc[dwSrcPtr++];

				DWORD dwBack = (((btHigh & 0xF0) << 4) | btLow);
				DWORD dwLength = ((btHigh & 0x0F) + dwOffset);

				if( (dwDstPtr + dwLength) > dwDstSize )
				{
					// Exceeds the output buffer

					dwLength = (dwDstSize - dwDstPtr);
				}

				for( DWORD j = 0 ; j < dwLength ; j++ )
				{
					pbtDst[dwDstPtr] = clmbtDic[dwDicPtr] = clmbtDic[dwBack];

					dwDstPtr++;
					dwDicPtr++;
					dwBack++;

					dwDicPtr &= (dwDicSize - 1);
					dwBack &= (dwDicSize - 1);
				}
			}

			btFlags >>= 1;
		}*/
	}

	return true;
}
