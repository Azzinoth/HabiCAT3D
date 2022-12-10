#include "ExcelWrapper.h"
using namespace FocalEngine;

ExcelWrapper* ExcelWrapper::Instance = nullptr;

ExcelWrapper::ExcelWrapper()
{
	// Initialize COM for this thread.
	CoInitialize(nullptr);

	// Get CLSID for our server.
	HRESULT hr = CLSIDFromProgID(L"Excel.Application", &Clsid);

	if (FAILED(hr))
	{
		::MessageBox(nullptr, "CLSIDFromProgID() failed", "Error", 0x10010);
		return;
	}
}

ExcelWrapper::~ExcelWrapper()
{
	// Uninitialize COM for this thread.
	CoUninitialize();
}

// AutoWrap() - Automation helper function...
HRESULT ExcelWrapper::AutoWrap(int autoType, VARIANT* pvResult, IDispatch* pDisp, LPOLESTR ptName, int cArgs...)
{
	// Begin variable-argument list...
	va_list marker;
	va_start(marker, cArgs);

	if (!pDisp) {
		MessageBox(nullptr, "NULL IDispatch passed to AutoWrap()", "Error", 0x10010);
		_exit(0);
	}

	// Variables used...
	DISPPARAMS dp = {nullptr, nullptr, 0, 0 };
	DISPID dispidNamed = DISPID_PROPERTYPUT;
	DISPID dispID;
	HRESULT hr;
	char buf[200];
	char szName[200];


	// Convert down to ANSI
	WideCharToMultiByte(CP_ACP, 0, ptName, -1, szName, 256, nullptr, nullptr);

	// Get DISPID for name passed...
	hr = pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, &dispID);

	if (bTesting)
		return hr;

	if (FAILED(hr)) {
		sprintf(buf, "IDispatch::GetIDsOfNames(\"%s\") failed w/err 0x%08lx", szName, hr);
		MessageBox(nullptr, buf, "AutoWrap()", 0x10010);
		_exit(0);
		return hr;
	}

	// Allocate memory for arguments...
	VARIANT* pArgs = new VARIANT[cArgs + 1];
	// Extract arguments...
	for (int i = 0; i < cArgs; i++) {
		pArgs[i] = va_arg(marker, VARIANT);
	}

	// Build DISPPARAMS
	dp.cArgs = cArgs;
	dp.rgvarg = pArgs;

	// Handle special-case for property-puts!
	if (autoType & DISPATCH_PROPERTYPUT) {
		dp.cNamedArgs = 1;
		dp.rgdispidNamedArgs = &dispidNamed;
	}

	// Make the call!
	hr = pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, autoType, &dp, pvResult, nullptr, nullptr);
	if (FAILED(hr)) {
		sprintf(buf, "IDispatch::Invoke(\"%s\"=%08lx) failed w/err 0x%08lx", szName, dispID, hr);
		MessageBox(nullptr, buf, "AutoWrap()", 0x10010);
		_exit(0);
		return hr;
	}
	// End variable-argument section...
	va_end(marker);

	delete[] pArgs;

	return hr;
}

std::string ExcelWrapper::ColumnFromIndex(size_t Index)
{
	std::string Result;

	while (Index > 0)
	{
		// Find remainder
		int rem = Index % 26;

		// If remainder is 0, then a 'Z' must be there in output
		if (rem == 0)
		{
			Result += 'Z';
			Index = (Index / 26) - 1;
		}
		else // If remainder is non-zero
		{
			Result += (rem - 1) + 'A';
			Index = Index / 26;
		}
	}

	// Reverse the string and print result
	std::reverse(Result.begin(), Result.end());

	return Result;
}

