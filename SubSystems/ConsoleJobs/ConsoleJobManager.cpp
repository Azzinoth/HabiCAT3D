#include "ConsoleJobManager.h"
using namespace FocalEngine;

ConsoleJobManager* ConsoleJobManager::Instance = nullptr;

ConsoleJobManager::ConsoleJobManager()
{
	ConsoleJobSettingsInfo CurrentSettingInfo;

	ConsoleJobsInfo["help"] = HelpJob::GetInfo();
	ConsoleJobsInfo["file_load"] = FileLoadJob::GetInfo();
	ConsoleJobsInfo["file_save"] = FileSaveJob::GetInfo();

	ConsoleJobsInfo["run_script_file"].CommandName = "run_script_file";
	ConsoleJobsInfo["run_script_file"].Purpose = "Executes a sequence of commands from a specified script(text) file. Each command in the file should be on a new line.";
	CurrentSettingInfo = ConsoleJobSettingsInfo();
	CurrentSettingInfo.Name = "filepath";
	CurrentSettingInfo.Description = "The path of the script file to execute.";
	CurrentSettingInfo.bIsOptional = false;
	ConsoleJobsInfo["run_script_file"].SettingsInfo.push_back(CurrentSettingInfo);

	ConsoleJobsInfo["complexity"] = ComplexityJob::GetInfo();
	ConsoleJobsInfo["evaluation"] = ComplexityEvaluationJob::GetInfo();
	ConsoleJobsInfo["global_settings"] = GlobalSettingJob::GetInfo();
	ConsoleJobsInfo["export_layer_as_image"] = ExportLayerAsImageJob::GetInfo();
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
	std::transform(CommandName.begin(), CommandName.end(), CommandName.begin(), [](unsigned char c) { return std::tolower(c); });

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
			"-evaluation type=[TYPE] subtype=[WHAT_TO_TEST]  Create an evaluation job with the specified settings to test a layer or other objects.\n"
			"-global_settings type=[TYPE]                    Set a global setting for the application.\n"
			"-export_layer_as_image export_mode=[MODE]       Export a layer as an image.\n\n"

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
	if (!Job->Execute(nullptr, nullptr))
	{
		// Handle error, if type of job is critical, clear all jobs.
	}

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

	if (ActionToParse.Action == "run_script_file")
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

	ConsoleJob* NewJobToAdd = nullptr;

	if (ActionToParse.Action == "load")
	{
		NewJobToAdd = FileLoadJob::CreateInstance(ActionToParse);
	}
	else if (ActionToParse.Action == "save")
	{
		NewJobToAdd = FileSaveJob::CreateInstance(ActionToParse);
	}
	else if (ActionToParse.Action == "complexity")
	{
		NewJobToAdd = ComplexityJob::CreateInstance(ActionToParse);
	}
	else if (ActionToParse.Action == "evaluation")
	{
		NewJobToAdd = ComplexityEvaluationJob::CreateInstance(ActionToParse);
	}
	else if (ActionToParse.Action == "global_settings")
	{
		NewJobToAdd = GlobalSettingJob::CreateInstance(ActionToParse);
	}
	else if (ActionToParse.Action == "export_layer_as_image")
	{
		NewJobToAdd = ExportLayerAsImageJob::CreateInstance(ActionToParse);
	}
	else if (ActionToParse.Action == "help")
	{
		NewJobToAdd = HelpJob::CreateInstance(ActionToParse);
	}
	else if (ActionToParse.Action == "exit")
	{
		NewJobToAdd = ExitJob::CreateInstance(ActionToParse);
	}

	if (NewJobToAdd != nullptr)
		Result.push_back(NewJobToAdd);

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