#include <windows.h>
#include <string>

class FileMapping {
private:
	const DWORD BUF_SIZE = (1 << (sizeof(short) * 8)) - 1;
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
		std::string szName(name.begin(), name.end());
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

	// INPUT msg : send data ptr
	// INPUT len : msg size (Bytes)
	// INPUT isBlocking : Block process until send success when it's true
	void SendMsg(const void* msg, size_t len, bool isBlocking) const {
		if (hMapFile == NULL) throw ER_CREATE_MAPPING_FAIL;
		if (pBuf == NULL) throw ER_ACCESS_MAPPING_FAIL;
		if (len > BUF_SIZE) throw ER_SIZE_OUT_OF_RANGE;
		CopyMemory((BYTE*)pBuf, &len, sizeof(short));
		CopyMemory((BYTE*)pBuf + sizeof(short), msg, len);
		if (isBlocking) {
			while (*(BYTE*)pBuf) { // Block process until msg size = 0
				Sleep(10);
			}
		}
	}

	// INPUT msg : recv container ptr
	// INPUT len : msg max size (Bytes)
	// INPUT isBlocking : Block process until recv success when it's true
	// OUTPUT return : recv size (Bytes)
	int RecvMsg(void* msg, size_t len, bool isBlocking) {
		if (hMapFile == NULL) throw ER_CREATE_MAPPING_FAIL;
		if (pBuf == NULL) throw ER_ACCESS_MAPPING_FAIL;
		if (isBlocking) {
			while (!*(BYTE*)pBuf) {  // Block process until msg size > 0
				Sleep(10);
			}
		}
		if (*(BYTE*)pBuf > len) throw ER_SIZE_NOT_ENOUGH;
		unsigned short get_size;
		CopyMemory(&get_size, (BYTE*)pBuf, sizeof(short));
		CopyMemory(msg, (BYTE*)pBuf + sizeof(short), get_size);
		memset((BYTE*)pBuf, 0, sizeof(short));
		Sleep(10);
		return get_size;
	}
};