//std::vector<std::string> GetAllWords(int Length)
//{
//	std::string Alphabet;
//	for (size_t i = 65; i <= 90; i++)
//	{
//		Alphabet.push_back(i);
//	}
//
//	std::vector<std::string> Result;
//
//	if (Length == 1)
//	{
//		for (size_t i = 0; i < Alphabet.size(); i++)
//		{
//			std::string Temp;
//			Temp.resize(1);
//			Temp[0] = Alphabet[i];
//			Result.push_back(Temp);
//		}
//	}
//	else if (Length == 2)
//	{
//		for (size_t i = 0; i < Alphabet.size(); i++)
//		{
//			for (size_t j = 0; j < Alphabet.size(); j++)
//			{
//				std::string Temp;
//				Temp.resize(2);
//
//				Temp[0] = Alphabet[i];
//				Temp[1] = Alphabet[j];
//
//				Result.push_back(Temp);
//			}
//		}
//	}
//	else if (Length == 3)
//	{
//		for (size_t i = 0; i < Alphabet.size(); i++)
//		{
//			for (size_t j = 0; j < Alphabet.size(); j++)
//			{
//				for (size_t k = 0; k < Alphabet.size(); k++)
//				{
//					std::string Temp;
//					Temp.resize(3);
//
//					Temp[0] = Alphabet[i];
//					Temp[1] = Alphabet[j];
//					Temp[2] = Alphabet[k];
//
//					Result.push_back(Temp);
//				}
//			}
//		}
//	}
//	else if (Length == 4)
//	{
//		for (size_t i = 0; i < Alphabet.size(); i++)
//		{
//			for (size_t j = 0; j < Alphabet.size(); j++)
//			{
//				for (size_t k = 0; k < Alphabet.size(); k++)
//				{
//					for (size_t p = 0; p < Alphabet.size(); p++)
//					{
//						std::string Temp;
//						Temp.resize(4);
//
//						Temp[0] = Alphabet[i];
//						Temp[1] = Alphabet[j];
//						Temp[2] = Alphabet[k];
//						Temp[3] = Alphabet[p];
//
//						Result.push_back(Temp);
//					}
//				}
//			}
//		}
//	}
//	else if (Length == 5)
//	{
//		for (size_t i = 0; i < Alphabet.size(); i++)
//		{
//			for (size_t j = 0; j < Alphabet.size(); j++)
//			{
//				for (size_t k = 0; k < Alphabet.size(); k++)
//				{
//					for (size_t p = 0; p < Alphabet.size(); p++)
//					{
//						for (size_t a = 0; a < Alphabet.size(); a++)
//						{
//							std::string Temp;
//							Temp.resize(5);
//
//							Temp[0] = Alphabet[i];
//							Temp[1] = Alphabet[j];
//							Temp[2] = Alphabet[k];
//							Temp[3] = Alphabet[p];
//							Temp[4] = Alphabet[a];
//
//							Result.push_back(Temp);
//						}
//					}
//				}
//			}
//		}
//	}
//
//	return Result;
//}

std::string ExcelWrapper::CellFromPosition(size_t ColumnIndex, size_t Row)
{
	std::string Result = ColumnFromIndex(ColumnIndex);
	Result += std::to_string(Row);

	return Result;
}

void ExcelWrapper::OutputToExcel(std::vector<std::vector<int>> Data)
{
	int DataW = Data.size();
	int DataH = Data[0].size();

	// Start server and get IDispatch...
	//IDispatch* pXlApp;
	HRESULT hr = CoCreateInstance(Clsid, nullptr, CLSCTX_LOCAL_SERVER, IID_IDispatch, (void**)&CurrentExcelApplication);
	if (FAILED(hr)) {
		::MessageBox(nullptr, "Excel not registered properly", "Error", 0x10010);
		//return -2;
		return;
	}

	// Make it visible (i.e. app.visible = 1)
	{

		VARIANT x;
		x.vt = VT_I4;
		x.lVal = 1;
		AutoWrap(DISPATCH_PROPERTYPUT, nullptr, CurrentExcelApplication, L"Visible", 1, x);
	}

	// Get Workbooks collection
	IDispatch* pXlBooks;
	{
		VARIANT result;
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, CurrentExcelApplication, L"Workbooks", 0);
		pXlBooks = result.pdispVal;
	}

	//bTesting = true;
	//std::vector<std::string> Correct;
	//for (size_t i = 1; i < 6; i++)
	//{
	//	auto AllWords = GetAllWords(i);
	//	for (size_t j = 0; j < AllWords.size(); j++)
	//	{
	//		VARIANT result;
	//		VariantInit(&result);
	//		std::wstring Temp(AllWords[j].begin(), AllWords[j].end());
	//		if (AutoWrap(DISPATCH_PROPERTYGET, &result, pXlBooks, LPOLESTR(Temp.c_str()), 0) == S_OK)
	//		{
	//			//Correct.push_back(AllWords[j]);
	//			LOG.Add(AllWords[j], "Correct");
	//		}

	//	}
	//}
	//bTesting = false;

	// Call Workbooks.Add() to get a new workbook...
	IDispatch* pXlBook;
	{
		VARIANT result;
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, pXlBooks, L"Add", 0);
		pXlBook = result.pdispVal;
	}

	// Create a 15x15 safearray of variants...
	VARIANT arr;
	arr.vt = VT_ARRAY | VT_VARIANT;

	SAFEARRAYBOUND sab[2];
	sab[0].lLbound = 1;
	sab[0].cElements = DataW;

	sab[1].lLbound = 1;
	sab[1].cElements = DataH;
	arr.parray = SafeArrayCreate(VT_VARIANT, 2, sab);


	// Fill safearray with some values...
	for (int i = 1; i <= DataW; i++)
	{
		for (int j = 1; j <= DataH; j++)
		{
			// Create entry value for (i,j)
			VARIANT tmp;
			tmp.vt = VT_I4;
			tmp.lVal = Data[i - 1][j - 1];
			// Add to safearray...
			long indices[] = { i,j };
			SafeArrayPutElement(arr.parray, indices, (void*)&tmp);
		}
	}

	// Get ActiveSheet object
	IDispatch* pXlSheet;
	{
		VARIANT result;
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, CurrentExcelApplication, L"ActiveSheet", 0);
		pXlSheet = result.pdispVal;
	}

	// Get Range object for the Range A1:O5...
	IDispatch* pXlRange;

	std::string FirstCell = CellFromPosition(1, 1);
	std::string LastCell = CellFromPosition(DataW, DataH);

	std::string Range = FirstCell + ":" + LastCell;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
	std::wstring RangeW = conv.from_bytes(Range);

	VARIANT parm;
	parm.vt = VT_BSTR;
	parm.bstrVal = ::SysAllocString(RangeW.c_str());

	VARIANT result;
	VariantInit(&result);
	AutoWrap(DISPATCH_PROPERTYGET, &result, pXlSheet, L"Range", 1, parm);
	VariantClear(&parm);

	pXlRange = result.pdispVal;

	// Set range with our safearray...
	AutoWrap(DISPATCH_PROPERTYPUT, nullptr, pXlRange, L"Value", 1, arr);

	// Wait for user...
	//::MessageBox(NULL, "All done.", "Notice", 0x10000);

	// Set .Saved property of workbook to TRUE so we aren't prompted
	// to save when we tell Excel to quit...
	{
		VARIANT x;
		x.vt = VT_I4;
		x.lVal = 1;
		AutoWrap(DISPATCH_PROPERTYPUT, nullptr, pXlBook, L"Saved", 1, x);
	}

	// Tell Excel to quit (i.e. App.Quit)
	//AutoWrap(DISPATCH_METHOD, NULL, pXlApp, L"Quit", 0);

	// Release references.
	pXlRange->Release();
	pXlSheet->Release();
	pXlBook->Release();
	pXlBooks->Release();
	//CurrentExcelApplication->Release();
	VariantClear(&arr);
}

