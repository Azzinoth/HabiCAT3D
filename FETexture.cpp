#include "FETexture.h"
using namespace FocalEngine;

FETexture::FETexture(const std::string Name) : FEObject(FE_TEXTURE, Name)
{
	this->Name = Name;
	GetNewGlTextureID();
}

FETexture::FETexture(const int Width, const int Height, const std::string Name) : FEObject(FE_TEXTURE, Name)
{
	this->Name = Name;
	this->Width = Width;
	this->Height = Height;
	GetNewGlTextureID();
	Bind(0);
	FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	UnBind();
}

FETexture::FETexture(const GLint InternalFormat, const GLenum Format, const int Width, const int Height, const std::string Name) : FEObject(FE_TEXTURE, Name)
{
	this->Width = Width;
	this->Height = Height;
	this->InternalFormat = InternalFormat;
	this->Format = Format;
	GetNewGlTextureID();
	Bind(0);
	FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, Format, GL_UNSIGNED_BYTE, nullptr);
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	// to-do: it is needed for screen space effects but could interfere with other purposes
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	if (InternalFormat == GL_RGBA16F || InternalFormat == GL_RGB16F || InternalFormat == GL_RGB32F || InternalFormat == GL_RGBA32F)
		bHDR = true;
}

void FETexture::GetNewGlTextureID()
{
	FE_GL_ERROR(glGenTextures(1, &TextureID));
	//LOG.add("Texture creation with textureID: " + std::to_string(textureID));
}

FETexture::~FETexture()
{
	for (size_t i = 0; i < NoDeletingList.size(); i++)
	{
		if (NoDeletingList[i] == TextureID)
			return;
	}
	//LOG.add("Texture deletion with textureID: " + std::to_string(textureID));
	FE_GL_ERROR(glDeleteTextures(1, &TextureID));
}

GLuint FETexture::GetTextureID()
{
	return TextureID;
}

void FETexture::Bind(const unsigned int TextureUnit)
{
	DefaultTextureUnit = TextureUnit;
	FE_GL_ERROR(glActiveTexture(GL_TEXTURE0 + TextureUnit));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, TextureID));
}

void FETexture::UnBind()
{
	if (DefaultTextureUnit != -1)
	{
		FE_GL_ERROR(glActiveTexture(GL_TEXTURE0 + DefaultTextureUnit));
		FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, 0));
	}
}

std::string FETexture::GetFileName()
{
	return FileName;
}

GLint FETexture::GetInternalFormat()
{
	return InternalFormat;
}

int FETexture::GetWidth()
{
	return Width;
}

int FETexture::GetHeight()
{
	return Height;
}

void FETexture::GPUAllocateTeture(const GLenum Target, const GLint Level, const GLint Internalformat, const GLsizei Width, const GLsizei Height, const GLint Border, const GLenum Format, const GLenum Type, const void* Data)
{
	FE_GL_ERROR(glTexImage2D(Target, Level, Internalformat, Width, Height, Border, Format, Type, Data));
#ifdef FE_GPUMEM_ALLOCATION_LOGING
	FELOG::getInstance().logError("Texture creation with width: " + std::to_string(width) + " height: " + std::to_string(height));
#endif
}

void FETexture::AddToOnDeleteCallBackList(const std::string ObjectID)
{
	CallListOnDeleteFEObject.push_back(ObjectID);
}

void FETexture::EraseFromOnDeleteCallBackList(const std::string ObjectID)
{
	for (size_t i = 0; i < CallListOnDeleteFEObject.size(); i++)
	{
		if (CallListOnDeleteFEObject[i] == ObjectID)
		{
			CallListOnDeleteFEObject.erase(CallListOnDeleteFEObject.begin() + i, CallListOnDeleteFEObject.begin() + i + 1);
			break;
		}
	}
}

