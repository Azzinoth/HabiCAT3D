#include "ConsoleJobManager.h"
using namespace FocalEngine;

ConsoleJobManager* ConsoleJobManager::Instance = nullptr;

ConsoleJobManager::ConsoleJobManager()
{
	ConsoleJobsInfo["file_load"].CommandName = "file_load";
	ConsoleJobsInfo["file_load"].Purpose = "Loads a file from the specified path.";
	ConsoleJobSettingsInfo CurrentSettingInfo;
	CurrentSettingInfo.Name = "filepath";
	CurrentSettingInfo.Description = "The path of the file to load.";
	CurrentSettingInfo.bIsOptional = false;
	ConsoleJobsInfo["file_load"].SettingsInfo.push_back(CurrentSettingInfo);
	
	ConsoleJobsInfo["file_save"].CommandName = "file_save";
	ConsoleJobsInfo["file_save"].Purpose = "Saves the current state to a file at the specified path.";
	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "filepath";
	CurrentSettingInfo.Description = "The path of the file to save.";
	CurrentSettingInfo.bIsOptional = false;
	ConsoleJobsInfo["file_save"].SettingsInfo.push_back(CurrentSettingInfo);

	ConsoleJobsInfo["run_script_file"].CommandName = "file_save";
	ConsoleJobsInfo["run_script_file"].Purpose = "Executes a sequence of commands from a specified script(text) file. Each command in the file should be on a new line.";
	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "filepath";
	CurrentSettingInfo.Description = "The path of the script file to execute.";
	CurrentSettingInfo.bIsOptional = false;
	ConsoleJobsInfo["run_script_file"].SettingsInfo.push_back(CurrentSettingInfo);

	// ********** COMPLEXITY **********
	ConsoleJobsInfo["complexity"].CommandName = "complexity";
	ConsoleJobsInfo["complexity"].Purpose = "Creates a job to add complexity layer of a model based on the specified type.";
	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "type";
	CurrentSettingInfo.Description = "Specifies the type of complexity calculation.";
	CurrentSettingInfo.bIsOptional = false;
	CurrentSettingInfo.PossibleValues = { "HEIGHT", "AREA", "RUGOSITY", "TRIANGLE_EDGE", "TRIANGLE_COUNT", "VECTOR_DISPERSION", "FRACTAL_DIMENSION", "COMPARE" };
	ConsoleJobsInfo["complexity"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "resolution";
	CurrentSettingInfo.Description = "Specifies the resolution in meters for the complexity calculation. Alternative to relative_resolution.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "Minimal possible";
	ConsoleJobsInfo["complexity"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "relative_resolution";
	CurrentSettingInfo.Description = "Specifies the resolution as a float between 0.0 and 1.0, where 0.0 represents the lowest possible resolution and 1.0 represents the highest. Alternative to resolution.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "0.0";
	ConsoleJobsInfo["complexity"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "jitter_quality";
	CurrentSettingInfo.Description = "Specifies the quality of jitter applied to the model. Higher values mean more jitters and potentially smoother results but slower processing.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "55";
	CurrentSettingInfo.PossibleValues = JITTER_MANAGER.GetJitterVectorSetNames();
	ConsoleJobsInfo["complexity"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "run_on_whole_model";
	CurrentSettingInfo.Description = "Specifies if the calculation should be run on the whole model without jitter.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "false";
	ConsoleJobsInfo["complexity"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "triangle_edges_mode";
	CurrentSettingInfo.Description = "Specifies the mode of triangle edges calculation. Relevant only for 'TRIANGLE_EDGE' complexity type.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "MAX_LEHGTH";
	ConsoleJobsInfo["complexity"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "rugosity_algorithm";
	CurrentSettingInfo.Description = "Specifies the algorithm for rugosity calculation. Relevant only for 'RUGOSITY' complexity type.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "AVERAGE";
	CurrentSettingInfo.PossibleValues = { "AVERAGE", "MIN", "LSF(CGAL)" };
	ConsoleJobsInfo["complexity"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "rugosity_is_using_unique_projected_area";
	CurrentSettingInfo.Description = "Specifies if the unique projected area should be used for rugosity calculation. Relevant only for 'RUGOSITY' complexity type.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "false";
	ConsoleJobsInfo["complexity"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "rugosity_delete_outliers";
	CurrentSettingInfo.Description = "Specifies if the outliers should be deleted from the rugosity calculation. Relevant only for 'RUGOSITY' complexity type.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "true";
	ConsoleJobsInfo["complexity"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "rugosity_min_algorithm_quality";
	CurrentSettingInfo.Description = "Specifies the quality of the rugosity calculation. Relevant only for 'RUGOSITY' complexity type and when the rugosity_algorithm is set to 'MIN'.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "91";
	CurrentSettingInfo.PossibleValues = RUGOSITY_LAYER_PRODUCER.GetOrientationSetNamesForMinRugosityList();
	ConsoleJobsInfo["complexity"].SettingsInfo.push_back(CurrentSettingInfo);
	
	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "fractal_dimension_should_filter_values";
	CurrentSettingInfo.Description = "Specifies if the app should filter values that are less that 2.0. Relevant only for 'FRACTAL_DIMENSION' complexity type.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "true";
	ConsoleJobsInfo["complexity"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "is_standard_deviation_needed";
	CurrentSettingInfo.Description = "Specifies if the app should also add layer with standard deviation.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "false";
	ConsoleJobsInfo["complexity"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "compare_first_layer_index";
	CurrentSettingInfo.Description = "Specifies the index of the first layer to compare. Relevant only for 'COMPARE' complexity type.";
	CurrentSettingInfo.bIsOptional = false;
	ConsoleJobsInfo["complexity"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "compare_second_layer_index";
	CurrentSettingInfo.Description = "Specifies the index of the second layer to compare. Relevant only for 'COMPARE' complexity type.";
	CurrentSettingInfo.bIsOptional = false;
	ConsoleJobsInfo["complexity"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "compare_normalize";
	CurrentSettingInfo.Description = "Specifies if the app should normalize the layers before comparing. Relevant only for 'COMPARE' complexity type.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "true";
	ConsoleJobsInfo["complexity"].SettingsInfo.push_back(CurrentSettingInfo);
	// ********** COMPLEXITY END **********

	// ********** EVALUATION **********
	ConsoleJobsInfo["evaluation"].CommandName = "evaluation";
	ConsoleJobsInfo["evaluation"].Purpose = "Creates an evaluation job with the specified settings to test a layer or other objects.";
	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "type";
	CurrentSettingInfo.Description = "Specifies the type of evaluation.";
	CurrentSettingInfo.bIsOptional = false;
	CurrentSettingInfo.PossibleValues = { "COMPLEXITY" };

	ConsoleJobsInfo["evaluation"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "subtype";
	CurrentSettingInfo.Description = "Specifies the subtype of evaluation.";
	CurrentSettingInfo.bIsOptional = false;
	CurrentSettingInfo.PossibleValues = { "MEAN_LAYER_VALUE", "MEDIAN_LAYER_VALUE", "MAX_LAYER_VALUE", "MIN_LAYER_VALUE" };
	ConsoleJobsInfo["evaluation"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "expected_value";
	CurrentSettingInfo.Description = "Specifies the expected value for the evaluation.";
	CurrentSettingInfo.bIsOptional = false;
	ConsoleJobsInfo["evaluation"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "tolerance";
	CurrentSettingInfo.Description = "Specifies the tolerance for the evaluation.";
	CurrentSettingInfo.bIsOptional = false;
	ConsoleJobsInfo["evaluation"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "layer_index";
	CurrentSettingInfo.Description = "Specifies the index of the layer to evaluate. Relevant only for 'COMPLEXITY' evaluation type.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "'-1' Which means the last layer.";
	ConsoleJobsInfo["evaluation"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "convert_to_script";
	CurrentSettingInfo.Description = "Specifies if the job should be converted to a script that later can be used to run the same job but with actual values.(Mostly used to make it easier to create a script file for new models)";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "false";
	ConsoleJobsInfo["evaluation"].SettingsInfo.push_back(CurrentSettingInfo);
	// ********** EVALUATION END **********

	// ********** GLOBAL SETTINGS **********
	ConsoleJobsInfo["global_settings"].CommandName = "global_settings";
	ConsoleJobsInfo["global_settings"].Purpose = "Sets a global setting for the application.";
	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "type";
	CurrentSettingInfo.Description = "Specifies the type of global setting.";
	CurrentSettingInfo.bIsOptional = false;
	CurrentSettingInfo.PossibleValues = { "EVALUATION_JOB_TO_SCRIPT" };
	ConsoleJobsInfo["global_settings"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "int_value";
	CurrentSettingInfo.Description = "Specifies the integer value for the global setting.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "0";
	ConsoleJobsInfo["global_settings"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "float_value";
	CurrentSettingInfo.Description = "Specifies the float value for the global setting.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "0.0";
	ConsoleJobsInfo["global_settings"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "bool_value";
	CurrentSettingInfo.Description = "Specifies the boolean value for the global setting.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "false";
	ConsoleJobsInfo["global_settings"].SettingsInfo.push_back(CurrentSettingInfo);
	// ********** GLOBAL SETTINGS END **********

	// ********** EXPORT LAYER AS IMAGE **********
	ConsoleJobsInfo["export_layer_as_image"].CommandName = "export_layer_as_image";
	ConsoleJobsInfo["export_layer_as_image"].Purpose = "Exports a layer as an image.";

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "export_mode";
	CurrentSettingInfo.Description = "Specifies the mode of the export.";
	CurrentSettingInfo.bIsOptional = false;
	CurrentSettingInfo.PossibleValues = { "MIN", "MAX", "MEAN", "CUMULATIVE"};
	ConsoleJobsInfo["export_layer_as_image"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "save_mode";
	CurrentSettingInfo.Description = "Specifies the type of image file.";
	CurrentSettingInfo.bIsOptional = false;
	CurrentSettingInfo.PossibleValues = { "PNG", "GEOTIF", "GEOTIF_32_BITS" };
	ConsoleJobsInfo["export_layer_as_image"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "filepath";
	CurrentSettingInfo.Description = "Specifies the path of the file to save.";
	CurrentSettingInfo.bIsOptional = false;
	ConsoleJobsInfo["export_layer_as_image"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "resolution";
	CurrentSettingInfo.Description = "Specifies the resolution in meters for the image.";
	CurrentSettingInfo.bIsOptional = false;
	ConsoleJobsInfo["export_layer_as_image"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "layer_index";
	CurrentSettingInfo.Description = "Specifies the index of the layer to export.";
	CurrentSettingInfo.bIsOptional = false;
	ConsoleJobsInfo["export_layer_as_image"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "force_projection_vector";
	CurrentSettingInfo.Description = "Specifies the projection vector for the image.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.PossibleValues = { "X", "Y", "Z" };
	CurrentSettingInfo.DefaultValue = "Calculated on fly";
	ConsoleJobsInfo["export_layer_as_image"].SettingsInfo.push_back(CurrentSettingInfo);

	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "persent_of_area_that_would_be_red";
	CurrentSettingInfo.Description = "Specifies the persent of area that would be considered outliers and would be red.";
	CurrentSettingInfo.bIsOptional = true;
	CurrentSettingInfo.DefaultValue = "5.0";
	ConsoleJobsInfo["export_layer_as_image"].SettingsInfo.push_back(CurrentSettingInfo);
	// ********** EXPORT LAYER AS IMAGE END **********
}

ConsoleJobManager::~ConsoleJobManager() {}

void ConsoleJobManager::AddJob(ConsoleJob* Job)
{
	JobsList.push_back(Job);
	JobsAdded++;
}

bool ConsoleJobManager::SetGridResolution(ComplexityJob* Job)
{
	if (Job->Settings.ResolutionInM == 0.0f && Job->Settings.RelativeResolution == 0.0f)
	{
		JITTER_MANAGER.SetResolutionInM(JITTER_MANAGER.GetLowestPossibleResolution());
		return true;
	}

	if (Job->Settings.ResolutionInM != 0.0f &&
		Job->Settings.ResolutionInM >= JITTER_MANAGER.GetLowestPossibleResolution() &&
		Job->Settings.ResolutionInM <= JITTER_MANAGER.GetHigestPossibleResolution())
	{
		JITTER_MANAGER.SetResolutionInM(Job->Settings.ResolutionInM);
		return true;
	}

	if (Job->Settings.ResolutionInM == 0.0f && Job->Settings.RelativeResolution != 0.0f)
	{
		float Range = JITTER_MANAGER.GetHigestPossibleResolution() - JITTER_MANAGER.GetLowestPossibleResolution();
		JITTER_MANAGER.SetResolutionInM(JITTER_MANAGER.GetLowestPossibleResolution() + Range * Job->Settings.RelativeResolution);
		return true;
	}

	return false;
}

void ConsoleJobManager::SetRugosityAlgorithm(ComplexityJob* Job)
{
	if (Job->ComplexityType != "RUGOSITY")
		return;

	RUGOSITY_LAYER_PRODUCER.SetUseFindSmallestRugosity(Job->Settings.GetRugosity_Algorithm() == "MIN");
	RUGOSITY_LAYER_PRODUCER.SetUseCGALVariant(Job->Settings.GetRugosity_Algorithm() == "LSF(CGAL)");
}

void ConsoleJobManager::WaitForJitterManager()
{
	while (JITTER_MANAGER.GetJitterDoneCount() != JITTER_MANAGER.GetJitterToDoCount())
	{
		float Progress = float(JITTER_MANAGER.GetJitterDoneCount()) / float(JITTER_MANAGER.GetJitterToDoCount());
		std::cout << "\rProgress: " << std::to_string(Progress * 100.0f) << " %" << std::flush;

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		THREAD_POOL.Update();
	}
}

void ConsoleJobManager::PrintCommandHelp(std::string CommandName)
{
	std::cout << "\n\n";

	ConsoleJobInfo* Info = nullptr;
	if (ConsoleJobsInfo.find(CommandName) != ConsoleJobsInfo.end())
		Info = &ConsoleJobsInfo[CommandName];
	
	if (Info == nullptr)
	{
		std::cout << "No help available for this command." << std::endl;
		return;
	}

	std::cout << "Help for '" << Info->CommandName << "' command:\n";
	std::cout << "Purpose:\n  " << Info->Purpose << "\n\n";
	std::cout << "Settings:\n";

	for (const auto& SettingInfo : Info->SettingsInfo)
	{
		std::cout << "  - " << SettingInfo.Name << " (" << (SettingInfo.bIsOptional ? "Optional" : "Required") << "): " << SettingInfo.Description << "\n";
		if (!SettingInfo.PossibleValues.empty())
		{
			std::cout << "      Possible Values: ";
			for (const auto& value : SettingInfo.PossibleValues)
			{
				std::cout << "'" << value << "', ";
			}
			std::cout << "\b\b \n"; // Removes the last comma and space
		}

		if (SettingInfo.bIsOptional)
		{
			std::cout << "      Default: " << SettingInfo.DefaultValue << "\n";
		}

		std::cout << "\n";
	}
}

void ConsoleJobManager::PrintHelp(std::string CommandName)
{
	if (CommandName.empty())
	{
		std::cout <<
			"Command signature:\n"
			"  -[COMMAND] [OPTIONS]=[VALUE]\n\n"

			"Commands:\n"
			"-help                                           Display this help message.\n"
			"-help command_name=[COMMAND]                    Display help for a specific command along with all its settings.\n"
			"-file_load filepath=[PATH]                      Load a file from the specified path.\n"
			"-file_save filepath=[PATH]                      Save the current state to a file at the specified path.\n"
			"-run_script_file filepath=[PATH]                Execute a sequence of commands from a specified script(text) file.Each command in the file should be on a new line.\n"
			"-complexity type=[LAYER_TYPE]                   Create a complexity job with the specified settings to create a layer.\n"
			"-evaluation type=[TYPE] subtype=[WHAT_TO_TEST]  Create an evaluation job with the specified settings to test a layer or other objects.\n\n"
			"-global_settings type=[TYPE]                    Set a global setting for the application.\n\n"

			"Examples:\n"
			"-load filepath=\"C:/data/mesh.obj\"\n"
			"-save filepath=\"C:/data/processed_mesh.rug\"\n"
			"-complexity type=RUGOSITY rugosity_algorithm = MIN jitter_quality=73\n"
			"-evaluation type=COMPLEXITY subtype=MAX_LAYER_VALUE expected_value=5.02 tolerance=0.01\n\n"

			"Notes:\n"
			"For Boolean settings, use true or false.\n"
			"Paths must be enclosed in quotes.\n\n";
	}
	else
	{
		PrintCommandHelp(CommandName);
	}
}

void ConsoleJobManager::ExecuteJob(ConsoleJob* Job)
{
	if (Job->Type == "FILE_LOAD")
	{
		FileLoadJob* FileJob = reinterpret_cast<FileLoadJob*>(Job);
		
		std::cout << "Initiating file load process for: " << FileJob->FilePath << std::endl;
		if (FILE_SYSTEM.CheckFile(FileJob->FilePath.c_str()))
		{
			std::cout << "File found. Loading file: " << FileJob->FilePath << std::endl;

			COMPLEXITY_METRIC_MANAGER.ImportOBJ(FileJob->FilePath.c_str(), true);
			COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->UpdateAverageNormal();

			OutputConsoleTextWithColor("Successfully completed loading file: ", 0, 255, 0);
			OutputConsoleTextWithColor(FileJob->FilePath, 0, 255, 0);
		}
		else
		{
			std::string ErrorMessage = "Error: File not found - " + FileJob->FilePath + ". Please check the file path and try again.";
			LOG.Add(ErrorMessage, "CONSOLE_LOG");
			OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);

			JobsList.clear();
		}
	}
	else if (Job->Type == "FILE_SAVE")
	{
		COMPLEXITY_METRIC_MANAGER.SaveToRUGFile(reinterpret_cast<FileSaveJob*>(Job)->FilePath);
		OutputConsoleTextWithColor("File saved successfully.", 0, 255, 0);
	}
	else if (Job->Type == "COMPLEXITY_JOB")
	{
		if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo == nullptr)
		{
			std::string ErrorMessage = "Error: No file loaded. Please load a file before attempting to calculate complexity.";
			LOG.Add(ErrorMessage, "CONSOLE_LOG");
			OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
			return;
		}

		ComplexityJob* CurrentComplexityJob = reinterpret_cast<ComplexityJob*>(Job);
		if (!SetGridResolution(CurrentComplexityJob))
		{
			std::string ErrorMessage = "Error: Invalid resolution value. Given value is - " + std::to_string(CurrentComplexityJob->Settings.ResolutionInM) + ". Bur value should be between " + std::to_string(JITTER_MANAGER.GetLowestPossibleResolution()) + " and " + std::to_string(JITTER_MANAGER.GetHigestPossibleResolution()) + ".";
			LOG.Add(ErrorMessage, "CONSOLE_LOG");
			OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
			return;
		}
		JITTER_MANAGER.SetCurrentJitterVectorSetName(CurrentComplexityJob->Settings.GetJitterQuality());

		if (CurrentComplexityJob->ComplexityType == "HEIGHT")
		{
			std::cout << "Initiating Height Layer calculation." << std::endl;

			COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(HEIGHT_LAYER_PRODUCER.Calculate());

			std::cout << "Height Layer calculation completed." << std::endl;
		}
		else if (CurrentComplexityJob->ComplexityType == "AREA")
		{
			std::cout << "Initiating Area Layer calculation." << std::endl;

			COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(AREA_LAYER_PRODUCER.Calculate());

			std::cout << "Area Layer calculation completed." << std::endl;
		}
		else if (CurrentComplexityJob->ComplexityType == "TRIANGLE_EDGE")
		{
			std::cout << "Initiating Triangle Edge Layer calculation." << std::endl;

			int Mode = 0;
			if (CurrentComplexityJob->Settings.GetTriangleEdges_Mode() == "MIN_LEHGTH")
			{
				Mode = 1;
			}
			else if (CurrentComplexityJob->Settings.GetTriangleEdges_Mode() == "MEAN_LEHGTH")
			{
				Mode = 2;
			}

			COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(TRIANGLE_EDGE_LAYER_PRODUCER.Calculate(Mode));

			std::cout << "Triangle Edge Layer calculation completed." << std::endl;
		}
		else if (CurrentComplexityJob->ComplexityType == "TRIANGLE_COUNT")
		{
			std::cout << "Initiating Triangle Count Layer calculation." << std::endl;

			if (CurrentComplexityJob->Settings.IsRunOnWholeModel())
			{
				TRIANGLE_COUNT_LAYER_PRODUCER.CalculateOnWholeModel();
			}
			else
			{
				TRIANGLE_COUNT_LAYER_PRODUCER.CalculateWithJitterAsync(false);
			}

			WaitForJitterManager();

			std::cout << "\rProgress: " << std::to_string(100.0f) << " %" << std::flush;
			std::cout << std::endl;
			std::cout << "Triangle Count Layer calculation completed." << std::endl;
		}
		else if (CurrentComplexityJob->ComplexityType == "RUGOSITY")
		{
			std::cout << "Initiating Rugosity Layer calculation." << std::endl;

			SetRugosityAlgorithm(CurrentComplexityJob);
			RUGOSITY_LAYER_PRODUCER.SetCalculateStandardDeviation(CurrentComplexityJob->Settings.IsStandardDeviationNeeded());
			RUGOSITY_LAYER_PRODUCER.SetDeleteOutliers(CurrentComplexityJob->Settings.IsRugosity_DeleteOutliers());

			RUGOSITY_LAYER_PRODUCER.SetOrientationSetForMinRugosityName(CurrentComplexityJob->Settings.GetRugosity_MinAlgorithm_Quality());
			RUGOSITY_LAYER_PRODUCER.SetIsUsingUniqueProjectedArea(CurrentComplexityJob->Settings.GetRugosity_IsUsingUniqueProjectedArea());

			if (CurrentComplexityJob->Settings.IsRunOnWholeModel())
			{
				RUGOSITY_LAYER_PRODUCER.CalculateOnWholeModel();
			}
			else
			{
				RUGOSITY_LAYER_PRODUCER.CalculateWithJitterAsync();
			}

			WaitForJitterManager();

			std::cout << "\rProgress: " << std::to_string(100.0f) << " %" << std::flush;
			std::cout << std::endl;
			std::cout << "Rugosity Layer calculation completed." << std::endl;
		}
		else if (CurrentComplexityJob->ComplexityType == "VECTOR_DISPERSION")
		{
			std::cout << "Initiating Vector Dispersion calculation." << std::endl;
			
			VECTOR_DISPERSION_LAYER_PRODUCER.SetShouldCalculateStandardDeviation(CurrentComplexityJob->Settings.IsStandardDeviationNeeded());

			if (CurrentComplexityJob->Settings.IsRunOnWholeModel())
			{
				VECTOR_DISPERSION_LAYER_PRODUCER.CalculateOnWholeModel();
			}
			else
			{
				VECTOR_DISPERSION_LAYER_PRODUCER.CalculateWithJitterAsync(false);
			}

			WaitForJitterManager();

			std::cout << "\rProgress: " << std::to_string(100.0f) << " %" << std::flush;
			std::cout << std::endl;
			std::cout << "Vector Dispersion calculation completed." << std::endl;
		}
		else if (CurrentComplexityJob->ComplexityType == "FRACTAL_DIMENSION")
		{
			std::cout << "Initiating Fractal Dimension Layer calculation." << std::endl;

			FRACTAL_DIMENSION_LAYER_PRODUCER.SetShouldCalculateStandardDeviation(CurrentComplexityJob->Settings.IsStandardDeviationNeeded());
			FRACTAL_DIMENSION_LAYER_PRODUCER.SetShouldFilterFractalDimensionValues(CurrentComplexityJob->Settings.GetFractalDimension_ShouldFilterValues());

			if (CurrentComplexityJob->Settings.IsRunOnWholeModel())
			{
				FRACTAL_DIMENSION_LAYER_PRODUCER.CalculateOnWholeModel();
			}
			else
			{
				FRACTAL_DIMENSION_LAYER_PRODUCER.CalculateWithJitterAsync(false);
			}

			WaitForJitterManager();

			std::cout << "\rProgress: " << std::to_string(100.0f) << " %" << std::flush;
			std::cout << std::endl;
			std::cout << "Fractal Dimension Layer calculation completed." << std::endl;
		}
		else if (CurrentComplexityJob->ComplexityType == "COMPARE")
		{
			std::cout << "Initiating Compare Layer calculation." << std::endl;

			int FirstLayerIndex = CurrentComplexityJob->Settings.GetCompare_FirstLayerIndex();
			if (FirstLayerIndex < 0 || FirstLayerIndex > COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size())
			{
				std::string ErrorMessage = "Error: First layer index is out of range. Please check the layer index and try again.";
				LOG.Add(ErrorMessage, "CONSOLE_LOG");
				OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
				return;
			}

			int SecondLayerIndex = CurrentComplexityJob->Settings.GetCompare_SecondLayerIndex();
			if (SecondLayerIndex < 0 || SecondLayerIndex > COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size())
			{
				std::string ErrorMessage = "Error: Second layer index is out of range. Please check the layer index and try again.";
				LOG.Add(ErrorMessage, "CONSOLE_LOG");
				OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
				return;
			}

			COMPARE_LAYER_PRODUCER.SetShouldNormalize(CurrentComplexityJob->Settings.IsCompare_Normalize());
			COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->AddLayer(COMPARE_LAYER_PRODUCER.Calculate(FirstLayerIndex, SecondLayerIndex));

			std::cout << "Compare Layer calculation completed." << std::endl;
		}
	}
 	else if (Job->Type == "EVALUATION_JOB")
	{
		EvaluationJob* CurrentEvaluationJob = reinterpret_cast<EvaluationJob*>(Job);

		if (CurrentEvaluationJob->EvaluationType == "COMPLEXITY")
		{
			ComplexityEvaluationJob* CurrentComplexityEvaluationJob = reinterpret_cast<ComplexityEvaluationJob*>(Job);

			if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.empty())
			{
				std::string ErrorMessage = "Error: No layers to evaluate. Please calculate a layer before attempting to evaluate.";
				LOG.Add(ErrorMessage, "CONSOLE_LOG");
				OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
				return;
			}

			MeshLayer* LayerToEvaluate = nullptr;
			if (CurrentComplexityEvaluationJob->GetLayerIndex() != -1)
			{
				LayerToEvaluate = &COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[CurrentComplexityEvaluationJob->GetLayerIndex()];
			}
			else
			{
				LayerToEvaluate = &COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.back();
			}
			
			if (LayerToEvaluate == nullptr)
			{
				std::string ErrorMessage = "Error: Layer to evaluate is null. Please check the layer index and try again.";
				LOG.Add(ErrorMessage, "CONSOLE_LOG");
				OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
				return;
			}

			CurrentEvaluationJob->bFailed = false;
			float Difference = FLT_MAX;

			if (CurrentComplexityEvaluationJob->GetEvaluationSubType() == "MEAN_LAYER_VALUE")
			{
				CurrentComplexityEvaluationJob->SetActualValue(LayerToEvaluate->GetMean());
			}
			else if (CurrentComplexityEvaluationJob->GetEvaluationSubType() == "MEDIAN_LAYER_VALUE")
			{
				CurrentComplexityEvaluationJob->SetActualValue(LayerToEvaluate->GetMedian());
			}
			else if (CurrentComplexityEvaluationJob->GetEvaluationSubType() == "MAX_LAYER_VALUE")
			{
				CurrentComplexityEvaluationJob->SetActualValue(LayerToEvaluate->GetMax());
			}
			else if (CurrentComplexityEvaluationJob->GetEvaluationSubType() == "MIN_LAYER_VALUE")
			{
				CurrentComplexityEvaluationJob->SetActualValue(LayerToEvaluate->GetMin());
			}

			Difference = CurrentComplexityEvaluationJob->GetExpectedValue() - CurrentComplexityEvaluationJob->GetActualValue();

			if (abs(Difference) > CurrentComplexityEvaluationJob->GetTolerance())
				CurrentComplexityEvaluationJob->bFailed = true;

			if (isnan(CurrentComplexityEvaluationJob->GetActualValue()))
				CurrentComplexityEvaluationJob->bFailed = true;

			EvaluationsTotalCount++;
			if (CurrentComplexityEvaluationJob->Failed())
			{
				std::string ErrorMessage = "Error: Evaluation failed. Type: " + CurrentComplexityEvaluationJob->EvaluationType + " SubType: " + CurrentComplexityEvaluationJob->GetEvaluationSubType() + " Expected: " + std::to_string(CurrentComplexityEvaluationJob->GetExpectedValue()) + " Tolerance: " + std::to_string(CurrentComplexityEvaluationJob->GetTolerance()) + " Actual: " + std::to_string(CurrentComplexityEvaluationJob->GetActualValue());
				LOG.Add(ErrorMessage, "CONSOLE_LOG");
				OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);

				EvaluationsFailedCount++;
			}
			else
			{
				std::string Message = "Evaluation passed. Type: " + CurrentComplexityEvaluationJob->EvaluationType + " SubType: " + CurrentComplexityEvaluationJob->GetEvaluationSubType() + " Expected: " + std::to_string(CurrentComplexityEvaluationJob->GetExpectedValue()) + " Tolerance: " + std::to_string(CurrentComplexityEvaluationJob->GetTolerance()) + " Actual: " + std::to_string(CurrentComplexityEvaluationJob->GetActualValue());
				LOG.Add(Message, "CONSOLE_LOG");
				OutputConsoleTextWithColor(Message, 0, 255, 0);
			}

			if (bConvertEvaluationToUsableScript)
			{
				std::string Script = "-evaluation type=" + CurrentComplexityEvaluationJob->EvaluationType + " subtype=" + CurrentComplexityEvaluationJob->GetEvaluationSubType() + " expected_value=" + std::to_string(CurrentComplexityEvaluationJob->GetActualValue()) + " tolerance=" + std::to_string(CurrentComplexityEvaluationJob->GetTolerance());
				if (CurrentComplexityEvaluationJob->GetLayerIndex() != -1)
					Script += " layer_index=" + std::to_string(CurrentComplexityEvaluationJob->GetLayerIndex());
				
				SavedConvertionsOfEvaluationToUsableScript.push_back(Script);
			}
		}
	}
	else if (Job->Type == "GLOBAL_SETTINGS_JOB")
	{
		GlobalSettingJob* CurrentGlobalSettingsJob = reinterpret_cast<GlobalSettingJob*>(Job);

		if (CurrentGlobalSettingsJob->GlobalSettingType == "EVALUATION_JOB_TO_SCRIPT")
		{
			bConvertEvaluationToUsableScript = CurrentGlobalSettingsJob->GetBoolValue();
		}
	}
	else if (Job->Type == "EXPORT_LAYER_AS_IMAGE_JOB")
	{
		ExportLayerAsImageJob* CurrentExportLayerJob = reinterpret_cast<ExportLayerAsImageJob*>(Job);

		if (COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.empty())
		{
			std::string ErrorMessage = "Error: No layers to export. Please calculate a layer before attempting to export.";
			LOG.Add(ErrorMessage, "CONSOLE_LOG");
			OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
			return;
		}

		MeshLayer* LayerToExport = nullptr;
		if (CurrentExportLayerJob->GetLayerIndex() >= 0 && CurrentExportLayerJob->GetLayerIndex() < COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers.size())
		{
			LayerToExport = &COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->Layers[CurrentExportLayerJob->GetLayerIndex()];
		}
		else
		{
			std::string ErrorMessage = "Error: Layer index is out of range. Please check the layer index and try again.";
			LOG.Add(ErrorMessage, "CONSOLE_LOG");
			OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
			return;
		}

		if (LayerToExport == nullptr)
		{
			std::string ErrorMessage = "Error: Layer to export is null. Please check the layer index and try again.";
			LOG.Add(ErrorMessage, "CONSOLE_LOG");
			OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
			return;
		}

		std::cout << "Initiating Layer export as image." << std::endl;

		LAYER_RASTERIZATION_MANAGER.SetGridRasterizationMode(CurrentExportLayerJob->GetExportMode());

		glm::vec2 MinMaxResolutionInMeters = LAYER_RASTERIZATION_MANAGER.GetMinMaxResolutionInMeters();
		float ResolutionInMeters = CurrentExportLayerJob->GetResolutionInM();
		if (ResolutionInMeters > MinMaxResolutionInMeters.x)
		{
			std::string ErrorMessage = "Error: Resolution is too high. Your value: " + std::to_string(ResolutionInMeters) + " meters. Max value: " + std::to_string(MinMaxResolutionInMeters.x) + " meters. Please check the resolution and try again.";
			LOG.Add(ErrorMessage, "CONSOLE_LOG");
			OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
			return;
		}
			
		if (ResolutionInMeters < MinMaxResolutionInMeters.y)
		{
			std::string ErrorMessage = "Error: Resolution is too low. Your value: " + std::to_string(ResolutionInMeters) + " meters. Min value: " + std::to_string(MinMaxResolutionInMeters.y) + " meters. Please check the resolution and try again.";
			LOG.Add(ErrorMessage, "CONSOLE_LOG");
			OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
			return;
		}
		LAYER_RASTERIZATION_MANAGER.SetResolutionInMeters(ResolutionInMeters);

		LAYER_RASTERIZATION_MANAGER.SetCumulativeModePersentOfAreaThatWouldBeRed(CurrentExportLayerJob->GetPersentOfAreaThatWouldBeRed());

		LAYER_RASTERIZATION_MANAGER.PrepareLayerForExport(LayerToExport, CurrentExportLayerJob->GetForceProjectionVector());

		while (abs(LAYER_RASTERIZATION_MANAGER.GetProgress() - 1.0f) > FLT_EPSILON)
		{
			std::cout << "\rProgress: " << std::to_string(LAYER_RASTERIZATION_MANAGER.GetProgress() * 100.0f) << " %" << std::flush;

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			THREAD_POOL.Update();
		}

		std::cout << "\rProgress: " << std::to_string(100.0f) << " %" << std::flush;
		std::cout << std::endl;
		std::cout << "Layer export calculation completed." << std::endl;

		LAYER_RASTERIZATION_MANAGER.SaveToFile(CurrentExportLayerJob->GetFilePath(), CurrentExportLayerJob->SaveMode);
	}
	else if (Job->Type == "HELP_JOB")
	{
		PrintHelp(reinterpret_cast<HelpJob*>(Job)->CommandName);
	}
	else if (Job->Type == "EXIT_JOB")
	{
		APPLICATION.Close();
	}
	else
	{
		std::string ErrorMessage = "Error: Unknown job type: " + Job->Type + ". Please check the job type and try again.";
		LOG.Add(ErrorMessage, "CONSOLE_LOG");
		OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
	}
}

void ConsoleJobManager::Update()
{
	if (JobsList.empty() && JobsAdded != 0)
	{
		OnAllJobsFinished();
		JobsAdded = 0;
	}
		
	if (!JobsList.empty())
	{
		ConsoleJob* CurrentJob = JobsList.front();
		if (CurrentJob != nullptr)
		{
			ExecuteJob(CurrentJob);

			if (!APPLICATION.IsNotTerminated())
			{
				JobsList.clear();
				return;
			}

			if (JobsList.empty())
				return;

			delete CurrentJob;
			JobsList.erase(JobsList.begin());
		}
	}
	else
	{
		std::string NewCommand;
		std::cout << "Enter command: ";
		std::getline(std::cin, NewCommand);
		std::vector<CommandLineAction> Actions = APPLICATION.ParseCommandLine(NewCommand);
		std::vector<ConsoleJob*> NewJobs = ConvertCommandAction(Actions);
		for (size_t i = 0; i < NewJobs.size(); i++)
		{
			AddJob(NewJobs[i]);
		}
	}
}

void ConsoleJobManager::OnAllJobsFinished()
{
	if (EvaluationsTotalCount > 1)
	{
		std::cout << "All jobs finished." << std::endl;

		if (EvaluationsFailedCount == 0)
		{
			OutputConsoleTextWithColor("All evaluations passed: " + std::to_string(EvaluationsTotalCount) + " out of " + std::to_string(EvaluationsTotalCount), 0, 255, 0);
		}
		else if (EvaluationsFailedCount > 0 && EvaluationsFailedCount < EvaluationsTotalCount)
		{
			OutputConsoleTextWithColor("Some evaluations failed, only: " + std::to_string(EvaluationsTotalCount - EvaluationsFailedCount) + " out of " + std::to_string(EvaluationsTotalCount) + " passed.", 255, 255, 0);
		}
		else if (EvaluationsFailedCount == EvaluationsTotalCount)
		{
			OutputConsoleTextWithColor("All evaluations failed: " + std::to_string(EvaluationsFailedCount) + " out of " + std::to_string(EvaluationsTotalCount), 255, 0, 0);

		}

		EvaluationsTotalCount = 0;
		EvaluationsFailedCount = 0;
	}

	if (bConvertEvaluationToUsableScript)
	{
		std::cout << "Requested convertions of evaluations to usable scripts with actual values:" << std::endl;
		for (const auto& Script : SavedConvertionsOfEvaluationToUsableScript)
		{
			OutputConsoleTextWithColor(Script, 0, 255, 255);
		}
		std::cout << std::endl;
	}

	SavedConvertionsOfEvaluationToUsableScript.clear();
}

std::vector<ConsoleJob*> ConsoleJobManager::ConvertCommandAction(CommandLineAction ActionToParse)
{
	std::vector<ConsoleJob*> Result;

	std::transform(ActionToParse.Action.begin(), ActionToParse.Action.end(), ActionToParse.Action.begin(), [](unsigned char c) { return std::tolower(c); });

	if (ActionToParse.Action == "load")
	{
		if (ActionToParse.Settings.find("filepath") != ActionToParse.Settings.end())
		{
			Result.push_back(new FileLoadJob(ActionToParse.Settings["filepath"]));
		}
	}
	else if (ActionToParse.Action == "save")
	{
		if (ActionToParse.Settings.find("filepath") != ActionToParse.Settings.end())
		{
			Result.push_back(new FileSaveJob(ActionToParse.Settings["filepath"]));
		}
	}
	else if (ActionToParse.Action == "complexity")
	{
		if (ActionToParse.Settings.find("type") == ActionToParse.Settings.end())
			return Result;

		std::string Type = ActionToParse.Settings["type"];
		std::transform(Type.begin(), Type.end(), Type.begin(), [](unsigned char c) { return std::toupper(c); });

		auto Iterator = ActionToParse.Settings.begin();
		while (Iterator != ActionToParse.Settings.end())
		{
			std::transform(Iterator->second.begin(), Iterator->second.end(), Iterator->second.begin(), [](unsigned char c) { return std::toupper(c); });
			Iterator++;
		}

		ComplexityJob* NewJobToAdd = new ComplexityJob();
		NewJobToAdd->ComplexityType = Type;

		if (ActionToParse.Settings.find("resolution") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetResolutionInM(std::stof(ActionToParse.Settings["resolution"]));
		}

		if (ActionToParse.Settings.find("relative_resolution") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetRelativeResolution(std::stof(ActionToParse.Settings["relative_resolution"]));
		}

		if (ActionToParse.Settings.find("jitter_quality") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetJitterQuality(ActionToParse.Settings["jitter_quality"]);
		}

		if (ActionToParse.Settings.find("run_on_whole_model") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetRunOnWholeModel(ActionToParse.Settings["run_on_whole_model"] == "TRUE" ? true : false);
		}

		if (ActionToParse.Settings.find("rugosity_algorithm") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetRugosity_Algorithm(ActionToParse.Settings["rugosity_algorithm"]);
		}

		if (ActionToParse.Settings.find("rugosity_min_algorithm_quality") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetRugosity_MinAlgorithm_Quality(ActionToParse.Settings["rugosity_min_algorithm_quality"]);
		}

		if (ActionToParse.Settings.find("rugosity_is_using_unique_projected_area") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetRugosity_IsUsingUniqueProjectedArea(ActionToParse.Settings["rugosity_is_using_unique_projected_area"] == "TRUE" ? true : false);
		}

		if (ActionToParse.Settings.find("rugosity_delete_outliers") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetRugosity_DeleteOutliers(ActionToParse.Settings["rugosity_delete_outliers"] == "TRUE" ? true : false);
		}

		if (ActionToParse.Settings.find("fractal_dimension_should_filter_values") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetFractalDimension_ShouldFilterValues(ActionToParse.Settings["fractal_dimension_should_filter_values"] == "TRUE" ? true : false);
		}

		if (ActionToParse.Settings.find("triangle_edges_mode") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetTriangleEdges_Mode(ActionToParse.Settings["triangle_edges_mode"]);
		}

		if (ActionToParse.Settings.find("is_standard_deviation_needed") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetIsStandardDeviationNeeded(ActionToParse.Settings["is_standard_deviation_needed"] == "TRUE" ? true : false);
		}

		if (ActionToParse.Settings.find("compare_first_layer_index") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetCompare_FirstLayerIndex(std::stoi(ActionToParse.Settings["compare_first_layer_index"]));
		}

		if (ActionToParse.Settings.find("compare_second_layer_index") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetCompare_SecondLayerIndex(std::stoi(ActionToParse.Settings["compare_second_layer_index"]));
		}

		if (ActionToParse.Settings.find("compare_normalize") != ActionToParse.Settings.end())
		{
			NewJobToAdd->Settings.SetCompare_Normalize(ActionToParse.Settings["compare_normalize"] == "TRUE" ? true : false);
		}

		Result.push_back(NewJobToAdd);
	}
	else if (ActionToParse.Action == "evaluation")
	{
		if (ActionToParse.Settings.find("type") == ActionToParse.Settings.end())
			return Result;

		if (ActionToParse.Settings.find("subtype") == ActionToParse.Settings.end())
			return Result;

		std::string Type = ActionToParse.Settings["type"];
		std::transform(Type.begin(), Type.end(), Type.begin(), [](unsigned char c) { return std::toupper(c); });

		auto Iterator = ActionToParse.Settings.begin();
		while (Iterator != ActionToParse.Settings.end())
		{
			std::transform(Iterator->second.begin(), Iterator->second.end(), Iterator->second.begin(), [](unsigned char c) { return std::toupper(c); });
			Iterator++;
		}

		if (ActionToParse.Settings["type"] == "COMPLEXITY")
		{
			ComplexityEvaluationJob* NewJobToAdd = new ComplexityEvaluationJob();

			NewJobToAdd->SetEvaluationSubType(ActionToParse.Settings["subtype"]);

			if (ActionToParse.Settings.find("expected_value") != ActionToParse.Settings.end())
			{
				NewJobToAdd->SetExpectedValue(std::stof(ActionToParse.Settings["expected_value"]));
			}

			if (ActionToParse.Settings.find("tolerance") != ActionToParse.Settings.end())
			{
				NewJobToAdd->SetTolerance(std::stof(ActionToParse.Settings["tolerance"]));
			}

			if (ActionToParse.Settings.find("layer_index") != ActionToParse.Settings.end())
			{
				NewJobToAdd->SetLayerIndex(std::stoi(ActionToParse.Settings["layer_index"]));
			}

			Result.push_back(NewJobToAdd);
		}
	}
	else if (ActionToParse.Action == "run_script_file")
	{
		if (ActionToParse.Settings.find("filepath") != ActionToParse.Settings.end())
		{
			std::string FilePath = ActionToParse.Settings["filepath"];
			if (FILE_SYSTEM.CheckFile(FilePath.c_str()))
			{
				std::ifstream File(FilePath);
				std::string Line;
				while (std::getline(File, Line))
				{
					std::vector<CommandLineAction> Actions = APPLICATION.ParseCommandLine(Line);
					std::vector<ConsoleJob*> NewJobs = ConvertCommandAction(Actions);
					for (size_t i = 0; i < NewJobs.size(); i++)
					{
						Result.push_back(NewJobs[i]);
					}
				}

				File.close();
				OutputConsoleTextWithColor("Script file read successfully. Jobs added to the queue: " + std::to_string(Result.size()), 0, 255, 0);
			}
			else
			{
				std::string ErrorMessage = "Error: File not found - " + FilePath + ". Please check the file path and try again.";
				LOG.Add(ErrorMessage, "CONSOLE_LOG");
				OutputConsoleTextWithColor(ErrorMessage, 255, 0, 0);
			}
		}
	}
	else if (ActionToParse.Action == "global_settings")
	{
		if (ActionToParse.Settings.find("type") == ActionToParse.Settings.end())
			return Result;

		auto Iterator = ActionToParse.Settings.begin();
		while (Iterator != ActionToParse.Settings.end())
		{
			std::transform(Iterator->second.begin(), Iterator->second.end(), Iterator->second.begin(), [](unsigned char c) { return std::toupper(c); });
			Iterator++;
		}

		GlobalSettingJob* NewJobToAdd = new GlobalSettingJob();
		NewJobToAdd->SetGlobalSettingType(ActionToParse.Settings["type"]);

		if (ActionToParse.Settings.find("int_value") != ActionToParse.Settings.end())
		{
			NewJobToAdd->SetIntValue(std::stoi(ActionToParse.Settings["int_value"]));
		}

		if (ActionToParse.Settings.find("float_value") != ActionToParse.Settings.end())
		{
			NewJobToAdd->SetFloatValue(std::stof(ActionToParse.Settings["float_value"]));
		}

		if (ActionToParse.Settings.find("bool_value") != ActionToParse.Settings.end())
		{
			NewJobToAdd->SetBoolValue(ActionToParse.Settings["bool_value"] == "TRUE" ? true : false);
		}

		Result.push_back(NewJobToAdd);
	}
	else if (ActionToParse.Action == "export_layer_as_image")
	{
		ExportLayerAsImageJob* NewJobToAdd = new ExportLayerAsImageJob();

		auto Iterator = ActionToParse.Settings.begin();
		while (Iterator != ActionToParse.Settings.end())
		{
			if (Iterator->first == "filepath")
			{
				Iterator++;
				continue;
			}

			std::transform(Iterator->second.begin(), Iterator->second.end(), Iterator->second.begin(), [](unsigned char c) { return std::toupper(c); });
			Iterator++;
		}

		if (ActionToParse.Settings.find("export_mode") != ActionToParse.Settings.end())
		{
			std::string ExportMode = ActionToParse.Settings["export_mode"];
			if (ExportMode == "MIN") NewJobToAdd->SetExportMode(LayerRasterizationManager::GridRasterizationMode::Min);
			if (ExportMode == "MAX") NewJobToAdd->SetExportMode(LayerRasterizationManager::GridRasterizationMode::Max);
			if (ExportMode == "MEAN") NewJobToAdd->SetExportMode(LayerRasterizationManager::GridRasterizationMode::Mean);
			if (ExportMode == "CUMULATIVE") NewJobToAdd->SetExportMode(LayerRasterizationManager::GridRasterizationMode::Cumulative);
		}

		if (ActionToParse.Settings.find("save_mode") != ActionToParse.Settings.end())
		{
			std::string SaveMode = ActionToParse.Settings["save_mode"];
			if (SaveMode == "PNG") NewJobToAdd->SetSaveMode(LayerRasterizationManager::SaveMode::SaveAsPNG);
			if (SaveMode == "GEOTIF") NewJobToAdd->SetSaveMode(LayerRasterizationManager::SaveMode::SaveAsTIF);
			if (SaveMode == "GEOTIF_32_BITS") NewJobToAdd->SetSaveMode(LayerRasterizationManager::SaveMode::SaveAs32bitTIF);
		}

		if (ActionToParse.Settings.find("filepath") != ActionToParse.Settings.end())
		{
			NewJobToAdd->SetFilePath(ActionToParse.Settings["filepath"]);
		}

		if (ActionToParse.Settings.find("resolution") != ActionToParse.Settings.end())
		{
			NewJobToAdd->SetResolutionInM(std::stof(ActionToParse.Settings["resolution"]));
		}

		if (ActionToParse.Settings.find("layer_index") != ActionToParse.Settings.end())
		{
			NewJobToAdd->SetLayerIndex(std::stoi(ActionToParse.Settings["layer_index"]));
		}

		if (ActionToParse.Settings.find("force_projection_vector") != ActionToParse.Settings.end())
		{
			std::string ForceProjectionVector = ActionToParse.Settings["force_projection_vector"];
			if (ForceProjectionVector == "X") NewJobToAdd->SetForceProjectionVector(glm::vec3(1.0f, 0.0f, 0.0f));
			if (ForceProjectionVector == "Y") NewJobToAdd->SetForceProjectionVector(glm::vec3(0.0f, 1.0f, 0.0f));
			if (ForceProjectionVector == "Z") NewJobToAdd->SetForceProjectionVector(glm::vec3(0.0f, 0.0f, 1.0f));
		}

		if (ActionToParse.Settings.find("persent_of_area_that_would_be_red") != ActionToParse.Settings.end())
		{
			NewJobToAdd->SetPersentOfAreaThatWouldBeRed(std::stof(ActionToParse.Settings["persent_of_area_that_would_be_red"]));
		}

		Result.push_back(NewJobToAdd);
	}
	else if (ActionToParse.Action == "help")
	{
		std::string CommandName;
		if (ActionToParse.Settings.find("command_name") != ActionToParse.Settings.end())
			CommandName = ActionToParse.Settings["command_name"];
		
		HelpJob* NewJobToAdd = new HelpJob(CommandName);
		Result.push_back(NewJobToAdd);
	}
	else if (ActionToParse.Action == "exit")
	{
		ConsoleJob* NewJobToAdd = new ConsoleJob();
		NewJobToAdd->Type = "EXIT_JOB";
		Result.push_back(NewJobToAdd);
	}

	return Result;
}

std::vector<ConsoleJob*> ConsoleJobManager::ConvertCommandAction(std::vector<CommandLineAction> Actions)
{
	std::vector<ConsoleJob*> Result;

	for (size_t i = 0; i < Actions.size(); i++)
	{
		std::vector<ConsoleJob*> NewJobs = ConvertCommandAction(Actions[i]);
		for (size_t j = 0; j < NewJobs.size(); j++)
		{
			Result.push_back(NewJobs[j]);
		}
	}

	return Result;
}

void ConsoleJobManager::OutputConsoleTextWithColor(std::string Text, int R, int G, int B)
{
	APPLICATION.GetConsoleWindow()->SetNearestConsoleTextColor(R, G, B);
	std::cout << Text << std::endl;
	APPLICATION.GetConsoleWindow()->SetNearestConsoleTextColor(255, 255, 255);
}