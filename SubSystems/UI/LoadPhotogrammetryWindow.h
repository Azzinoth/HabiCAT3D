#include "../COLMAP/COLMAPDataManager.h"

class LoadPhotogrammetryWindow
{
	SINGLETON_PRIVATE_PART(LoadPhotogrammetryWindow)

	bool bShouldOpen = false;
	bool bShouldClose = false;

	std::string FolderPath = "";
	COLMAPFoundData FoundData;

	void InternalClose();
	
public:
	SINGLETON_PUBLIC_PART(LoadPhotogrammetryWindow)

	void Show(std::string FolderPath, COLMAPFoundData FoundData);
	void Close();
	void Render();
};

#define LOAD_PHOTOGRAMMETRY_WINDOW LoadPhotogrammetryWindow::GetInstance()