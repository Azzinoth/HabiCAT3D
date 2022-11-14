#include "ExcelWrapper.h"
using namespace FocalEngine;

ExcelWrapper* ExcelWrapper::Instance = nullptr;

ExcelWrapper::ExcelWrapper() {}
ExcelWrapper::~ExcelWrapper() {}

// AutoWrap() - Automation helper function...
HRESULT ExcelWrapper::AutoWrap(int autoType, VARIANT* pvResult, IDispatch* pDisp, LPOLESTR ptName, int cArgs...)
{
	// Begin variable-argument list...
	va_list marker;
	va_start(marker, cArgs);

	if (!pDisp) {
		MessageBox(NULL, "NULL IDispatch passed to AutoWrap()", "Error", 0x10010);
		_exit(0);
	}

	// Variables used...
	DISPPARAMS dp = { NULL, NULL, 0, 0 };
	DISPID dispidNamed = DISPID_PROPERTYPUT;
	DISPID dispID;
	HRESULT hr;
	char buf[200];
	char szName[200];


	// Convert down to ANSI
	WideCharToMultiByte(CP_ACP, 0, ptName, -1, szName, 256, NULL, NULL);

	// Get DISPID for name passed...
	hr = pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, &dispID);
	if (FAILED(hr)) {
		sprintf(buf, "IDispatch::GetIDsOfNames(\"%s\") failed w/err 0x%08lx", szName, hr);
		MessageBox(NULL, buf, "AutoWrap()", 0x10010);
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
	hr = pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, autoType, &dp, pvResult, NULL, NULL);
	if (FAILED(hr)) {
		sprintf(buf, "IDispatch::Invoke(\"%s\"=%08lx) failed w/err 0x%08lx", szName, dispID, hr);
		MessageBox(NULL, buf, "AutoWrap()", 0x10010);
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

std::string ExcelWrapper::CellFromPosition(size_t ColumnIndex, size_t Row)
{
	std::string Result = ColumnFromIndex(ColumnIndex);
	Result += std::to_string(Row);

	return Result;
}

void ExcelWrapper::OutputToExcel(std::vector<std::vector<int>> Data)
{
	auto test0 = ColumnFromIndex(1);
	test0 = ColumnFromIndex(7);
	test0 = ColumnFromIndex(37);

	int DataW = Data.size();
	int DataH = Data[0].size();

	// Initialize COM for this thread...
	CoInitialize(NULL);

	// Get CLSID for our server...
	CLSID clsid;
	HRESULT hr = CLSIDFromProgID(L"Excel.Application", &clsid);

	if (FAILED(hr)) {

		::MessageBox(NULL, "CLSIDFromProgID() failed", "Error", 0x10010);
		//return -1;
		return;
	}

	// Start server and get IDispatch...
	IDispatch* pXlApp;
	hr = CoCreateInstance(clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IDispatch, (void**)&pXlApp);
	if (FAILED(hr)) {
		::MessageBox(NULL, "Excel not registered properly", "Error", 0x10010);
		//return -2;
		return;
	}

	// Make it visible (i.e. app.visible = 1)
	{

		VARIANT x;
		x.vt = VT_I4;
		x.lVal = 1;
		AutoWrap(DISPATCH_PROPERTYPUT, NULL, pXlApp, L"Visible", 1, x);
	}

	// Get Workbooks collection
	IDispatch* pXlBooks;
	{
		VARIANT result;
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, pXlApp, L"Workbooks", 0);
		pXlBooks = result.pdispVal;
	}

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
		AutoWrap(DISPATCH_PROPERTYGET, &result, pXlApp, L"ActiveSheet", 0);
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
	AutoWrap(DISPATCH_PROPERTYPUT, NULL, pXlRange, L"Value", 1, arr);

	// Wait for user...
	//::MessageBox(NULL, "All done.", "Notice", 0x10000);

	// Set .Saved property of workbook to TRUE so we aren't prompted
	// to save when we tell Excel to quit...
	{
		VARIANT x;
		x.vt = VT_I4;
		x.lVal = 1;
		AutoWrap(DISPATCH_PROPERTYPUT, NULL, pXlBook, L"Saved", 1, x);
	}

	// Tell Excel to quit (i.e. App.Quit)
	//AutoWrap(DISPATCH_METHOD, NULL, pXlApp, L"Quit", 0);

	// Release references...
	pXlRange->Release();
	pXlSheet->Release();
	pXlBook->Release();
	pXlBooks->Release();
	pXlApp->Release();
	VariantClear(&arr);

	// Uninitialize COM for this thread...
	CoUninitialize();
}