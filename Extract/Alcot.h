#pragma once

#include "ExtractBase.h"

class CAlcot final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	bool DecodeASB(CArcFile* archive);
	bool DecodeCPB(CArcFile* archive);

	void Decomp(u8* dst, size_t dst_size, const u8* src);
	void Decrypt(u8* src, size_t src_size, size_t dst_size);
};
