To use the application in command line interface (CLI) mode, the user can set `-console` as an application argument. After that, they can start inputting all other commands listed below. Alternatively, the user can proceed with commands in the original argument list, such as:

```bash
-console -NEXT_COMMAND -NEXT ...
```

# Command signature:
  `-[COMMAND] [OPTIONS]=[VALUE]`

## Commands:

`-help`

Purpose:

Prints help for all commands or for a specific command.

Settings:
  - `command_name` (Optional): The name of the command to print help for.
      Default: (prints help for all commands)  
---
`-file_load`

Purpose:

Loads a file from the specified path.

Settings:
  - filepath (Required): The path of the file to load.
---
`-file_save`

Purpose:

Saves the current state to a file at the specified path.

Settings:
  - `filepath` (Required): The path of the file to save.
    
---
`-run_script_file`                

Purpose:

Execute a sequence of commands from a specified script(text) file. Each command in the file should be on a new line.

Settings:
  - `filepath` (Required): The path of the script file to execute.
---
`-complexity`                   

Purpose:

Creates a job to add complexity layer of a model based on the specified type.

Settings:
  - `type` (Required): Specifies the type of complexity calculation.
    Possible Values: `'HEIGHT'`, `'AREA'`, `'RUGOSITY'`, `'TRIANGLE_EDGE'`, `'TRIANGLE_COUNT'`, `'VECTOR_DISPERSION'`, `'FRACTAL_DIMENSION'`, `'COMPARE'`

  - `resolution` (Optional): Specifies the resolution in meters for the complexity calculation. Alternative to relative_resolution.
    Default: `Minimal possible`

  - `relative_resolution` (Optional): Specifies the resolution as a float between `0.0` and `1.0`, where `0.0` represents the lowest possible resolution and `1.0` represents the highest. Alternative to resolution.
    Default: `0.0`

  - `jitter_quality` (Optional): Specifies the quality of jitter applied to the model. Higher values mean more jitters and potentially smoother results but slower processing.
    Possible Values: `1`, `7`, `13`, `25`, `37`, `55`, `73`
    Default: `55`

  - `run_on_whole_model` (Optional): Specifies if the calculation should be run on the whole model without jitter.
    Default: `"false"`

  - `triangle_edges_mode` (Optional): Specifies the mode of triangle edges calculation. Relevant only for `'TRIANGLE_EDGE'` complexity type.
    Default: `MAX_LEHGTH`

  - `rugosity_algorithm` (Optional): Specifies the algorithm for rugosity calculation. Relevant only for `'RUGOSITY'` complexity type.
    Possible Values: `'AVERAGE'`, `'MIN'`, `'LSF(CGAL)'`
    Default: `MIN`

  - `rugosity_is_using_unique_projected_area` (Optional): Specifies if the unique projected area should be used for rugosity calculation. Relevant only for `'RUGOSITY'` complexity type.
    Default: `"false"`

  - `rugosity_is_unique_projected_area_approximated` (Optional): Specifies if the approximation should be used for unique projected area rugosity calculation(Speeds Up by Over 100x). Relevant only for 'RUGOSITY' complexity type.
    Default: `"true"`

  - `rugosity_delete_outliers` (Optional): Specifies if the outliers should be deleted from the rugosity calculation. Relevant only for `'RUGOSITY'` complexity type.
    Default: `"false"`

  - `rugosity_min_algorithm_quality` (Optional): Specifies the quality of the rugosity calculation.
    Relevant only for `'RUGOSITY'` complexity type and when the rugosity_algorithm is set to `'MIN'`.
    Possible Values: `1`, `9`, `19`, `33`, `51`, `73`, `91`, `99`, `129`, `163`, `201`, `289`, `339`, `393`, `441`
    Default: `91`

  - `fractal_dimension_should_filter_values` (Optional): Specifies if the app should filter values that are less that `2.0`. Relevant only for `'FRACTAL_DIMENSION'` complexity type.
    Default: `"true"`

  - `is_standard_deviation_needed` (Optional): Specifies if the app should also add layer with standard deviation.
    Default: `"false"`

  - `compare_first_layer_index` (Required): Specifies the index of the first layer to compare. Relevant only for `'COMPARE'` complexity type.

  - `compare_second_layer_index` (Required): Specifies the index of the second layer to compare. Relevant only for `'COMPARE'` complexity type.

  - `compare_normalize` (Optional): Specifies if the app should normalize the layers before comparing. Relevant only for `'COMPARE'` complexity type.
    Default: `"true"`