void ExcelWrapper::Test(std::vector<std::vector<int>> Data)
{
	if (CurrentExcelApplication == nullptr)
		return;

	int DataW = Data.size();
	int DataH = Data[0].size();

	// Get Workbooks collection
	IDispatch* pXlBooks;
	{
		VARIANT result;
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, CurrentExcelApplication, L"Workbooks", 0);
		pXlBooks = result.pdispVal;
	}

	// Create a 15x15 safearray of variants...
	VARIANT arr;
	arr.vt = VT_ARRAY | VT_VARIANT;

	SAFEARRAYBOUND sab[2];
	sab[0].lLbound = 1;
	sab[0].cElements = DataW;

	sab[1].lLbound = 1;
	sab[1].cElements = DataH;
	arr.parray = SafeArrayCreate(VT_VARIANT, 2, sab);


	// Fill safearray with some values...
	for (int i = 1; i <= DataW; i++)
	{
		for (int j = 1; j <= DataH; j++)
		{
			// Create entry value for (i,j)
			VARIANT tmp;
			tmp.vt = VT_I4;
			tmp.lVal = Data[i - 1][j - 1];

			//tmp.vt = VT_BSTR/*VT_LPSTR*/;
			//tmp.bstrVal = BSTR("asd");
			// Add to safearray...
			long indices[] = { i,j };
			SafeArrayPutElement(arr.parray, indices, (void*)&tmp);
		}
	}

	// Get ActiveSheet object
	IDispatch* pXlSheet;
	{
		VARIANT result;
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, CurrentExcelApplication, L"ActiveSheet", 0);
		pXlSheet = result.pdispVal;
	}

	// Get Range object for the Range A1:O5...
	IDispatch* pXlRange;

	std::string FirstCell = CellFromPosition(1, NextRowToUse);
	std::string LastCell = CellFromPosition(DataW, NextRowToUse - 1 + DataH);
	NextRowToUse += DataH + 2;

	std::string Range = FirstCell + ":" + LastCell;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
	std::wstring RangeW = conv.from_bytes(Range);

	VARIANT parm;
	parm.vt = VT_BSTR;
	parm.bstrVal = ::SysAllocString(RangeW.c_str());

	VARIANT result;
	VariantInit(&result);
	AutoWrap(DISPATCH_PROPERTYGET, &result, pXlSheet, L"Range", 1, parm);
	VariantClear(&parm);

	pXlRange = result.pdispVal;

	// Set range with our safearray...
	AutoWrap(DISPATCH_PROPERTYPUT, nullptr, pXlRange, L"Value", 1, arr);

	// Release references.
	pXlRange->Release();
	pXlSheet->Release();
	pXlBooks->Release();
	VariantClear(&arr);
}