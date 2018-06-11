#include "Types.h"
std::ostream& operator<<(std::ostream& os, SyncCout sc) {
	static std::mutex m;

	if (sc == IO_LOCK)
		m.lock();

	if (sc == IO_UNLOCK)
		m.unlock();

	return os;
}