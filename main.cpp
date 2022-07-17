#include "SubSystems/FECGALWrapper.h"
using namespace FocalEngine;

static const char* const sTestVS = R"(
@In_Position@
@In_Normal@

layout (location = 8) in float RugosityData;

@In_Color@
@In_Segments_colors@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

uniform int colorMode;

out VS_OUT
{
	vec2 UV;
	vec3 worldPosition;
	vec4 viewPosition;
	mat3 TBN;
	vec3 vertexNormal;
	float materialIndex;

	vec3 color;
	vec3 segmentsColors;

	float Rugosity;
} vs_out;

void main(void)
{
	//gl_Position = ProjectionMatrix * vec4(vPos, 1.0);

	vec4 worldPosition = FEWorldMatrix * vec4(FEPosition, 1.0);
	vs_out.worldPosition = worldPosition.xyz;
	vs_out.viewPosition = FEViewMatrix * worldPosition;
	gl_Position = FEProjectionMatrix * vs_out.viewPosition;

	vs_out.vertexNormal = normalize(vec3(FEWorldMatrix * vec4(FENormal, 0.0)));

	if (colorMode == 1)
		vs_out.color = FEColor;

	if (colorMode == 2)
		vs_out.segmentsColors = FESegmentsColors;

	vs_out.Rugosity = RugosityData;
}
)";

static const char* const sTestFS = R"(
in VS_OUT
{
	vec2 UV;
	vec3 worldPosition;
	vec4 viewPosition;
	mat3 TBN;
	vec3 vertexNormal;
	flat float materialIndex;

	flat vec3 color;
	flat vec3 segmentsColors;

	float Rugosity;
} FS_IN;

@ViewMatrix@
@ProjectionMatrix@

uniform int colorMode;
uniform vec3 lightDirection;

uniform float minRugorsity;
uniform float maxRugorsity;

layout (location = 0) out vec4 out_Color;

// Copyright 2019 Google LLC.
// SPDX-License-Identifier: Apache-2.0

// Author: Anton Mikhailov

