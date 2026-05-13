#include "STDInc.h"
#include <fstream>
#include "DebugConsole.h"
#include "STDPatches.h"


void Storage::dw_storage_get_publisher_file(bdByteBuffer& data)
{
	char Filename[512] = { 0 };
	data.readString(Filename, sizeof(Filename));

	static uint8_t fileBuffer[131072] = { 0 };

	printf("[DW STORAGE] requested publisher file: %s\n", Filename);
	Log::Debug("dwstorage", "fetching publisher file %s", Filename);

	if (strstr(Filename, "ffotd") && strstr(Filename, "_mp"))
	{
		strcpy_s(Filename, sizeof(Filename), "ffotd_tu17_mp");
	}
	else if (strstr(Filename, "online") && strstr(Filename, "_mp"))
	{
		strcpy_s(Filename, sizeof(Filename), "online_tu17_mp.wad");
	}
	else if (strstr(Filename, "ffotd") && strstr(Filename, "_zm"))
	{
		strcpy_s(Filename, sizeof(Filename), "ffotd_tu17_zm");
	}
	else if (strstr(Filename, "online") && strstr(Filename, "_zm"))
	{
		strcpy_s(Filename, sizeof(Filename), "online_tu17_zm.wad");
	}

	printf("[DW STORAGE] normalized publisher file: %s\n", Filename);

	PMAsync<PMGetPublisherFileResult>* async =
		xNP_GetPublisherFile(Filename, fileBuffer, sizeof(fileBuffer));

	PMGetPublisherFileResult* result = async->Wait();

	printf("[DW STORAGE] publisher result=%d size=%d\n", result->result, result->fileSize);
	Log::Debug("dwstorage", "result %d, size %d", result->result, result->fileSize);

	if (result->result == GetFileResultOK)
	{
		dwMessage reply(1, false);
		reply.byteBuffer.writeUInt64(0x8000000000000001);
		reply.byteBuffer.writeUInt32(0);
		reply.byteBuffer.writeByte(7);
		reply.byteBuffer.writeUInt32(1);
		reply.byteBuffer.writeUInt32(1);
		reply.byteBuffer.writeBlob((char*)result->buffer, result->fileSize);
		reply.send(true);
	}
	else
	{
		dwMessage reply(1, false);
		reply.byteBuffer.writeUInt64(0x8000000000000001);
		reply.byteBuffer.writeUInt32(0x3E8);
		reply.send(true);
	}

	Log::Debug("bdstorage", "Done.");
}

void Storage::dw_storage_get_user_file(bdByteBuffer& data)
{
	char filename[512] = { 0 };
	data.readString(filename, sizeof(filename));

	static uint8_t fileBuffer[131072] = { 0 };

	printf("[DW STORAGE] requested user file: %s\n", filename);
	Log::Debug("dwstorage", "fetching user file %s", filename);

	xNPID myxNPID;
	xNP_GetxNPID(&myxNPID);

	PMAsync<PMGetUserFileResult>* async =
		xNP_GetUserFile(filename, myxNPID, fileBuffer, sizeof(fileBuffer));

	PMGetUserFileResult* result = async->Wait();

	printf("[DW STORAGE] user result=%d size=%d\n", result->result, result->fileSize);
	Log::Debug("dwstorage", "result %d, size %d", result->result, result->fileSize);

	if (result->result == GetFileResultOK)
	{
		dwMessage reply(1, false);
		reply.byteBuffer.writeUInt64(0x8000000000000001);
		reply.byteBuffer.writeUInt32(0);
		reply.byteBuffer.writeByte(7);
		reply.byteBuffer.writeUInt32(1);
		reply.byteBuffer.writeUInt32(1);
		reply.byteBuffer.writeBlob((char*)result->buffer, result->fileSize);
		reply.send(true);
	}
	else if (result->result == GetFileResultNotFound)
	{
		dwMessage reply(1, false);
		reply.byteBuffer.writeUInt64(0x8000000000000001);
		reply.byteBuffer.writeUInt32(0x3E8);
		reply.send(true);
	}
	else
	{
		dwMessage reply(1, false);
		reply.byteBuffer.writeUInt64(0x8000000000000001);
		reply.byteBuffer.writeUInt32(2);
		reply.send(true);
	}
}

void Storage::dw_storage_upload_user_file(bdByteBuffer& data)
{
	char filename[512] = { 0 };
	bool stuff = false;
	char* filedata = NULL;
	int filelen = 0;

	data.readString(filename, sizeof(filename));
	data.readBoolean(&stuff);
	data.readBlob(&filedata, &filelen);

	printf("[DW STORAGE] upload user file: %s size=%d\n", filename, filelen);
	Log::Debug("dwstorage", "writing user file %s", filename);

	xNPID myxNPID;
	xNP_GetxNPID(&myxNPID);

	if (filedata && filelen > 0)
	{
		xNP_WriteUserFile(filename, myxNPID, (uint8_t*)filedata, filelen);
	}

	dwMessage reply(1, false);
	reply.byteBuffer.writeUInt64(0x8000000000000001);
	reply.byteBuffer.writeUInt32(0);
	reply.byteBuffer.writeByte(1);
	reply.byteBuffer.writeUInt32(0);
	reply.byteBuffer.writeUInt32(0);
	reply.send(true);
}

void Storage::dw_handle_storage_message(int type, const char* buf, int len)
{

	LOG_SCOPE();

	DebugConsole::Info("Storage", "type=%d len=%d", type, len);
	bdByteBuffer data((char*)buf, len);

	char subtype = 0;
	data.readByte(&subtype);

	printf("[DW STORAGE] type=%d subtype=%d len=%d\n", type, subtype, len);
	Log::Debug("dwstorage", "call %i", subtype);

	switch (subtype)
	{
	case 1:
		dw_storage_upload_user_file(data);
		break;

	case 3:
		dw_storage_get_user_file(data);
		break;

	case 7:
		dw_storage_get_publisher_file(data);
		break;

	default:
		Log::Debug("dwstorage", "call %i", subtype);
		break;
	}
}