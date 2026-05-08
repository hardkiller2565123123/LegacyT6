// ==========================================================
// PlusMW3 project
// 
// Component: xPM
// Sub-component: libPM
// Purpose: definitions for the PM storage service
//
// Initial author: NTAuthority
// Started: 2011-07-17
// ==========================================================

enum OverWatchStatus
{
	Clean = 0,
	OverWatch = 1
};

enum PMGVUResult
{
	OK = 0,
	NotAllowed = 1
};

class PMGVUploadResultMessage
{
public:
	PMGVUResult result;
};

class PMGSResult
{
public:
	OverWatchStatus result;
};

enum EGetFileResult
{
	GetFileResultOK = 0,
	GetFileResultNotFound = 1,
	GetFileResultNotAllowed = 2,
	GetFileResultServiceError = 3
};

enum EWriteFileResult
{
	WriteFileResultOK = 0,
	WriteFileResultNotAllowed = 1,
	WriteFileResultServiceError = 2
};

class PMGetPublisherFileResult
{
public:
	// the request result
	EGetFileResult result;

	// the amount of bytes written to the buffer
	uint32_t fileSize;

	// the buffer passed to PM_NP_GetPublisherFile()
	uint8_t* buffer;
};

class PMGetUserFileResult
{
public:
	// the request result
	EGetFileResult result;

	// the amount of bytes written to the buffer
	uint32_t fileSize;

	// the buffer passed to PM_NP_GetUserFileFromStorage()
	uint8_t* buffer;
};

class PMWriteUserFileResult
{
public:
	// the request result
	EWriteFileResult result;
};