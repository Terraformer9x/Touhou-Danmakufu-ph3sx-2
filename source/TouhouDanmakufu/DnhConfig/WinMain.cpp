#include "source/GcLib/pch.h"

#include "GcLibImpl.hpp"
#include "MainWindow.hpp"


//*******************************************************************
//WinMain
//*******************************************************************
int APIENTRY wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR lpCmdLine,
	int nCmdShow)
{
	//gstd::DebugUtility::DumpMemoryLeaksOnExit();

	try {
		SystemUtility::InitializeCOM();

		directx::EDirect3D9::CreateInstance();
		DnhConfiguration::CreateInstance();

		MainWindow* wndMain = MainWindow::CreateInstance();
		wndMain->Initialize();

		EApplication* app = EApplication::CreateInstance();
		app->Initialize();

		if (app->IsRun()) {
			bool bInit = app->_Initialize();
			if (bInit)
				app->Run();
			app->_Finalize();
		}
	}
	catch (std::exception& e) {
		::MessageBoxW(nullptr, StringUtility::ConvertMultiToWide(e.what()).c_str(),
			L"Unexpected Error", MB_ICONERROR | MB_APPLMODAL | MB_OK);
	}
	catch (gstd::wexception& e) {
		::MessageBoxW(nullptr, e.what(),
			L"Engine Error", MB_ICONERROR | MB_APPLMODAL | MB_OK);
	}

	EApplication::DeleteInstance();
	MainWindow::DeleteInstance();
	DnhConfiguration::DeleteInstance();
	directx::EDirect3D9::DeleteInstance();

	SystemUtility::UninitializeCOM();

	return 0;
}

