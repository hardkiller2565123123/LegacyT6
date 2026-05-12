#include "STDInc.h"

static void AuthDebug(const char* fmt, ...)
{
	char buffer[1024] = { 0 };

	va_list args;
	va_start(args, fmt);
	_vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, fmt, args);
	va_end(args);

	Log::Debug("dwauth", "%s", buffer);
}

static unsigned int ClampSize(unsigned int value, unsigned int maxValue)
{
	return (value > maxValue) ? maxValue : value;
}

static void BuildEncryptedTickets(
	unsigned int gameID,
	const BYTE* encryptionKey,
	char* encryptedGameTicket,
	size_t encryptedGameTicketSize,
	char* lsgTicket,
	size_t lsgTicketSize)
{
	BYTE iv[24] = { 0 };
	char gameTicket[128] = { 0 };

	memset(encryptedGameTicket, 0, encryptedGameTicketSize);
	memset(lsgTicket, 0, lsgTicketSize);

	dw_Crypto::dw_calculate_iv(0xDEADC0DE, iv);

	dw_Handler::dw_build_game_ticket(gameTicket, GLOBAL_KEY, gameID);
	dw_Handler::dw_build_lsg_ticket(lsgTicket, GLOBAL_KEY);

	dw_Crypto::dw_encrypt_data(
		gameTicket,
		iv,
		(BYTE*)encryptionKey,
		encryptedGameTicket,
		(int)encryptedGameTicketSize);
}

static void SendAuthReply(
	int replyType,
	unsigned int resultCode,
	unsigned int ivSeed,
	const char* encryptedGameTicket,
	const char* lsgTicket)
{
	dwMessage reply(replyType, true);

	reply.bitBuffer.setUseDataTypes(false);
	reply.bitBuffer.writeBoolean(false);
	reply.bitBuffer.writeUInt32(resultCode);
	reply.bitBuffer.writeUInt32(ivSeed);
	reply.bitBuffer.writeBytes(128, (BYTE*)encryptedGameTicket);
	reply.bitBuffer.writeBytes(128, (BYTE*)lsgTicket);

	reply.send(false);
}

void dw_Auth::dw_handle_auth_message_server(const char* buf, int len)
{
	if (!buf || len <= 0)
		return;

	bool unknownBool = false;
	unsigned int randomNumber = 0;
	unsigned int gameID = 0;

	char keyBase[8] = { 0 };
	char lsgTicket[128] = { 0 };
	char encryptedGameTicket[128] = { 0 };

	bdBitBuffer data((char*)buf, len);

	data.setUseDataTypes(false);
	data.readBoolean(&unknownBool);

	data.setUseDataTypes(true);
	data.readUInt32(&randomNumber);
	data.readUInt32(&gameID);
	data.read(64, keyBase);

	AuthDebug(
		"got dedi auth message: randomNumber %u, gameID %u",
		randomNumber,
		gameID);

	BuildEncryptedTickets(
		gameID,
		(BYTE*)DEDI_KEY_HASH,
		encryptedGameTicket,
		sizeof(encryptedGameTicket),
		lsgTicket,
		sizeof(lsgTicket));

	unsigned int resultCode = (keyBase[0] == DEDI_KEY_HASH[0]) ? 700 : 706;

	SendAuthReply(
		13,
		resultCode,
		0xDEADC0DE,
		encryptedGameTicket,
		lsgTicket);
}

void dw_Auth::dw_handle_auth_message_steam(const char* buf, int len)
{
	if (!buf || len <= 0)
		return;

	bool unknownBool = false;
	unsigned int randomNumber = 0;
	unsigned int gameID = 0;
	unsigned int ticketLength = 0;

	char ticket[1024] = { 0 };
	char lsgTicket[128] = { 0 };
	char encryptedGameTicket[128] = { 0 };

	bdBitBuffer data((char*)buf, len);

	data.setUseDataTypes(false);
	data.readBoolean(&unknownBool);

	data.setUseDataTypes(true);
	data.readUInt32(&randomNumber);
	data.readUInt32(&gameID);
	data.readUInt32(&ticketLength);

	ticketLength = ClampSize(ticketLength, sizeof(ticket));

	if (ticketLength > 0)
		data.readBytes(ticketLength, (BYTE*)ticket);

	AuthDebug(
		"got steam auth message: randomNumber %u, gameID %u, ticketLength %u",
		randomNumber,
		gameID,
		ticketLength);

	const BYTE* encryptionKey = NULL;

	if (ticketLength >= 56)
		encryptionKey = (BYTE*)(ticket + 32);
	else
		encryptionKey = (BYTE*)GLOBAL_KEY;

	BuildEncryptedTickets(
		gameID,
		encryptionKey,
		encryptedGameTicket,
		sizeof(encryptedGameTicket),
		lsgTicket,
		sizeof(lsgTicket));

	SendAuthReply(
		29,
		700,
		0xDEADC0DE,
		encryptedGameTicket,
		lsgTicket);
}