std::string FETexture::TextureInternalFormatToString(const GLint InternalFormat)
{
	std::string result;

	if (InternalFormat == GL_RGBA)
	{
		result += "GL_RGBA";
	}
	else if (InternalFormat == GL_RED)
	{
		result += "GL_RED";
	}
	else if (InternalFormat == GL_R16)
	{
		result += "GL_R16";
	}
	else if (InternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
	{
		result += "GL_COMPRESSED_RGBA_S3TC_DXT5_EXT";
	}
	else if (InternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		result += "GL_COMPRESSED_RGBA_S3TC_DXT1_EXT";
	}
	
	return result;
}

std::vector<GLuint> FETexture::NoDeletingList = std::vector<GLuint>();
void FETexture::AddToNoDeletingList(const GLuint TextureID)
{
	NoDeletingList.push_back(TextureID);
}

unsigned char* FETexture::GetRawData(size_t* RawDataSize)
{
	unsigned char* result = nullptr;
	if (RawDataSize != nullptr)
		*RawDataSize = 0;

	if (InternalFormat != GL_RGBA &&
		InternalFormat != GL_RED &&
		InternalFormat != GL_R16 &&
		InternalFormat != GL_COMPRESSED_RGBA_S3TC_DXT5_EXT &&
		InternalFormat != GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		LOG.Add("FETexture::getRawData internalFormat is not supported", "FE_LOG_SAVING", FE_LOG_ERROR);
		return result;
	}

	FE_GL_ERROR(glActiveTexture(GL_TEXTURE0));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, TextureID));

	if (InternalFormat == GL_R16)
	{
		if (RawDataSize != nullptr)
			*RawDataSize = GetWidth() * GetHeight() * 2;
		result = new unsigned char[GetWidth() * GetHeight() * 2];
		glPixelStorei(GL_PACK_ALIGNMENT, 2);
		FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_SHORT, result));
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
	}
	else if (InternalFormat == GL_RED)
	{
		if (RawDataSize != nullptr)
			*RawDataSize = GetWidth() * GetHeight();
		result = new unsigned char[GetWidth() * GetHeight()];
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, result));
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
	}
	else
	{
		if (RawDataSize != nullptr)
			*RawDataSize = GetWidth() * GetHeight() * 4;
		result = new unsigned char[GetWidth() * GetHeight() * 4];
		FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, result));
	}

	return result;
}

void FETexture::UpdateRawData(unsigned char* NewRawData, const size_t MipCount)
{
	if (InternalFormat != GL_RGBA &&
		InternalFormat != GL_RED &&
		InternalFormat != GL_R16 &&
		InternalFormat != GL_COMPRESSED_RGBA_S3TC_DXT5_EXT &&
		InternalFormat != GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		LOG.Add("FETexture::updateRawData internalFormat of texture is not supported", "FE_LOG_SAVING", FE_LOG_ERROR);
		return;
	}

	if (InternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT || InternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		InternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		for (size_t i = 3; i < static_cast<size_t>(GetWidth() * GetHeight() * 4); i += 4)
		{
			if (NewRawData[i] != 255)
			{
				InternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				break;
			}
		}
	}

	FE_GL_ERROR(glDeleteTextures(1, &TextureID));
	FE_GL_ERROR(glGenTextures(1, &TextureID));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, TextureID));

	if (InternalFormat == GL_RGBA)
	{
		FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, static_cast<int>(MipCount), GL_RGBA8, GetWidth(), GetHeight()));
		FE_GL_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GetWidth(), GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, (void*)(NewRawData)));
	}
	else if (InternalFormat == GL_RED)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, static_cast<int>(MipCount), GL_R8, GetWidth(), GetHeight()));
		FE_GL_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GetWidth(), GetHeight(), GL_RED, GL_UNSIGNED_BYTE, (void*)(NewRawData)));
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	}
	else if (InternalFormat == GL_R16)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
		FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, static_cast<int>(MipCount), GL_R16, GetWidth(), GetHeight()));
		FE_GL_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GetWidth(), GetHeight(), GL_RED, GL_UNSIGNED_SHORT, (void*)(NewRawData)));
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	}
	else
	{
		FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, static_cast<int>(MipCount), InternalFormat, GetWidth(), GetHeight()));
		FE_GL_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GetWidth(), GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, (void*)(NewRawData)));
	}
}

FETexture* FETexture::LoadPNGTexture(const char* FileName, std::string Name)
{
	std::vector<unsigned char> RawData;
	unsigned UWidth, UHeight;

	lodepng::decode(RawData, UWidth, UHeight, FileName);
	if (RawData.empty())
	{
		LOG.Add(std::string("can't load file: ") + FileName + " in function FEResourceManager::LoadPNGTexture.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

	bool bUsingAlpha = false;
	for (size_t i = 3; i < RawData.size(); i += 4)
	{
		if (RawData[i] != 255)
		{
			bUsingAlpha = true;
			break;
		}
	}

	FETexture* NewTexture = new FETexture(Name);
	NewTexture->Width = UWidth;
	NewTexture->Height = UHeight;

	const int InternalFormat = bUsingAlpha ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;

	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, NewTexture->TextureID));
	FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, InternalFormat, NewTexture->Width, NewTexture->Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, RawData.data());
	NewTexture->InternalFormat = InternalFormat;

	if (NewTexture->MipEnabled)
	{
		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f)); // to-do: fix this
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));
	}

	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	if (NewTexture->MagFilter == FE_LINEAR)
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
	else
	{
		FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	}
	NewTexture->FileName = FileName;

	if (Name.empty())
	{
		const std::string FilePath = NewTexture->FileName;
		std::size_t index = FilePath.find_last_of("/\\");
		const std::string NewFileName = FilePath.substr(index + 1);
		index = NewFileName.find_last_of(".");
		const std::string FileNameWithOutExtention = NewFileName.substr(0, index);
		NewTexture->SetName(FileNameWithOutExtention);
	}

	return NewTexture;
}