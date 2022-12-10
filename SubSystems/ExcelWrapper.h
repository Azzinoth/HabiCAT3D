#pragma once

#include "FECGALWrapper.h"
#include <ole2.h>
#include <codecvt>

class ExcelWrapper
{
public:
	SINGLETON_PUBLIC_PART(ExcelWrapper)

	void OutputToExcel(std::vector<std::vector<int>> Data);
	void Test(std::vector<std::vector<int>> Data);
private:
	SINGLETON_PRIVATE_PART(ExcelWrapper)

	CLSID Clsid;
	IDispatch* CurrentExcelApplication = nullptr;
	HRESULT AutoWrap(int autoType, VARIANT* pvResult, IDispatch* pDisp, LPOLESTR ptName, int cArgs...);
	std::string ColumnFromIndex(size_t Index);
	std::string CellFromPosition(size_t ColumnIndex, size_t Row);

	bool bTesting = false;
	int NextRowToUse = 4;
};

#define ExcelManager ExcelWrapper::getInstance()