// The look-up tables contains 256 entries. Each entry is a an sRGB triplet.
// From : https://gist.github.com/mikhailov-work/6a308c20e494d9e0ccc29036b28faa7a#file-turbo_colormap-c
vec3 getTurboColormapValue(float factor)
{
	float turbo_srgb_floats[256][3] = {{0.18995,0.07176,0.23217},{0.19483,0.08339,0.26149},{0.19956,0.09498,0.29024},{0.20415,0.10652,0.31844},{0.20860,0.11802,0.34607},{0.21291,0.12947,0.37314},{0.21708,0.14087,0.39964},{0.22111,0.15223,0.42558},{0.22500,0.16354,0.45096},{0.22875,0.17481,0.47578},{0.23236,0.18603,0.50004},{0.23582,0.19720,0.52373},{0.23915,0.20833,0.54686},{0.24234,0.21941,0.56942},{0.24539,0.23044,0.59142},{0.24830,0.24143,0.61286},{0.25107,0.25237,0.63374},{0.25369,0.26327,0.65406},{0.25618,0.27412,0.67381},{0.25853,0.28492,0.69300},{0.26074,0.29568,0.71162},{0.26280,0.30639,0.72968},{0.26473,0.31706,0.74718},{0.26652,0.32768,0.76412},{0.26816,0.33825,0.78050},{0.26967,0.34878,0.79631},{0.27103,0.35926,0.81156},{0.27226,0.36970,0.82624},{0.27334,0.38008,0.84037},{0.27429,0.39043,0.85393},{0.27509,0.40072,0.86692},{0.27576,0.41097,0.87936},{0.27628,0.42118,0.89123},{0.27667,0.43134,0.90254},{0.27691,0.44145,0.91328},{0.27701,0.45152,0.92347},{0.27698,0.46153,0.93309},{0.27680,0.47151,0.94214},{0.27648,0.48144,0.95064},{0.27603,0.49132,0.95857},{0.27543,0.50115,0.96594},{0.27469,0.51094,0.97275},{0.27381,0.52069,0.97899},{0.27273,0.53040,0.98461},{0.27106,0.54015,0.98930},{0.26878,0.54995,0.99303},{0.26592,0.55979,0.99583},{0.26252,0.56967,0.99773},{0.25862,0.57958,0.99876},{0.25425,0.58950,0.99896},{0.24946,0.59943,0.99835},{0.24427,0.60937,0.99697},{0.23874,0.61931,0.99485},{0.23288,0.62923,0.99202},{0.22676,0.63913,0.98851},{0.22039,0.64901,0.98436},{0.21382,0.65886,0.97959},{0.20708,0.66866,0.97423},{0.20021,0.67842,0.96833},{0.19326,0.68812,0.96190},{0.18625,0.69775,0.95498},{0.17923,0.70732,0.94761},{0.17223,0.71680,0.93981},{0.16529,0.72620,0.93161},{0.15844,0.73551,0.92305},{0.15173,0.74472,0.91416},{0.14519,0.75381,0.90496},{0.13886,0.76279,0.89550},{0.13278,0.77165,0.88580},{0.12698,0.78037,0.87590},{0.12151,0.78896,0.86581},{0.11639,0.79740,0.85559},{0.11167,0.80569,0.84525},{0.10738,0.81381,0.83484},{0.10357,0.82177,0.82437},{0.10026,0.82955,0.81389},{0.09750,0.83714,0.80342},{0.09532,0.84455,0.79299},{0.09377,0.85175,0.78264},{0.09287,0.85875,0.77240},{0.09267,0.86554,0.76230},{0.09320,0.87211,0.75237},{0.09451,0.87844,0.74265},{0.09662,0.88454,0.73316},{0.09958,0.89040,0.72393},{0.10342,0.89600,0.71500},{0.10815,0.90142,0.70599},{0.11374,0.90673,0.69651},{0.12014,0.91193,0.68660},{0.12733,0.91701,0.67627},{0.13526,0.92197,0.66556},{0.14391,0.92680,0.65448},{0.15323,0.93151,0.64308},{0.16319,0.93609,0.63137},{0.17377,0.94053,0.61938},{0.18491,0.94484,0.60713},{0.19659,0.94901,0.59466},{0.20877,0.95304,0.58199},{0.22142,0.95692,0.56914},{0.23449,0.96065,0.55614},{0.24797,0.96423,0.54303},{0.26180,0.96765,0.52981},{0.27597,0.97092,0.51653},{0.29042,0.97403,0.50321},{0.30513,0.97697,0.48987},{0.32006,0.97974,0.47654},{0.33517,0.98234,0.46325},{0.35043,0.98477,0.45002},{0.36581,0.98702,0.43688},{0.38127,0.98909,0.42386},{0.39678,0.99098,0.41098},{0.41229,0.99268,0.39826},{0.42778,0.99419,0.38575},{0.44321,0.99551,0.37345},{0.45854,0.99663,0.36140},{0.47375,0.99755,0.34963},{0.48879,0.99828,0.33816},{0.50362,0.99879,0.32701},{0.51822,0.99910,0.31622},{0.53255,0.99919,0.30581},{0.54658,0.99907,0.29581},{0.56026,0.99873,0.28623},{0.57357,0.99817,0.27712},{0.58646,0.99739,0.26849},{0.59891,0.99638,0.26038},{0.61088,0.99514,0.25280},{0.62233,0.99366,0.24579},{0.63323,0.99195,0.23937},{0.64362,0.98999,0.23356},{0.65394,0.98775,0.22835},{0.66428,0.98524,0.22370},{0.67462,0.98246,0.21960},{0.68494,0.97941,0.21602},{0.69525,0.97610,0.21294},{0.70553,0.97255,0.21032},{0.71577,0.96875,0.20815},{0.72596,0.96470,0.20640},{0.73610,0.96043,0.20504},{0.74617,0.95593,0.20406},{0.75617,0.95121,0.20343},{0.76608,0.94627,0.20311},{0.77591,0.94113,0.20310},{0.78563,0.93579,0.20336},{0.79524,0.93025,0.20386},{0.80473,0.92452,0.20459},{0.81410,0.91861,0.20552},{0.82333,0.91253,0.20663},{0.83241,0.90627,0.20788},{0.84133,0.89986,0.20926},{0.85010,0.89328,0.21074},{0.85868,0.88655,0.21230},{0.86709,0.87968,0.21391},{0.87530,0.87267,0.21555},{0.88331,0.86553,0.21719},{0.89112,0.85826,0.21880},{0.89870,0.85087,0.22038},{0.90605,0.84337,0.22188},{0.91317,0.83576,0.22328},{0.92004,0.82806,0.22456},{0.92666,0.82025,0.22570},{0.93301,0.81236,0.22667},{0.93909,0.80439,0.22744},{0.94489,0.79634,0.22800},{0.95039,0.78823,0.22831},{0.95560,0.78005,0.22836},{0.96049,0.77181,0.22811},{0.96507,0.76352,0.22754},{0.96931,0.75519,0.22663},{0.97323,0.74682,0.22536},{0.97679,0.73842,0.22369},{0.98000,0.73000,0.22161},{0.98289,0.72140,0.21918},{0.98549,0.71250,0.21650},{0.98781,0.70330,0.21358},{0.98986,0.69382,0.21043},{0.99163,0.68408,0.20706},{0.99314,0.67408,0.20348},{0.99438,0.66386,0.19971},{0.99535,0.65341,0.19577},{0.99607,0.64277,0.19165},{0.99654,0.63193,0.18738},{0.99675,0.62093,0.18297},{0.99672,0.60977,0.17842},{0.99644,0.59846,0.17376},{0.99593,0.58703,0.16899},{0.99517,0.57549,0.16412},{0.99419,0.56386,0.15918},{0.99297,0.55214,0.15417},{0.99153,0.54036,0.14910},{0.98987,0.52854,0.14398},{0.98799,0.51667,0.13883},{0.98590,0.50479,0.13367},{0.98360,0.49291,0.12849},{0.98108,0.48104,0.12332},{0.97837,0.46920,0.11817},{0.97545,0.45740,0.11305},{0.97234,0.44565,0.10797},{0.96904,0.43399,0.10294},{0.96555,0.42241,0.09798},{0.96187,0.41093,0.09310},{0.95801,0.39958,0.08831},{0.95398,0.38836,0.08362},{0.94977,0.37729,0.07905},{0.94538,0.36638,0.07461},{0.94084,0.35566,0.07031},{0.93612,0.34513,0.06616},{0.93125,0.33482,0.06218},{0.92623,0.32473,0.05837},{0.92105,0.31489,0.05475},{0.91572,0.30530,0.05134},{0.91024,0.29599,0.04814},{0.90463,0.28696,0.04516},{0.89888,0.27824,0.04243},{0.89298,0.26981,0.03993},{0.88691,0.26152,0.03753},{0.88066,0.25334,0.03521},{0.87422,0.24526,0.03297},{0.86760,0.23730,0.03082},{0.86079,0.22945,0.02875},{0.85380,0.22170,0.02677},{0.84662,0.21407,0.02487},{0.83926,0.20654,0.02305},{0.83172,0.19912,0.02131},{0.82399,0.19182,0.01966},{0.81608,0.18462,0.01809},{0.80799,0.17753,0.01660},{0.79971,0.17055,0.01520},{0.79125,0.16368,0.01387},{0.78260,0.15693,0.01264},{0.77377,0.15028,0.01148},{0.76476,0.14374,0.01041},{0.75556,0.13731,0.00942},{0.74617,0.13098,0.00851},{0.73661,0.12477,0.00769},{0.72686,0.11867,0.00695},{0.71692,0.11268,0.00629},{0.70680,0.10680,0.00571},{0.69650,0.10102,0.00522},{0.68602,0.09536,0.00481},{0.67535,0.08980,0.00449},{0.66449,0.08436,0.00424},{0.65345,0.07902,0.00408},{0.64223,0.07380,0.00401},{0.63082,0.06868,0.00401},{0.61923,0.06367,0.00410},{0.60746,0.05878,0.00427},{0.59550,0.05399,0.00453},{0.58336,0.04931,0.00486},{0.57103,0.04474,0.00529},{0.55852,0.04028,0.00579},{0.54583,0.03593,0.00638},{0.53295,0.03169,0.00705},{0.51989,0.02756,0.00780},{0.50664,0.02354,0.00863},{0.49321,0.01963,0.00955},{0.47960,0.01583,0.01055}};

	int index = int(255 * factor);

	return vec3(turbo_srgb_floats[index][0], turbo_srgb_floats[index][1], turbo_srgb_floats[index][2]);
}

