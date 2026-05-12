#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "STDInc.h"

#include <fstream>

bool dw_Handler::queuedPacketHere = false;
std::queue<char> dw_Handler::packetQueue;
CRITICAL_SECTION dw_Handler::packetCS;
std::queue<std::string> dw_Handler::incomingQueue;
CRITICAL_SECTION dw_Handler::incomingCS;

static void DWPrint(const char* text)
{
	if (!text)
		return;

	printf("%s\n", text);
	OutputDebugStringA(text);
	OutputDebugStringA("\n");
}

DWORD WINAPI dw_Handler::dw_thread(LPVOID param)
{
	while (true)
	{
		Sleep(1);

		if (dw_Handler::incomingQueue.empty())
			continue;

		EnterCriticalSection(&dw_Handler::incomingCS);

		if (dw_Handler::incomingQueue.empty())
		{
			LeaveCriticalSection(&dw_Handler::incomingCS);
			continue;
		}

		std::string packet = dw_Handler::incomingQueue.front();
		dw_Handler::incomingQueue.pop();

		LeaveCriticalSection(&dw_Handler::incomingCS);

		const char* buf = packet.c_str();
		int buflen = (int)packet.size();
		int pos = 0;

		while (pos + 4 <= buflen)
		{
			int totalBytes = *(int*)(buf + pos);

			printf(
				"[DW THREAD] frame pos=%d totalBytes=%d buflen=%d\n",
				pos,
				totalBytes,
				buflen);

			if (totalBytes == 0xC8)
			{
				DWPrint("[DW THREAD] Got C8 auth seed packet");

				char replyPacket[15] =
				{
					0x0B, 0x00, 0x00, 0x00,
					0x00, 0x04, 0x0A,
					(char)0x9D, 0x28, 0x25,
					(char)0xCD, (char)0x9B,
					(char)0xC9, (char)0x81, 0x64
				};

				dw_Handler::dw_queue_packet(replyPacket, sizeof(replyPacket));
				break;
			}

			if (totalBytes <= 0)
			{
				DWPrint("[DW THREAD] heartbeat frame");

				char emptyPacket[4] = { 0, 0, 0, 0 };
				dw_Handler::dw_queue_packet(emptyPacket, sizeof(emptyPacket));
				break;
			}

			if (pos + 4 + totalBytes > buflen)
			{
				printf(
					"[DW THREAD] bad frame size pos=%d totalBytes=%d buflen=%d\n",
					pos,
					totalBytes,
					buflen);

				break;
			}

			pos += 4;

			printf(
				"[DW THREAD] packet header=0x%02X totalBytes=%d\n",
				(unsigned char)buf[pos],
				totalBytes);

			dw_Handler::dw_handle_message(buf + pos, totalBytes);

			pos += totalBytes;
		}
	}

	return 0;
}

void dw_Handler::UnknownMessageInit(bdByteBuffer& data)
{
}

void dw_Handler::UnknownMessage(int type, const char* buf, int len)
{
	bdByteBuffer data((char*)buf, len);

	char subtype = 0;
	data.readByte(&subtype);

	Log::Debug("bdUnknown", hString::va("%d subtype", subtype));

	printf(
		"[DW UNKNOWN] type=%d subtype=%d len=%d\n",
		type,
		subtype,
		len);

	dw_Handler::UnknownMessageInit(data);
}

