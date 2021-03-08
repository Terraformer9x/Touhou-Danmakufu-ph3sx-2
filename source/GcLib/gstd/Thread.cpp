#include "source/GcLib/pch.h"

#include "Thread.hpp"
#include "GstdUtility.hpp"

using namespace gstd;

//*******************************************************************
//Thread
//*******************************************************************
Thread::Thread() {
	hThread_ = nullptr;
	idThread_ = 0;
	status_ = STOP;
}
Thread::~Thread() {
	this->Stop();
	this->Join();
	if (hThread_) {
		::CloseHandle(hThread_);
		hThread_ = nullptr;
		idThread_ = 0;
	}
}
unsigned int __stdcall Thread::_StaticRun(LPVOID data) {
	try {
		Thread* thread = reinterpret_cast<Thread*>(data);
		thread->status_ = RUN;
		thread->_Run();
		thread->status_ = STOP;
	}
	catch (...) {
		//エラーは無視
	}
	return 0;
}
void Thread::Start() {
	if (status_ != STOP) {
		this->Stop();
		this->Join();
	}
	hThread_ = (HANDLE)_beginthreadex(nullptr, 0, _StaticRun, (void*)this, 0, &idThread_);
}
void Thread::Stop() {
	if (status_ == RUN) status_ = REQUEST_STOP;
}
bool Thread::IsStop() {
	return hThread_ == nullptr || status_ == STOP;
}
DWORD Thread::Join(int mills) {
	DWORD res = WAIT_OBJECT_0;

	if (hThread_) {
		res = ::WaitForSingleObject(hThread_, mills);
	}

	if (hThread_) {
		if (res != WAIT_TIMEOUT)
			::CloseHandle(hThread_);
		hThread_ = nullptr;
		idThread_ = 0;
		status_ = STOP;
	}
	return res;
}

//*******************************************************************
//CriticalSection
//*******************************************************************
CriticalSection::CriticalSection() {
	idThread_ = 0;
	countLock_ = 0;
	::InitializeCriticalSection(&cs_);
}
CriticalSection::~CriticalSection() {
	::DeleteCriticalSection(&cs_);
}
void CriticalSection::Enter() {
	if (::GetCurrentThreadId() == idThread_) {
		countLock_++;
		return;
	}

	::EnterCriticalSection(&cs_);
	countLock_ = 1;
	idThread_ = ::GetCurrentThreadId();
}
void CriticalSection::Leave() {
	if (::GetCurrentThreadId() == idThread_) {
		countLock_--;
		if (countLock_ != 0) return;
		if (countLock_ < 0)
			throw std::exception("CriticalSection: Thread is not locked.");
	}
	else {
		throw std::exception("CriticalSection: Thread cannot be locked.");
	}
	idThread_ = 0;
	::LeaveCriticalSection(&cs_);
}

//*******************************************************************
//StaticLock
//*******************************************************************
CRITICAL_SECTION* StaticLock::cs_ = nullptr;
StaticLock::StaticLock() {
	if (cs_ == nullptr) {
		cs_ = new CRITICAL_SECTION;
		::InitializeCriticalSection(cs_);
	}
	::EnterCriticalSection(cs_);
}
StaticLock::~StaticLock() {
	::LeaveCriticalSection(cs_);
}

//*******************************************************************
//ThreadSignal
//*******************************************************************
ThreadSignal::ThreadSignal(bool bManualReset) {
	hEvent_ = ::CreateEventW(nullptr, bManualReset, false, nullptr);
}
ThreadSignal::~ThreadSignal() {
	::CloseHandle(hEvent_);
}
DWORD ThreadSignal::Wait(int mills) {
	DWORD res = WAIT_OBJECT_0;
	if (hEvent_)
		res = ::WaitForSingleObject(hEvent_, mills);
	return res;
}
void ThreadSignal::SetSignal(bool bOn) {
	if (bOn)
		::SetEvent(hEvent_);
	else
		::ResetEvent(hEvent_);
}
