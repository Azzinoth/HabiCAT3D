#pragma once

#include "FECGALWrapper.h"

#include <ole2.h>
#include <codecvt>

class ExcelWrapper
{
public:
	SINGLETON_PUBLIC_PART(ExcelWrapper)

	void OutputToExcel(std::vector<std::vector<int>> Data);
private:
	SINGLETON_PRIVATE_PART(ExcelWrapper)

	HRESULT AutoWrap(int autoType, VARIANT* pvResult, IDispatch* pDisp, LPOLESTR ptName, int cArgs...);
	std::string ColumnFromIndex(size_t Index);
	std::string CellFromPosition(size_t ColumnIndex, size_t Row);
};

#define ExcelManager ExcelWrapper::getInstance()