void dw_Handler::dw_handle_message(const char* buf, int len)
{
	BYTE ptype = 0xFF;
	bool encrypted = false;

	if (!buf || len <= 0)
	{
		DWPrint("[DW HANDLE] invalid buffer");
		return;
	}

	printf(
		"[DW RAW] first=%02X second=%02X third=%02X len=%d\n",
		(unsigned char)buf[0],
		len > 1 ? (unsigned char)buf[1] : 0,
		len > 2 ? (unsigned char)buf[2] : 0,
		len);

	if ((unsigned char)buf[0] == 1)
	{
		encrypted = true;

		DWPrint("[DW HANDLE] encrypted packet");

		buf++;
		len--;

		if (len < 4)
		{
			DWPrint("[DW HANDLE] invalid encrypted header");
			return;
		}

		unsigned int ivSeed = *(unsigned int*)buf;

		printf("[DW HANDLE] ivSeed=0x%08X\n", ivSeed);

		buf += 4;
		len -= 4;

		if (len <= 4)
		{
			DWPrint("[DW HANDLE] encrypted body too small");
			return;
		}

		BYTE iv[24] = { 0 };
		BYTE key[24] = { 0 };
		static char decryptedData[256 * 1024] = { 0 };

		memset(decryptedData, 0, sizeof(decryptedData));

		dw_Crypto::dw_calculate_iv(ivSeed, iv);
		dw_Crypto::dw_get_global_key(key);

		printf(
			"[DW HANDLE] decrypting len=%d key0=%02X\n",
			len,
			key[0]);

		dw_Crypto::dw_decrypt_data(
			buf,
			iv,
			key,
			decryptedData,
			len);

		buf = decryptedData;

		printf(
			"[DW DECRYPTED] first=%02X second=%02X third=%02X fourth=%02X len=%d\n",
			(unsigned char)buf[0],
			(unsigned char)buf[1],
			(unsigned char)buf[2],
			(unsigned char)buf[3],
			len);

		buf += 4;
		len -= 4;
	}
	else
	{
		DWPrint("[DW HANDLE] non-encrypted packet");

		buf++;
		len--;
	}

	if (len <= 0)
	{
		DWPrint("[DW HANDLE] empty after header");
		return;
	}

	ptype = (BYTE)buf[0];

	printf(
		"[DW HANDLE] ptype=%i encrypted=%s remainingLen=%d\n",
		ptype,
		encrypted ? "true" : "false",
		len);

	buf++;
	len--;

	dw_Handler::queuedPacketHere = false;

	Log::Error(
		"DW stuff",
		"ptype = %i encrypted = %s len = %i",
		ptype,
		encrypted ? "true" : "false",
		len);

	if (encrypted)
	{
		if (ptype == 10)
		{
			DWPrint("[DW HANDLE] dispatch -> Storage");
			Storage::dw_handle_storage_message(ptype, buf, len);
		}
		else if (ptype == 12 || ptype == 27 || ptype == 6)
		{
			DWPrint("[DW HANDLE] dispatch -> TitleUtils");
			TitleUtils::dw_handle_tutils_message(ptype, buf, len);
		}
		else if (ptype == 21)
		{
			DWPrint("[DW HANDLE] dispatch -> Match");
			dw_match::match_packet_received(ptype, buf, len);
		}
		else if (ptype == 8)
		{
			DWPrint("[DW HANDLE] dispatch -> Profile");
			bdProfile::HandleMessage(ptype, buf, len);
		}
		else if (ptype == 8)
		{
			DWPrint("[DW HANDLE] dispatch -> Profile");
			bdProfile::HandleMessage(ptype, buf, len);
		}
		else if (ptype == 81)
		{
			printf("[DW HANDLE] faking success for ptype 81\n");

			dwMessage reply(1, true);

			reply.byteBuffer.writeUInt64(0x8000000000000001);
			reply.byteBuffer.writeUInt32(700);
			reply.byteBuffer.writeUInt32(0);
			reply.byteBuffer.writeUInt32(0);

			reply.send(true);
		}
		else if (ptype == 68)
		{
			printf("[DW HANDLE] faking success for ptype 68\n");

			dwMessage reply(1, true);

			reply.byteBuffer.writeUInt64(0x8000000000000001);
			reply.byteBuffer.writeUInt32(700);
			reply.byteBuffer.writeUInt32(0);
			reply.byteBuffer.writeUInt32(0);

			reply.send(true);
		}
		else if (ptype == 23 || ptype == 28)
		{
			printf(
				"[DW HANDLE] isolating online service ptype=%i\n",
				ptype);

			dw_Handler::queuedPacketHere = true;
		}
		if (!dw_Handler::queuedPacketHere)
		{
			DWPrint("[DW HANDLE] sending generic encrypted ACK");

			dwMessage reply(1, false);

			reply.byteBuffer.writeUInt64(0x8000000000000001);
			reply.byteBuffer.writeUInt32(0);
			reply.byteBuffer.writeUInt32(0);
			reply.byteBuffer.writeUInt32(0);

			reply.send(true);
		}
	}
	else
	{
		if (ptype == 28 || ptype == 12 || ptype == 26)
		{
			DWPrint("[DW HANDLE] dispatch -> Auth");
			dw_Auth::dw_handle_auth_message(ptype, buf, len);
		}
		else if (ptype == 7)
		{
			DWPrint("[DW HANDLE] dispatch -> Lobby");
			dw_Auth::dw_handle_lobby_message(ptype, buf, len);
		}
		else
		{
			printf("[DW HANDLE] unknown non-encrypted ptype=%i\n", ptype);

			std::ofstream fp;
			fp.open(hString::va("not encrypted %d", ptype), std::ios::out | std::ios::binary);
			fp.write((char*)buf, len);
		}
	}
}

