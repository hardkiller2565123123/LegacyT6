#include "StdInc.h"

void bdProfile::HandleMessage(int type, const char* buf, int len)
{
	bdByteBuffer data((char*)buf, len);
	char subtype = 0;
	data.readByte(&subtype);

	Log::Debug("bdProfile", "call %i", subtype);
	switch (subtype)
	{
	case 1:
		bdProfile::DownloadCallingCardsofFriends(data);
		//retreive with uint64 PMIDs
		break;
	case 3:
		bdProfile::SaveCallingCard(data);
		//upload calling card data
		break;
	default:
		Log::Debug("bdProfile", "call %i", subtype);
		break;
	}
}

void bdProfile::SaveCallingCard(bdByteBuffer& data)
{
	int unknown;
	char* blob;
	int filelen;
	data.readInt32(&unknown);
	data.readBlob(&blob, &filelen);

	PMID myPmID;
	PM_GetPMID(&myPmID);

	//upload to PMServer and a database

	PMAsync<PMWriteUserFileResult>* async = PM_WriteUserFile("t6pub", myPmID, (uint8_t*)blob, filelen);


	Log::Debug("bdProfile", "sent");
}

void bdProfile::DownloadCallingCardsofFriends(bdByteBuffer& data)
{
	PMID friends[1024];
	int count = 0;
	while (data.peekbyte() == 10)
	{
		data.readUInt64(&friends[count]);

		Log::Debug("dwprofiles", "%llu", friends[count]);
		count++;
	}

	PMPT5ProfileData result;

	PMAsync<PMGetPT5ProfileDataResult> *async = PM_GetPT5ProfileData(count, friends, &result);

	async->Wait();

	PMGetPT5ProfileDataResult* profiles = async->GetResult();

	dwMessage reply(1, false);
	reply.byteBuffer.writeUInt64(0x8000000000000001);
	reply.byteBuffer.writeUInt32(0);
	reply.byteBuffer.writeByte(8);
	reply.byteBuffer.writeUInt32(profiles->numResults);
	reply.byteBuffer.writeUInt32(profiles->numResults);

	for (int i = 0; i<profiles->numResults; i++)
	{
		if (profiles->results[i].data != NULL)
		{
			reply.byteBuffer.writeUInt64(profiles->results[i].PMID);
			reply.byteBuffer.writeBlob(profiles->results[i].data, strlen(profiles->results[i].data));
		}
		else
		{
			Log::Debug("dwprofiles", "profiledata for user %llu was null", profiles->results[i].PMID);
		}
	}


	reply.send(true);
}