vec3 getRainbowScaledColor(float factor)
{
	factor = 1.0 - factor;
    factor *= 6.0;
    int sextant = int(factor);
    float vsf = factor - sextant;
    float mid1 = vsf;
    float mid2 = 1.0 - vsf;
	
	vec3 result = vec3 (1, 0, 0);

    switch (sextant)
    {
		case 0:
				result.x = 1;
                result.y = 0;
                result.z = 0;
                break;
        case 1:
                result.x = 1;
                result.y = mid1;
                result.z = 0;
                break;
        case 2:
                result.x = mid2;
                result.y = 1;
                result.z = 0;
                break;
        case 3:
                result.x = 0;
                result.y = 1;
                result.z = mid1;
                break;
        case 4:
                result.x = 0;
                result.y = mid2;
                result.z = 1;
                break;
        case 5:
                result.x = mid1;
                result.y = 0;
                result.z = 1;
                break;
    }
  
    return result;
}

vec3 getScaledColor(float factor)
{
	vec3 darkBlue = vec3(0.0f, 0.0f, 0.4f);
	vec3 lightCyan = vec3(27.0f / 255.0f, 213.0f / 255.0f, 200.0f / 255.0f);
	vec3 green = vec3(0.0f / 255.0f, 255.0f / 255.0f, 64.0f / 255.0f);
	vec3 yellow = vec3(225.0f / 255.0f, 225.0f / 255.0f, 0.0f / 255.0f);
	vec3 red = vec3(225.0f / 255.0f, 0 / 255.0f, 0.0f / 255.0f);

	vec3 result;

	if (factor <= 0.125 && factor > 0.0)
	{
		float DistanceToLower = abs(factor - 0.0);
		float DistanceToUpper = abs(factor - 0.125);

		float DistanceToLowerCof = 1.0f - DistanceToLower / abs(0.125 - 0.0);
		float DistanceToUpperCof = 1.0f - DistanceToUpper / abs(0.125 - 0.0);

		vec3 mix = darkBlue * DistanceToLowerCof + lightCyan * DistanceToUpperCof;

		result = mix;
	}
	else if (factor <= 0.25 && factor > 0.125)
	{
		float DistanceToLower = abs(factor - 0.125);
		float DistanceToUpper = abs(factor - 0.25);

		float DistanceToLowerCof = 1.0f - DistanceToLower / abs(0.25 - 0.125);
		float DistanceToUpperCof = 1.0f - DistanceToUpper / abs(0.25 - 0.125);

		result = lightCyan * DistanceToLowerCof + green * DistanceToUpperCof;
	}
	else if (factor <= 0.5 && factor > 0.25)
	{
		float DistanceToLower = abs(factor - 0.25);
		float DistanceToUpper = abs(factor - 0.5);

		float DistanceToLowerCof = 1.0f - DistanceToLower / abs(0.5 - 0.25);
		float DistanceToUpperCof = 1.0f - DistanceToUpper / abs(0.5 - 0.25);

		result = green * DistanceToLowerCof + yellow * DistanceToUpperCof;
	}
	else if (factor <= 0.75 && factor > 0.5)
	{
		float DistanceToLower = abs(factor - 0.5);
		float DistanceToUpper = abs(factor - 0.75);

		float DistanceToLowerCof = 1.0f - DistanceToLower / abs(0.75 - 0.5);
		float DistanceToUpperCof = 1.0f - DistanceToUpper / abs(0.75 - 0.5);

		result = yellow * DistanceToLowerCof + red * DistanceToUpperCof;
	}
	else
	{
		result = red;
	}

	return result;
}

vec3 getCorrectColor()
{
	vec3 result = vec3(0.0, 0.5, 1.0);
	float normalizedRugorsity = (FS_IN.Rugosity - minRugorsity) / (maxRugorsity - minRugorsity);
	normalizedRugorsity = clamp(normalizedRugorsity, 0, 1);

	switch (colorMode)
    {
        case 2:
                result = FS_IN.segmentsColors;
                break;
        case 3:
				result = getScaledColor(normalizedRugorsity);
                break;
        case 4:
				result = getRainbowScaledColor(normalizedRugorsity);
                break;
		case 5:
				result = getTurboColormapValue(normalizedRugorsity);
                break;
    }

	return result;
}

void main(void)
{
	float diffuseFactor = max(dot(FS_IN.vertexNormal, lightDirection), 0.15);
	vec3 ambientColor = vec3(0.55f, 0.73f, 0.87f) * 2.8f;

	out_Color = vec4(ambientColor * diffuseFactor * getCorrectColor(), 1.0f);
}
)";


void showTransformConfiguration(std::string name, FETransformComponent* transform)
{
	// ********************* POSITION *********************
	glm::vec3 position = transform->getPosition();
	ImGui::Text("Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X pos : ") + name).c_str(), &position[0], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y pos : ") + name).c_str(), &position[1], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z pos : ") + name).c_str(), &position[2], 0.1f);
	transform->setPosition(position);

	// ********************* ROTATION *********************
	glm::vec3 rotation = transform->getRotation();
	ImGui::Text("Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X rot : ") + name).c_str(), &rotation[0], 0.1f, -360.0f, 360.0f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y rot : ") + name).c_str(), &rotation[1], 0.1f, -360.0f, 360.0f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z rot : ") + name).c_str(), &rotation[2], 0.1f, -360.0f, 360.0f);
	transform->setRotation(rotation);

	// ********************* SCALE *********************
	ImGui::Checkbox("Uniform scaling", &transform->uniformScaling);
	glm::vec3 scale = transform->getScale();
	ImGui::Text("Scale : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X scale : ") + name).c_str(), &scale[0], 0.01f, 0.01f, 1000.0f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y scale : ") + name).c_str(), &scale[1], 0.01f, 0.01f, 1000.0f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z scale : ") + name).c_str(), &scale[2], 0.01f, 0.01f, 1000.0f);

	glm::vec3 oldScale = transform->getScale();
	transform->changeXScaleBy(scale[0] - oldScale[0]);
	transform->changeYScaleBy(scale[1] - oldScale[1]);
	transform->changeZScaleBy(scale[2] - oldScale[2]);
}

