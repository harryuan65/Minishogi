#ifndef _FILEMAPPING_
#define _FILEMAPPING_
#include <windows.h>
#include <string>

class FileMapping {
private:
	const int BUF_SIZE = 256;
	HANDLE hMapFile = NULL;
	LPVOID pBuf = NULL;

public:
	enum {
		ER_NO_ERROR,            // 沒有錯誤
		ER_CREATE_MAPPING_FAIL, // CreateFileMapping 失敗
		ER_ACCESS_MAPPING_FAIL, // MapViewOfFile 失敗
		ER_SIZE_OUT_OF_RANGE,   // SendMsg len超過BUF_SIZE(256B)
		ER_SIZE_NOT_ENOUGH      // RecvMsg len不夠接收完整資料
	};

	FileMapping() {}

	FileMapping(const std::string name) {
		Open(name);
	}

	~FileMapping() {
		Close();
	}

	// INPUT name : name of mapping object
	void Open(const std::string name) {
		Close();
		std::wstring szName(name.begin(), name.end());
		hMapFile = CreateFileMapping(
			INVALID_HANDLE_VALUE,    // use paging file
			NULL,                    // default security
			PAGE_READWRITE,          // read/write access
			0,                       // maximum object size (high-order DWORD)
			BUF_SIZE,                // maximum object size (low-order DWORD)
			szName.c_str());         // name of mapping object
		if (hMapFile == NULL) {
			throw ER_CREATE_MAPPING_FAIL;
		}

		pBuf = MapViewOfFile(
			hMapFile,             // handle to map object
			FILE_MAP_ALL_ACCESS,  // read/write permission
			0,
			0,
			BUF_SIZE + 1);
		if (pBuf == NULL) {
			throw ER_ACCESS_MAPPING_FAIL;
		}
	}

	void Close() {
		if (pBuf != NULL) UnmapViewOfFile(pBuf);
		if (hMapFile != NULL) CloseHandle(hMapFile);
		pBuf = NULL;
		hMapFile = NULL;
	}

	bool isOpen() const {
		return pBuf && hMapFile;
	}

	// Block process until send success
	// INPUT msg : send data ptr
	// INPUT len : msg size (Bytes)
	void SendMsg(const void* msg, size_t len) const {
		if (hMapFile == NULL) throw ER_CREATE_MAPPING_FAIL;
		if (pBuf == NULL) throw ER_ACCESS_MAPPING_FAIL;
		if (len > BUF_SIZE) throw ER_SIZE_OUT_OF_RANGE;
		*(BYTE*)pBuf = len;
		CopyMemory((BYTE*)pBuf + 1, msg, len);
		while (*(BYTE*)pBuf) { // Block process until msg size = 0
			Sleep(10);
		}
	}

	// Block process until recv success
	// INPUT msg : recv container ptr
	// INPUT len : msg max size (Bytes)
	// OUTPUT return : recv size (Bytes)
	int RecvMsg(void* msg, size_t len) {
		if (hMapFile == NULL) throw ER_CREATE_MAPPING_FAIL;
		if (pBuf == NULL) throw ER_ACCESS_MAPPING_FAIL;
		while (!*(BYTE*)pBuf) {  // Block process until msg size > 0
			Sleep(10);
		}
		if (*(BYTE*)pBuf > len) throw ER_SIZE_NOT_ENOUGH;
		CopyMemory(msg, (BYTE*)pBuf + 1, *(BYTE*)pBuf);
		int get_size = *(BYTE*)pBuf;
		*(BYTE*)pBuf = 0;
		return get_size;
	}
};

#endif