void dw_Handler::dw_queue_packet(char* buf, int len)
{
	if (!buf || len <= 0)
		return;

	EnterCriticalSection(&dw_Handler::packetCS);

	for (int i = 0; i < len; i++)
		dw_Handler::packetQueue.push(buf[i]);

	LeaveCriticalSection(&dw_Handler::packetCS);

	dw_Handler::queuedPacketHere = true;

	printf("[DW QUEUE] queued len=%d\n", len);
}

int dw_Handler::dw_dequeue_packet(char* buf, int len)
{
	if (!buf || len <= 0)
		return 0;

	if (dw_Handler::packetQueue.empty())
		return 0;

	EnterCriticalSection(&dw_Handler::packetCS);

	int toPop = min((int)dw_Handler::packetQueue.size(), len);

	for (int i = 0; i < toPop; i++)
	{
		buf[i] = dw_Handler::packetQueue.front();
		dw_Handler::packetQueue.pop();
	}

	LeaveCriticalSection(&dw_Handler::packetCS);

	printf("[DW QUEUE] dequeued len=%d\n", toPop);

	return toPop;
}

bool dw_Handler::dw_packet_available()
{
	return !dw_Handler::packetQueue.empty();
}

void dw_Handler::dw_im_received(PMID from, const uint8_t* buf, uint32_t buflen)
{
	Log::Debug("dwhandler", "received IM from %llx", from);

	printf(
		"[DW IM] from=%llx len=%u\n",
		from,
		buflen);

	dwMessage msg(2, false);

	msg.byteBuffer.writeUInt32(21);
	msg.byteBuffer.writeUInt64(from);
	msg.byteBuffer.writeString("me");
	msg.byteBuffer.writeBlob((const char*)buf, buflen);

	msg.send(true);
}

void dw_Handler::dw_handle_packet(const char* buf, int buflen)
{
	if (!buf || buflen <= 0)
		return;

	printf("[DW HANDLE PACKET] incoming len=%d\n", buflen);

	EnterCriticalSection(&dw_Handler::incomingCS);

	std::string packet(buf, buflen);
	dw_Handler::incomingQueue.push(packet);

	LeaveCriticalSection(&dw_Handler::incomingCS);
}

void dw_Handler::dw_build_lsg_ticket(char* buf, char* key)
{
	if (!buf || !key)
		return;

	memset(buf, 0, 128);
	memcpy(buf, key, 24);
}

void dw_Handler::dw_build_game_ticket(char* buf, char* key, int gameID)
{
	if (!buf || !key)
		return;

	dw_game_ticket* ticket = (dw_game_ticket*)buf;

	memset(ticket, 0x0A, sizeof(dw_game_ticket));

	ticket->signature[0] = (char)0xDE;
	ticket->signature[1] = (char)0xAD;
	ticket->signature[2] = (char)0xBD;
	ticket->signature[3] = (char)0xEF;

	ticket->licenseType = 0;
	ticket->gameID = gameID;
	ticket->userID = 0x1100001DEADC0DE;

	ticket->nick[0] = 'a';
	ticket->nick[1] = 0;

	memcpy(ticket->key, key, sizeof(ticket->key));
}