void showCameraTransform(FEFreeCamera* camera)
{
	// ********* POSITION *********
	glm::vec3 cameraPosition = camera->getPosition();

	ImGui::Text("Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::DragFloat("##X pos", &cameraPosition[0], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::DragFloat("##Y pos", &cameraPosition[1], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::DragFloat("##Z pos", &cameraPosition[2], 0.1f);

	camera->setPosition(cameraPosition);

	// ********* ROTATION *********
	glm::vec3 cameraRotation = glm::vec3(camera->getYaw(), camera->getPitch(), camera->getRoll());

	ImGui::Text("Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::DragFloat("##X rot", &cameraRotation[0], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::DragFloat("##Y rot", &cameraRotation[1], 0.1f);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::DragFloat("##Z rot", &cameraRotation[2], 0.1f);

	camera->setYaw(cameraRotation[0]);
	camera->setPitch(cameraRotation[1]);
	camera->setRoll(cameraRotation[2]);

	float cameraSpeed = camera->getMovementSpeed();
	ImGui::Text("Camera speed in m/s : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(70);
	ImGui::DragFloat("##Camera_speed", &cameraSpeed, 0.01f, 0.01f, 100.0f);
	camera->setMovementSpeed(cameraSpeed);
}

FEFreeCamera* currentCamera = nullptr;
bool wireframeMode = false;
FEShader* meshShader = nullptr;

double mouseX;
double mouseY;

float TimeTookToJitter = 0.0f;
bool showTrianglesInCells = true;

int SDFRenderingMode = 0;

glm::dvec3 mouseRay(double mouseX, double mouseY)
{
	int W, H;
	APPLICATION.GetWindowSize(&W, &H);

	glm::dvec2 normalizedMouseCoords;
	normalizedMouseCoords.x = (2.0f * mouseX) / W - 1;
	normalizedMouseCoords.y = 1.0f - (2.0f * (mouseY)) / H;

	glm::dvec4 clipCoords = glm::dvec4(normalizedMouseCoords.x, normalizedMouseCoords.y, -1.0, 1.0);
	glm::dvec4 eyeCoords = glm::inverse(currentCamera->getProjectionMatrix()) * clipCoords;
	eyeCoords.z = -1.0f;
	eyeCoords.w = 0.0f;
	glm::dvec3 worldRay = glm::inverse(currentCamera->getViewMatrix()) * eyeCoords;
	worldRay = glm::normalize(worldRay);

	return worldRay;
}

void renderTargetCenterForCamera()
{
	int centerX, centerY = 0;
	int shiftX, shiftY = 0;

	int xpos, ypos;
	APPLICATION.GetWindowPosition(&xpos, &ypos);

	//if (renderTargetMode == FE_GLFW_MODE)
	//{
		int windowW, windowH = 0;
		APPLICATION.GetWindowSize(&windowW, &windowH);
		centerX = xpos + (windowW / 2);
		centerY = ypos + (windowH / 2);

		shiftX = xpos;
		shiftY = ypos;
	/*}
	else if (renderTargetMode == FE_CUSTOM_MODE)
	{
		centerX = xpos + renderTargetXShift + (renderTargetW / 2);
		centerY = ypos + renderTargetYShift + (renderTargetH / 2);

		shiftX = renderTargetXShift + xpos;
		shiftY = renderTargetYShift + ypos;
	}*/

	currentCamera->setRenderTargetCenterX(centerX);
	currentCamera->setRenderTargetCenterY(centerY);

	currentCamera->setRenderTargetShiftX(shiftX);
	currentCamera->setRenderTargetShiftY(shiftY);
}

void addLinesOFSDF(SDF* SDF)
{
	for (size_t i = 0; i < SDF->data.size(); i++)
	{
		for (size_t j = 0; j < SDF->data[i].size(); j++)
		{
			for (size_t k = 0; k < SDF->data[i][j].size(); k++)
			{
				bool render = false;
				SDF->data[i][j][k].wasRenderedLastFrame = false;

				if (!SDF->data[i][j][k].trianglesInCell.empty() || SDFRenderingMode == 2)
					render = true;

				if (render)
				{
					glm::vec3 color = glm::vec3(0.1f, 0.6f, 0.1f);
					if (SDF->data[i][j][k].selected)
						color = glm::vec3(0.9f, 0.1f, 0.1f);

					LINE_RENDERER.RenderAABB(SDF->data[i][j][k].AABB, color);

					SDF->data[i][j][k].wasRenderedLastFrame = true;

					if (showTrianglesInCells && SDF->data[i][j][k].selected)
					{
						for (size_t l = 0; l < SDF->data[i][j][k].trianglesInCell.size(); l++)
						{
							auto currentTriangle = SDF->mesh->Triangles[SDF->data[i][j][k].trianglesInCell[l]];
							
							LINE_RENDERER.AddLineToBuffer(FELine(currentTriangle[0], currentTriangle[1], glm::vec3(1.0f, 1.0f, 0.0f)));
							LINE_RENDERER.AddLineToBuffer(FELine(currentTriangle[0], currentTriangle[2], glm::vec3(1.0f, 1.0f, 0.0f)));
							LINE_RENDERER.AddLineToBuffer(FELine(currentTriangle[1], currentTriangle[2], glm::vec3(1.0f, 1.0f, 0.0f)));
						}
					}
				}
			}
		}
	}
}

FEMesh* loadedMesh = nullptr;
FEMesh* currentMesh = nullptr;
std::vector<std::string> dimentionsList;
std::vector<std::string> colorSchemesList;

std::string colorSchemeIndexToString(int index)
{
	switch (index)
	{
		case 3:
			return colorSchemesList[0];
		case 4:
			return colorSchemesList[1];
		case 5:
			return colorSchemesList[2];
	}

	return "Default";
}

int colorSchemeIndexFromString(std::string name)
{
	if (name == colorSchemesList[0])
		return 3;

	if (name == colorSchemesList[1])
		return 4;

	if (name == colorSchemesList[2])
		return 5;

	return 3;
};

static void dropCallback(int count, const char** paths);
void dropCallback(int count, const char** paths)
{
	for (size_t i = 0; i < size_t(count); i++)
	{
		if (FILE_SYSTEM.isFolder(paths[i]) && count == 1)
		{
			/*if (PROJECT_MANAGER.getCurrent() == nullptr)
			{
				PROJECT_MANAGER.setProjectsFolder(paths[i]);
			}*/
		}

		if (!FILE_SYSTEM.checkFile(paths[i]))
		{
			//LOG.add("Can't locate file: " + std::string(fileName) + " in FEResourceManager::importAsset", FE_LOG_ERROR, FE_LOG_LOADING);
			continue;
		}

		std::string fileExtention = FILE_SYSTEM.getFileExtension(paths[i]);
		if (fileExtention == ".obj")
		{
			loadedMesh = CGALWrapper.importOBJ(paths[i], true);
			currentMesh = loadedMesh;
		}

		//if (PROJECT_MANAGER.getCurrent() != nullptr)
		//{
		//	std::vector<FEObject*> loadedObjects = RESOURCE_MANAGER.importAsset(paths[i]);
		//	for (size_t i = 0; i < loadedObjects.size(); i++)
		//	{
		//		if (loadedObjects[i] != nullptr)
		//		{
		//			if (loadedObjects[i]->getType() == FE_ENTITY)
		//			{
		//				//SCENE.addEntity(reinterpret_cast<FEEntity*>(loadedObjects[i]));
		//			}
		//			else
		//			{
		//				VIRTUAL_FILE_SYSTEM.createFile(loadedObjects[i], VIRTUAL_FILE_SYSTEM.getCurrentPath());
		//				PROJECT_MANAGER.getCurrent()->setModified(true);
		//				PROJECT_MANAGER.getCurrent()->addUnSavedObject(loadedObjects[i]);
		//			}
		//		}
		//	}
		//}
	}
}

void mouseMoveCallback(double xpos, double ypos)
{
	if (currentCamera != nullptr)
		currentCamera->mouseMoveInput(xpos, ypos);

	mouseX = xpos;
	mouseY = ypos;
}

void keyButtonCallback(int key, int scancode, int action, int mods)
{
	currentCamera->keyboardInput(key, scancode, action, mods);
}

void mouseButtonCallback(int button, int action, int mods)
{
	if (ImGui::GetIO().WantCaptureMouse)
	{
		currentCamera->setIsInputActive(false);
		return;
	}

	if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS)
	{
		currentCamera->setIsInputActive(true);
	}
	else if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_RELEASE)
	{
		currentCamera->setIsInputActive(false);
	}

	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
	{
		if (currentMesh != nullptr)
		{
			if (SDFRenderingMode == 0)
			{
				currentMesh->SelectTriangle(mouseRay(mouseX, mouseY), currentCamera);
				LINE_RENDERER.clearAll();

				if (currentMesh->TriangleSelected != -1)
				{
					LINE_RENDERER.AddLineToBuffer(FELine(currentMesh->Triangles[currentMesh->TriangleSelected][0], currentMesh->Triangles[currentMesh->TriangleSelected][1], glm::vec3(1.0f, 1.0f, 0.0f)));
					LINE_RENDERER.AddLineToBuffer(FELine(currentMesh->Triangles[currentMesh->TriangleSelected][0], currentMesh->Triangles[currentMesh->TriangleSelected][2], glm::vec3(1.0f, 1.0f, 0.0f)));
					LINE_RENDERER.AddLineToBuffer(FELine(currentMesh->Triangles[currentMesh->TriangleSelected][1], currentMesh->Triangles[currentMesh->TriangleSelected][2], glm::vec3(1.0f, 1.0f, 0.0f)));

					if (!currentMesh->TrianglesNormals.empty())
					{
						glm::vec3 Point = currentMesh->Triangles[currentMesh->TriangleSelected][0];
						glm::vec3 Normal = currentMesh->TrianglesNormals[currentMesh->TriangleSelected][0];
						LINE_RENDERER.AddLineToBuffer(FELine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));

						Point = currentMesh->Triangles[currentMesh->TriangleSelected][1];
						Normal = currentMesh->TrianglesNormals[currentMesh->TriangleSelected][1];
						LINE_RENDERER.AddLineToBuffer(FELine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));

						Point = currentMesh->Triangles[currentMesh->TriangleSelected][2];
						Normal = currentMesh->TrianglesNormals[currentMesh->TriangleSelected][2];
						LINE_RENDERER.AddLineToBuffer(FELine(Point, Point + Normal, glm::vec3(0.0f, 0.0f, 1.0f)));
					}

					if (!currentMesh->originalTrianglesToSegments.empty() && !currentMesh->segmentsNormals.empty())
					{
						glm::vec3 Centroid = (currentMesh->Triangles[currentMesh->TriangleSelected][0] +
							currentMesh->Triangles[currentMesh->TriangleSelected][1] +
							currentMesh->Triangles[currentMesh->TriangleSelected][2]) / 3.0f;

						glm::vec3 Normal = currentMesh->segmentsNormals[currentMesh->originalTrianglesToSegments[currentMesh->TriangleSelected]];
						LINE_RENDERER.AddLineToBuffer(FELine(Centroid, Centroid + Normal, glm::vec3(1.0f, 0.0f, 0.0f)));
					}

					LINE_RENDERER.SyncWithGPU();
				}
			}
			else
			{
				RUGOSITY_MANAGER.currentSDF->mouseClick(mouseX, mouseY);

				LINE_RENDERER.clearAll();
				addLinesOFSDF(RUGOSITY_MANAGER.currentSDF);
				LINE_RENDERER.SyncWithGPU();
			}
			
		}
	}
}