---
`-evaluation`

Purpose:

Creates an evaluation job with the specified settings to test a layer or other objects.

Settings:
  - `type` (Required): Specifies the type of evaluation.
    Possible Values: `'COMPLEXITY'`

  - `subtype` (Required): Specifies the subtype of evaluation.
    Possible Values: `'MEAN_LAYER_VALUE'`, `'MEDIAN_LAYER_VALUE'`, `'MAX_LAYER_VALUE'`, `'MIN_LAYER_VALUE'`

  - `expected_value` (Required): Specifies the expected value for the evaluation.

  - `tolerance` (Required): Specifies the tolerance for the evaluation.

  - `layer_index` (Optional): Specifies the index of the layer to evaluate. Relevant only for `'COMPLEXITY'` evaluation type.
    Default: `'-1'` Which means the last layer.

  - `convert_to_script` (Optional): Specifies if the job should be converted to a script that later can be used to run the same job but with actual values.(Mostly used to make it easier to create a script file for new models)
    Default: `"false"`
---
`-global_settings`

Purpose:

Sets a global setting for the application.

Settings:
  - `type` (Required): Specifies the type of global setting.
    Possible Values: `'EVALUATION_JOB_TO_SCRIPT'`, `'OUTPUT_LOG_TO_FILE'`

  - `int_value` (Optional): Specifies the integer value for the global setting.
    Default: `0`

  - `float_value` (Optional): Specifies the float value for the global setting.
    Default: `0.0`

  - `bool_value` (Optional): Specifies the boolean value for the global setting.
    Default: `"false"`
---
`-export_layer_as_image`

Purpose:

Exports a layer as an image.

Settings:
  - `export_mode` (Required): Specifies the mode of the export.
    Possible Values: `'MIN'`, `'MAX'`, `'MEAN'`, `'CUMULATIVE'`

  - `save_mode` (Required): Specifies the type of image file.
    Possible Values: `'PNG'`, `'GEOTIF'`, `'GEOTIF_32_BITS'`

  - `filepath` (Required): Specifies the path of the file to save.

  - `resolution` (Required): Specifies the resolution in meters for the image.

  - `resolution_in_pixels` (Optional): Specifies the resolution in pixels for the image.
    Default:

  - `layer_index` (Required): Specifies the index of the layer to export.

  - `force_projection_vector` (Optional): Specifies the projection vector for the image.
    Possible Values: `'X'`, `'Y'`, `'Z'`
    Default: `Calculated on fly`

  - `persent_of_area_that_would_be_red` (Optional): Specifies the persent of area that would be considered outliers and would be red.
    Default: `5.0`
---
`-query`

Purpose:

With this command, users can query information to be outputted to the console or log.

Settings:
  - `request` (Required): Specifies what to query.
    Possible Values: `'EVALUATION_SUMMARY'`
---

## Examples:
```
-load filepath="C:/data/mesh.obj"
-save filepath="C:/data/processed_mesh.rug"
-complexity type=RUGOSITY rugosity_algorithm=MIN jitter_quality=73
-evaluation type=COMPLEXITY subtype=MAX_LAYER_VALUE expected_value=5.02 tolerance=0.01
```

## Notes:
- For Boolean settings, use `"true"` or `"false"`.
- Paths must be enclosed in quotes.