void dw_Auth::dw_handle_auth_message_register_server(const char* buf, int len)
{
	if (!buf || len <= 0)
		return;

	bool unknownBool = false;
	unsigned int randomNumber = 0;
	unsigned int gameID = 0;

	char rsaKey[140] = { 0 };
	char lsgTicket[128] = { 0 };
	char encryptedGameTicket[128] = { 0 };

	bdBitBuffer data((char*)buf, len);

	data.setUseDataTypes(false);
	data.readBoolean(&unknownBool);

	data.setUseDataTypes(true);
	data.readUInt32(&randomNumber);
	data.readUInt32(&gameID);
	data.read(1120, rsaKey);

	AuthDebug(
		"got dedi register message: randomNumber %u, gameID %u",
		randomNumber,
		gameID);

	BuildEncryptedTickets(
		gameID,
		(BYTE*)DEDI_KEY_HASH,
		encryptedGameTicket,
		sizeof(encryptedGameTicket),
		lsgTicket,
		sizeof(lsgTicket));

	dwMessage reply(25, true);

	reply.bitBuffer.setUseDataTypes(false);
	reply.bitBuffer.writeBoolean(false);
	reply.bitBuffer.writeUInt32(700);
	reply.bitBuffer.writeUInt32(0xDEADC0DE);
	reply.bitBuffer.writeBytes(128, (BYTE*)encryptedGameTicket);
	reply.bitBuffer.writeBytes(128, (BYTE*)lsgTicket);
	reply.bitBuffer.writeBytes(86, (BYTE*)DEDI_KEY_OB);
	reply.bitBuffer.writeInt32(1234);

	reply.send(false);
}

void dw_Auth::dw_handle_auth_message(int type, const char* buf, int len)
{
	if (!buf || len <= 0)
	{
		AuthDebug("auth message ignored: invalid buffer type %i", type);
		return;
	}

	switch (type)
	{
	case 12:
		dw_Auth::dw_handle_auth_message_server(buf, len);
		break;

	case 26:
		dw_Auth::dw_handle_auth_message_register_server(buf, len);
		break;

	case 28:
		dw_Auth::dw_handle_auth_message_steam(buf, len);
		break;

	default:
		AuthDebug("unknown auth message type %i", type);
		break;
	}
}

void dw_Auth::dw_handle_lobby_message(int type, const char* buf, int len)
{
	if (type != 7)
	{
		AuthDebug("unknown lobby auth type %i", type);
		return;
	}

	if (!buf || len <= 0)
	{
		AuthDebug("lobby auth ignored: invalid buffer");
		return;
	}

	bool unknownBool = false;
	unsigned int gameID = 0;
	unsigned int randomNumber = 0;
	char ticket[128] = { 0 };

	bdBitBuffer data((char*)buf, len);

	data.setUseDataTypes(false);
	data.readBoolean(&unknownBool);

	data.setUseDataTypes(true);
	data.readUInt32(&gameID);
	data.readUInt32(&randomNumber);
	data.readBytes(128, (BYTE*)ticket);

	AuthDebug(
		"lobby auth message: gameID %u, randomNumber %u",
		gameID,
		randomNumber);

	dw_Crypto::dw_set_global_key((BYTE*)ticket);

	AuthDebug("sending simple lobby auth success");

	dwMessage reply(7, false);

	reply.byteBuffer.writeUInt32(700);
	reply.byteBuffer.writeUInt32(randomNumber);

	reply.send(false);
}