void renderFEMesh(FEMesh* mesh)
{
	meshShader->getParameter("colorMode")->updateData(mesh->colorMode);
	if (!mesh->showRugosity)
		meshShader->getParameter("colorMode")->updateData(0);

	meshShader->getParameter("minRugorsity")->updateData(float(mesh->minRugorsity));
	meshShader->getParameter("maxRugorsity")->updateData(float(mesh->maxRugorsity));

	FE_GL_ERROR(glBindVertexArray(mesh->getVaoID()));
	if ((mesh->vertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
	if ((mesh->vertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
	if ((mesh->vertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
	if ((mesh->vertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
	if ((mesh->vertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));

	if ((mesh->vertexAttributes & FE_SEGMENTS_COLORS) == FE_SEGMENTS_COLORS) FE_GL_ERROR(glEnableVertexAttribArray(7));
	// Rugosity
	if (!mesh->rugosityData.empty())
		FE_GL_ERROR(glEnableVertexAttribArray(8));

	if ((mesh->vertexAttributes & FE_INDEX) == FE_INDEX)
		FE_GL_ERROR(glDrawElements(GL_TRIANGLES, mesh->getVertexCount(), GL_UNSIGNED_INT, 0));
	if ((mesh->vertexAttributes & FE_INDEX) != FE_INDEX)
		FE_GL_ERROR(glDrawArrays(GL_TRIANGLES, 0, mesh->getVertexCount()));

	glBindVertexArray(0);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	/*Point_3 a = Point_3(1.0, 5.0, 1.0);
	Point_3 b = Point_3(-2.0, 15.0, 4.0);
	Point_3 c = Point_3(-2.0, 5.0, -2.0);

	double area = sqrt(CGAL::squared_area(a, b, c));

	Plane_3 testPlane = Plane_3(Point_3(0.0, 0.0, 0.0), Direction_3(0.0, 1.0, 0.0));

	Point_3 aProjection = testPlane.projection(a);
	Point_3 bProjection = testPlane.projection(b);
	Point_3 cProjection = testPlane.projection(c);

	double projectionArea = sqrt(CGAL::squared_area(aProjection, bProjection, cProjection));
	double rugosity = area / projectionArea;


	int y = 0;
	y++;*/

	//Surface_mesh surface_mesh;

	//std::ifstream objFile("C:/Users/kandr/Downloads/OBJ_Models/sphere.obj");
	//std::vector<Point_3> points;
	//std::vector<Polygon_3> faces;

	//bool result = CGAL::IO::read_OBJ(objFile, points, faces);


	////PMP::orient_polygon_soup(points, faces); // optional if your mesh is not correctly oriented
	//Surface_mesh sm;
	//try
	//{
	//	PMP::polygon_soup_to_polygon_mesh(points, faces, sm);
	//}
	//catch (const std::exception& e)
	//{
	//	int y = 0;
	//	y++;
	//}
	//
	//// In this example, the simplification stops when the number of undirected edges
	//// drops below 10% of the initial count
	//double stop_ratio = 0.5;
	//SMS::Count_ratio_stop_predicate<Surface_mesh> stop(stop_ratio);

	//int r = SMS::edge_collapse(sm, stop);

	

	//saveSurfaceMeshToOBJFile("C:/Users/kandr/Downloads/OBJ_Models/sphereR_.obj", sm);

	

	//APPLICATION.createWindow(1280, 720, "Rugosity Calculator");
	APPLICATION.InitWindow(1920, 1080, "Rugosity Calculator");
	APPLICATION.SetDropCallback(dropCallback);
	APPLICATION.SetKeyCallback(keyButtonCallback);
	APPLICATION.SetMouseMoveCallback(mouseMoveCallback);
	APPLICATION.SetMouseButtonCallback(mouseButtonCallback);

	THREAD_POOL.SetConcurrentThreadCount(8);

	glClearColor(153.0f / 255.0f, 217.0f / 255.0f, 234.0f / 255.0f, 1.0f);
	FE_GL_ERROR(glEnable(GL_DEPTH_TEST));

	meshShader = new FEShader("mainShader", sTestVS, sTestFS);
	meshShader->getParameter("lightDirection")->updateData(glm::vec3(0.0, 1.0, 0.2));

	static int FEWorldMatrix_hash = int(std::hash<std::string>{}("FEWorldMatrix"));
	static int FEViewMatrix_hash = int(std::hash<std::string>{}("FEViewMatrix"));
	static int FEProjectionMatrix_hash = int(std::hash<std::string>{}("FEProjectionMatrix"));

	FETransformComponent* position = new FETransformComponent();


	currentCamera = new FEFreeCamera("mainCamera");
	currentCamera->setIsInputActive(false);
	currentCamera->setAspectRatio(1280.0f / 720.0f);

	RUGOSITY_MANAGER.currentCamera = currentCamera;

	dimentionsList.push_back("4");
	dimentionsList.push_back("8");
	dimentionsList.push_back("16");
	dimentionsList.push_back("32");
	dimentionsList.push_back("64");
	dimentionsList.push_back("128");
	dimentionsList.push_back("256");
	dimentionsList.push_back("512");
	dimentionsList.push_back("1024");
	dimentionsList.push_back("2048");
	dimentionsList.push_back("4096");

	colorSchemesList.push_back("Default");
	colorSchemesList.push_back("Rainbow");
	colorSchemesList.push_back("Turbo colormap");

	while (APPLICATION.IsWindowOpened())
	{
		FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		APPLICATION.BeginFrame();

		renderTargetCenterForCamera();
		currentCamera->move(10);

		if (currentMesh != nullptr)
		{
			meshShader->start();

			auto iterator = meshShader->parameters.begin();
			while (iterator != meshShader->parameters.end())
			{
				if (iterator->second.nameHash == FEWorldMatrix_hash)
					iterator->second.updateData(position->getTransformMatrix());

				if (iterator->second.nameHash == FEViewMatrix_hash)
					iterator->second.updateData(currentCamera->getViewMatrix());

				if (iterator->second.nameHash == FEProjectionMatrix_hash)
					iterator->second.updateData(currentCamera->getProjectionMatrix());

				iterator++;
			}

			meshShader->loadDataToGPU();

			if (wireframeMode)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			renderFEMesh(currentMesh);

			/*FETransformComponent* newPosition = new FETransformComponent();
			newPosition->setPosition(glm::vec3(0.0, 0.0, 5.0));
			testShader->getParameter("FEWorldMatrix")->updateData(newPosition->getTransformMatrix());
			testShader->loadDataToGPU();

			renderFEMesh(compareToMesh);*/
			
			//APPLICATION.setWindowCaption("vertexCount: " + std::to_string(loadedMesh->getVertexCount()));

			meshShader->stop();
		}

		LINE_RENDERER.Render(currentCamera);
			
		// ********************* LIGHT DIRECTION *********************
		glm::vec3 position = *reinterpret_cast<glm::vec3*>(meshShader->getParameter("lightDirection")->data);
		ImGui::Text("Light direction : ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::DragFloat((std::string("##X pos : ")).c_str(), &position[0], 0.1f);
		
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::DragFloat((std::string("##Y pos : ")).c_str(), &position[1], 0.1f);
		
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		ImGui::DragFloat((std::string("##Z pos : ")).c_str(), &position[2], 0.1f);
	
		meshShader->getParameter("lightDirection")->updateData(position);

		showCameraTransform(currentCamera);

		if (currentMesh != nullptr)
		{
			ImGui::Checkbox("Wireframe", &wireframeMode);

			//if (currentMesh->minRugorsity != DBL_MAX)
			//	ImGui::Text(("minRugorsity: " + std::to_string(currentMesh->minRugorsity)).c_str());

			//if (currentMesh->maxRugorsity != -DBL_MAX)
			//	ImGui::Text(("maxRugorsity: " + std::to_string(currentMesh->maxRugorsity)).c_str());

			if (currentMesh->TriangleSelected != -1)
			{
				ImGui::Separator();
				ImGui::Text("Selected triangle information :");

				std::string Text = "Index : " + std::to_string(currentMesh->TriangleSelected);
				ImGui::Text(Text.c_str());

				Text = "First vertex : ";
				Text += "x: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][0].x);
				Text += " y: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][0].y);
				Text += " z: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][0].z);
				ImGui::Text(Text.c_str());

				Text = "Second vertex : ";
				Text += "x: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][1].x);
				Text += " y: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][1].y);
				Text += " z: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][1].z);
				ImGui::Text(Text.c_str());

				Text = "Third vertex : ";
				Text += "x: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][2].x);
				Text += " y: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][2].y);
				Text += " z: " + std::to_string(currentMesh->Triangles[currentMesh->TriangleSelected][2].z);
				ImGui::Text(Text.c_str());

				Text = "Segment ID : ";
				if (!currentMesh->originalTrianglesToSegments.empty())
				{
					Text += std::to_string(currentMesh->originalTrianglesToSegments[currentMesh->TriangleSelected]);
				}
				else
				{
					Text += "No information.";
				}
				ImGui::Text(Text.c_str());

				Text = "Segment normal : ";
				if (!currentMesh->originalTrianglesToSegments.empty() && !currentMesh->segmentsNormals.empty())
				{
					glm::vec3 normal = currentMesh->segmentsNormals[currentMesh->originalTrianglesToSegments[currentMesh->TriangleSelected]];

					Text += "x: " + std::to_string(normal.x);
					Text += " y: " + std::to_string(normal.y);
					Text += " z: " + std::to_string(normal.z);
				}
				else
				{
					Text += "No information.";
				}
				ImGui::Text(Text.c_str());
			}

			ImGui::Separator();
			if (RUGOSITY_MANAGER.currentSDF == nullptr)
			{
				if (ImGui::Button("Generate SDF"))
				{
					RUGOSITY_MANAGER.JitterCounter = 0;
					currentMesh->TrianglesRugosity.clear();
					currentMesh->rugosityData.clear();

					RUGOSITY_MANAGER.bLastJitter = true;
					RUGOSITY_MANAGER.RunCreationOfSDFAsync(currentMesh);

					//calculateSDF(currentMesh, SDFDimention);
					//MoveRugosityInfoToMesh(currentSDF, true);
				}

				ImGui::SameLine();
				ImGui::SetNextItemWidth(128);
				if (ImGui::BeginCombo("##ChooseSDFDimention", std::to_string(RUGOSITY_MANAGER.SDFDimention).c_str(), ImGuiWindowFlags_None))
				{
					for (size_t i = 0; i < dimentionsList.size(); i++)
					{
						bool is_selected = (std::to_string(RUGOSITY_MANAGER.SDFDimention) == dimentionsList[i]);
						if (ImGui::Selectable(dimentionsList[i].c_str(), is_selected))
						{
							RUGOSITY_MANAGER.SDFDimention = atoi(dimentionsList[i].c_str());
						}

						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				ImGui::Checkbox("Weighted normals", &RUGOSITY_MANAGER.bWeightedNormals);
				ImGui::Checkbox("Normalized normals", &RUGOSITY_MANAGER.bNormalizedNormals);

				if (ImGui::Button("Generate SDF with Jitter"))
				{
					TIME.BeginTimeStamp("TimeTookToJitter");

					RUGOSITY_MANAGER.calculateRugorsityWithJitterAsyn(currentMesh);

					TimeTookToJitter = TIME.EndTimeStamp("TimeTookToJitter");
				}

				ImGui::SameLine();
				ImGui::SetNextItemWidth(100);
				ImGui::DragInt("Smoothing factor", &RUGOSITY_MANAGER.SmoothingFactor);
				if (RUGOSITY_MANAGER.SmoothingFactor < 2)
					RUGOSITY_MANAGER.SmoothingFactor = 2;
			}
			else if (RUGOSITY_MANAGER.currentSDF != nullptr && RUGOSITY_MANAGER.currentSDF->bFullyLoaded)
			{
				if (ImGui::Button("Delete SDF"))
				{
					//currentSDF->mesh clear all rugosity info

					LINE_RENDERER.clearAll();
					LINE_RENDERER.SyncWithGPU();

					delete RUGOSITY_MANAGER.currentSDF;
					RUGOSITY_MANAGER.currentSDF = nullptr;
				}
			}

			if (RUGOSITY_MANAGER.newSDFSeen > 0 && RUGOSITY_MANAGER.newSDFSeen < RUGOSITY_MANAGER.SmoothingFactor)
			{
				std::string Text = "Progress: " + std::to_string(RUGOSITY_MANAGER.newSDFSeen) + " out of " + std::to_string(RUGOSITY_MANAGER.SmoothingFactor);
				ImGui::Text(Text.c_str());
			}

			if (RUGOSITY_MANAGER.currentSDF != nullptr && RUGOSITY_MANAGER.currentSDF->bFullyLoaded)
			{
				if (ImGui::Checkbox("Show Rugosity", &currentMesh->showRugosity))
				{
					if (currentMesh->showRugosity)
					{
						currentMesh->colorMode = 5;
					}
					else
					{
						currentMesh->colorMode = 0;
					}
				}

				ImGui::Text("Color scheme: ");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(128);
				if (ImGui::BeginCombo("##ChooseColorScheme", (colorSchemeIndexToString(currentMesh->colorMode)).c_str(), ImGuiWindowFlags_None))
				{
					for (size_t i = 0; i < colorSchemesList.size(); i++)
					{
						bool is_selected = ((colorSchemeIndexToString(currentMesh->colorMode)).c_str() == colorSchemesList[i]);
						if (ImGui::Selectable(colorSchemesList[i].c_str(), is_selected))
						{
							currentMesh->colorMode = colorSchemeIndexFromString(colorSchemesList[i]);
						}

						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				
				float maxRugorsity = currentMesh->maxRugorsity;
				ImGui::Text("Max rugorsity for color scaling: ");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(128);
				ImGui::DragFloat("##MaxRugorsity", &maxRugorsity, 0.01f);
				if (maxRugorsity < currentMesh->minRugorsity)
					maxRugorsity = currentMesh->minRugorsity + 0.1f;
				currentMesh->maxRugorsity = maxRugorsity;

				ImGui::Separator();

				float TotalTime = 0.0f;

				std::string debugTimers = "Building of SDF grid : " + std::to_string(RUGOSITY_MANAGER.currentSDF->TimeTookToGenerateInMS) + " ms";
				debugTimers += "\n";
				TotalTime += RUGOSITY_MANAGER.currentSDF->TimeTookToGenerateInMS;

				debugTimers += "Fill cells with triangle info : " + std::to_string(RUGOSITY_MANAGER.currentSDF->TimeTookFillCellsWithTriangleInfo) + " ms";
				debugTimers += "\n";
				TotalTime += RUGOSITY_MANAGER.currentSDF->TimeTookFillCellsWithTriangleInfo;

				debugTimers += "Calculate rugosity : " + std::to_string(RUGOSITY_MANAGER.currentSDF->TimeTookCalculateRugosity) + " ms";
				debugTimers += "\n";
				TotalTime += RUGOSITY_MANAGER.currentSDF->TimeTookCalculateRugosity;

				debugTimers += "Assign colors to mesh : " + std::to_string(RUGOSITY_MANAGER.currentSDF->TimeTookFillMeshWithRugosityData) + " ms";
				debugTimers += "\n";
				TotalTime += RUGOSITY_MANAGER.currentSDF->TimeTookFillMeshWithRugosityData;

				debugTimers += "Total time : " + std::to_string(TotalTime) + " ms";
				debugTimers += "\n";

				debugTimers += "TimeTookToJitter : " + std::to_string(TimeTookToJitter) + " ms";
				debugTimers += "\n";

				ImGui::Text((debugTimers).c_str());

				ImGui::Text(("debugTotalTrianglesInCells: " + std::to_string(RUGOSITY_MANAGER.currentSDF->debugTotalTrianglesInCells)).c_str());

				ImGui::Separator();
				ImGui::Text("Visualization of SDF:");

				if (ImGui::RadioButton("Do not draw", &SDFRenderingMode, 0))
				{
					SDFRenderingMode = 0;

					LINE_RENDERER.clearAll();
					LINE_RENDERER.SyncWithGPU();
				}

				if (ImGui::RadioButton("Show cells with triangles", &SDFRenderingMode, 1))
				{
					SDFRenderingMode = 1;

					LINE_RENDERER.clearAll();
					addLinesOFSDF(RUGOSITY_MANAGER.currentSDF);
					LINE_RENDERER.SyncWithGPU();
				}

				if (ImGui::RadioButton("Show all cells", &SDFRenderingMode, 2))
				{
					SDFRenderingMode = 2;

					LINE_RENDERER.clearAll();
					addLinesOFSDF(RUGOSITY_MANAGER.currentSDF);
					LINE_RENDERER.SyncWithGPU();
				}

				ImGui::Separator();
			}
			else if (RUGOSITY_MANAGER.currentSDF != nullptr && !RUGOSITY_MANAGER.currentSDF->bFullyLoaded)
			{
				ImGui::Text("Creating SDF...");
			}

			ImGui::Separator();
			if (RUGOSITY_MANAGER.currentSDF != nullptr && RUGOSITY_MANAGER.currentSDF->selectedCell != glm::vec3(0.0))
			{
				SDFNode currentlySelectedCell = RUGOSITY_MANAGER.currentSDF->data[RUGOSITY_MANAGER.currentSDF->selectedCell.x][RUGOSITY_MANAGER.currentSDF->selectedCell.y][RUGOSITY_MANAGER.currentSDF->selectedCell.z];

				//ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(15, Ystep));
				ImGui::Text("Selected cell info: ");

				std::string indexes = "i: " + std::to_string(RUGOSITY_MANAGER.currentSDF->selectedCell.x);
				indexes += " j: " + std::to_string(RUGOSITY_MANAGER.currentSDF->selectedCell.y);
				indexes += " k: " + std::to_string(RUGOSITY_MANAGER.currentSDF->selectedCell.z);
				//ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(15, Ystep));
				ImGui::Text((indexes).c_str());

				//ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(15, Ystep));
				ImGui::Text(("value: " + std::to_string(currentlySelectedCell.value)).c_str());

				//ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(15, Ystep));
				ImGui::Text(("distanceToTrianglePlane: " + std::to_string(currentlySelectedCell.distanceToTrianglePlane)).c_str());

				ImGui::Separator();

				//ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(15, Ystep));
				ImGui::Text("Rugosity info: ");

				std::string text = "averageCellNormal x: " + std::to_string(currentlySelectedCell.averageCellNormal.x);
				text += " y: " + std::to_string(currentlySelectedCell.averageCellNormal.y);
				text += " z: " + std::to_string(currentlySelectedCell.averageCellNormal.z);
				//ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(15, Ystep));
				ImGui::Text((text).c_str());


				text = "CellTrianglesCentroid x: " + std::to_string(currentlySelectedCell.CellTrianglesCentroid.x);
				text += " y: " + std::to_string(currentlySelectedCell.CellTrianglesCentroid.y);
				text += " z: " + std::to_string(currentlySelectedCell.CellTrianglesCentroid.z);
				//ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(15, Ystep));
				ImGui::Text((text).c_str());

				if (currentlySelectedCell.approximateProjectionPlane != nullptr)
				{
					//glm::vec3 NormalStart = currentlySelectedCell.approximateProjectionPlane->PointOnPlane + choosenEntity->transform.getPosition();
					//glm::vec3 NormalEnd = NormalStart + currentlySelectedCell.approximateProjectionPlane->Normal;
					//RENDERER.drawLine(NormalStart, NormalEnd, glm::vec3(0.1f, 0.1f, 0.9f), 0.25f);
				}

				ImGui::Text(("Rugosity : " + std::to_string(currentlySelectedCell.rugosity)).c_str());

				static std::string debugText;

				if (ImGui::Button("Show debug info"))
				{
					RUGOSITY_MANAGER.currentSDF->calculateCellRugosity(&RUGOSITY_MANAGER.currentSDF->
						data[RUGOSITY_MANAGER.currentSDF->selectedCell.x][RUGOSITY_MANAGER.currentSDF->selectedCell.y][RUGOSITY_MANAGER.currentSDF->selectedCell.z], &debugText);
				}

				ImGui::Text(debugText.c_str());
			}
		}
		
		APPLICATION.EndFrame();
	}

	return 0;
}