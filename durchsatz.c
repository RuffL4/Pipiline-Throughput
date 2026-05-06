/**
 * @file durchsatz.c
 * @brief Interactive visualization tool for pipeline throughput
 * 
 * This program calculates and plots the theoretical throughput of a 
 * pipeline architecture based on various parameters. It utilizes Raylib 
 * for hardware-accelerated rendering and Raygui for a dynamic 
 * immediate-mode user interface (IMGUI).
 * 
 * Memory Management: Dynamic memory allocation for the spline's vertex data.
 */

#include <stdlib.h>
#include <stdio.h>
#include "raylib.h"
#include <stdbool.h>
#include <math.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// WINDOW & LAYOUT CONFIGURATION
#define INIT_WIDTH 900
#define INIT_HEIGHT 600
#define PAD_LEFT   80
#define PAD_RIGHT  220
#define PAD_BOTTOM 60
#define PAD_TOP    40

// GRAPH METRICS
#define MAX_S 35.0
#define MAX_G 1.0
#define MAX_G_STAGES (int)(MAX_G * 10)
#define MAX_POINTS 1920 

// STYLING & COLORS
#define AXISCOLOR BLACK
#define FUNCTIONCOLOR RED
#define FONTSIZE 15
#define GRAPHTHICKNESS 2.5f
#define HOWERRADIUS 5
#define HOWERCOLOR BLUE
#define THROUGHPUTRADIUS 5
#define THROUGHPUTCOLOR PURPLE

// MATHEMATICAL STARTING VALUES 
#define START_T 5.5f
#define START_C 1.0f
#define START_k 1.0f
#define START_b 0.016f

/**
 * @struct ScreenData
 * @brief Manages all data for scaling and positioning within the window.
 */
typedef struct {
	Font mainFont;
	double scaleX, scaleY; // Conversion factor from mathematical units to pixels
	int currentWidth, currentHeight;
	int startX, endX, startY, endY; // Boundaries of the drawing area (padding subtracted)
       	int graphWidth, graphHeight;	
} ScreenData;

/**
 * @struct GraphData
 * @brief Stores calculated points, metrics, and UI state flags.
 */
typedef struct {
	Vector2 *points; // Dynamic array for spline points
	double optimalS;
	double maxG;
	float T; // Base Time
	float C; // Overhead
	float k; // Penalty stages
	float b; // Branch penalty factor
	int numPoints; // Current number of pixels to draw
	bool inspect; // Flag: Hover mode active
	bool round; // Flag: Round S values (snapping)
	bool showMaxG; // Flag: Show theoretical maximum
	bool showUI; // Flag: Show parameter menu
} GraphData;

// RENDER FUNCTIONS (GRAPHICS)

// Draws the actual function graph as a spline
void DrawFunction(GraphData *data){
	DrawSplineLinear(data->points, data->numPoints, GRAPHTHICKNESS, FUNCTIONCOLOR);
}

// Draws the point of maximum throughput including visual highlighting and an info box
void DrawMaxThroughput(ScreenData *scData, GraphData *grData){
	int py = (int)(grData->maxG * scData->scaleY);
	int yPos = scData->startY - py;
	int px = (int)(grData->optimalS *scData->scaleX);
	int xPos = scData->startX + px;
	DrawLine(xPos, scData->startY, xPos, yPos, THROUGHPUTCOLOR);
	DrawCircle(xPos, yPos, THROUGHPUTRADIUS, THROUGHPUTCOLOR);
	int textX = scData->endX  + 30;
	int textY = scData->endY;
	DrawRectangle(textX - 10, textY - 10, 190, 60, Fade(LIGHTGRAY, 0.8f));
	DrawTextEx(scData->mainFont, "THEORETISCHES MAXIMUM", (Vector2){textX, textY}, 10, 1.0f, DARKGRAY);
	DrawTextEx(scData->mainFont, TextFormat("S_opt: %.2f", grData->optimalS), (Vector2){textX, textY + 15}, FONTSIZE, 1.0f, THROUGHPUTCOLOR);
	DrawTextEx(scData->mainFont, TextFormat("G_max: %.8f", grData->maxG), (Vector2){textX, textY + 35}, FONTSIZE, 1.0f, THROUGHPUTCOLOR);
}

// Draws the coordinate system (axes, grid marks, and labels)
void DrawCOS(ScreenData *data){
	DrawLine(data->startX, data->startY, data->endX + 20, data->startY, AXISCOLOR);
	DrawLine(data->startX, data->startY, data->startX, data->endY - 10, AXISCOLOR);

	// X-axis labels (S stages)
	int S, x, yStart, yEnd;
	for (S = 1; S <= (int)MAX_S; S++){
		x = data->startX + (int)(S * data->scaleX);
		yStart = data->startY - 5;
		yEnd = data->startY + 5;
		DrawLine(x, yStart, x, yEnd, AXISCOLOR);
		if (S % 5 == 0) {
			DrawTextEx(data->mainFont, TextFormat("%d", S), (Vector2){x - 8, yEnd + 10}, FONTSIZE, 1.0f, AXISCOLOR);
		}
	}

	// Y-axis labels (G throughput)
	int i;
	for (i = 1; i <= MAX_G_STAGES; i++) {
	    double val = i * 0.1;
	    int y = data->startY - (int)(val * data->scaleY);
	    DrawLine(data->startX - 5, y, data->startX + 5, y, AXISCOLOR);
	    DrawTextEx(data->mainFont, TextFormat("%.1f", val), (Vector2){data->startX - 35, y - 5}, 10, 1.0f, AXISCOLOR);
	}
	// Axis titles
	int textWidthX = MeasureText("Pipelinestufen S", FONTSIZE);
	DrawTextEx(data->mainFont, "Pipelinestufen S", (Vector2){data->startX + (data->graphWidth / 2) - (textWidthX / 2), data->startY + 30}, FONTSIZE, 1.0f, AXISCOLOR);
	int textWidthY = MeasureText("Durchsatz G", FONTSIZE);
	DrawTextEx(data->mainFont, "Durchsatz G", (Vector2){data->startX - (textWidthY / 2), data->endY - 30}, FONTSIZE, 1.0f, AXISCOLOR);
}

// Draws the parameter tuning menu (Immediate-Mode GUI)
void DrawUI(ScreenData *scData, GraphData *grData){
	int startX = scData->endX + 30;
	int startY = scData->endY + 180;
	DrawRectangle(startX - 10, startY - 30, 190, 190, Fade(LIGHTGRAY, 0.8f));
	DrawTextEx(scData->mainFont, "PARAMETER INPUT", (Vector2){startX, startY - 20}, 10, 1.0f, DARKGRAY);
	int sliderWidth = 120;
    	int sliderHeight = 20;
    	GuiSliderBar((Rectangle){ startX + 5, startY, sliderWidth, sliderHeight },
                 "T",
                 TextFormat("%.1f", grData->T),
                 &grData->T, 1.0f, 10.0f);

    	GuiSliderBar((Rectangle){ startX + 5, startY + 40, sliderWidth, sliderHeight },
                 "C",
                 TextFormat("%.2f", grData->C),
                 &grData->C, 0.1f, 3.0f);

    	GuiSliderBar((Rectangle){ startX + 5, startY + 80, sliderWidth, sliderHeight },
                 "k",
                 TextFormat("%.1f", grData->k),
                 &grData->k, 1.0f, 5.0f);

    	GuiSliderBar((Rectangle){ startX + 5, startY + 120, sliderWidth, sliderHeight },
                 "b",
                 TextFormat("%.3f", grData->b),
                 &grData->b, 0.001f, 0.1f);
}


// MATHEMATICS & CALCULATION FUNCTIONS

// Interpolates the pipeline equation across the X-axis and transforms it into screen-space pixels
void CalculateThroughput(ScreenData *scData, GraphData *grData){
	double G;
	int px;
	for (px = 0; px < grData->numPoints; px++){
		double S = px / scData->scaleX;
		if (S <= 0.001) S = 0.001;
		G = (1 / grData->T) * (1 / (1 + (S - grData->k) * grData->b)) * (S / (1 + (S - 1) * (grData->C/grData->T)));
		int py = (int)(G * scData->scaleY);
		grData->points[px].x = scData->startX + px;
		grData->points[px].y = scData->startY - py; 
	}
}

// Analytically determines the exact maximum of the throughput curve
void CalculateMaxThroughput(ScreenData *scData, GraphData *grData){
	double S = sqrt((((1 - grData->b * grData->k) * (1 - grData->C / grData->T)) / (grData->b * grData->C / grData->T)));
	double G = (1 / grData->T) * (1 / (1 + (S - grData->k) * grData->b)) * (S / (1 + (S - 1) * (grData->C/grData->T)));
	grData->optimalS = S;
	grData->maxG = G;
}

// Updates layout properties based on the current window size
void CalculateScreenData(ScreenData *data){
	data->currentWidth = GetScreenWidth();
	data->currentHeight = GetScreenHeight();
	data->startX = PAD_LEFT;
	data->endX = data->currentWidth - PAD_RIGHT;
	data->startY = data->currentHeight - PAD_BOTTOM;
	data->endY = PAD_TOP;
	data->graphWidth = data->endX - data->startX;
	data->graphHeight = data->startY - data->endY;
	data->scaleX = (double)data->graphWidth / MAX_S;
	data->scaleY = (double)data->graphHeight / MAX_G;
}

// Wrapper function for the per-frame calculation cycle
void MakeCalculations(ScreenData *scData, GraphData *grData){
	CalculateScreenData(scData);
	grData->numPoints = scData->graphWidth + 1;
	if (grData->numPoints > MAX_POINTS) grData->numPoints = MAX_POINTS;
	CalculateThroughput(scData, grData);
	CalculateMaxThroughput(scData, grData);
}

// "Hover / Inspect" Mode: Tracks the mouse and calculates live values on the curve
void MakeHower(ScreenData *scData, GraphData *grData){
	int mouseX = GetMouseX();
	if (mouseX >= scData->startX && mouseX <= scData->endX){
		double S = (mouseX - scData->startX) / scData->scaleX;
		if (grData->round){
			S = round(S);
			mouseX = scData->startX + S * scData->scaleX;
		};
		double G = (1 / grData->T) * (1 / (1 + (S - grData->k) * grData->b)) * (S / (1 + (S - 1) * (grData->C/grData->T)));
		int py = (int)(G * scData->scaleY);
		int yPos = scData->startY - py;
		DrawCircle(mouseX, yPos, HOWERRADIUS, HOWERCOLOR);
		int textX = scData->endX + 30;
		int textY = scData->endY + 80; 
		DrawRectangle(textX - 10, textY - 10, 190, 60, Fade(LIGHTGRAY, 0.8f));
		DrawTextEx(scData->mainFont, "AKTUELLER WERT (INSPECT)", (Vector2){textX, textY}, 10, 1.0f, DARKGRAY);
		DrawTextEx(scData->mainFont, TextFormat("S: %.2f", S), (Vector2){textX, textY + 15}, FONTSIZE, 1.0f, HOWERCOLOR);
		DrawTextEx(scData->mainFont, TextFormat("G: %.8f", G), (Vector2){textX, textY + 35}, FONTSIZE, 1.0f, HOWERCOLOR);
	}
}


int main(void){
	// Window setup
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(INIT_WIDTH, INIT_HEIGHT, "Durchsatz");
	SetTargetFPS(60);

	// Font setup
	ScreenData screenData;
	screenData.mainFont = LoadFontEx("arial.ttf", 32, 0, 250);
	SetTextureFilter(screenData.mainFont.texture, TEXTURE_FILTER_BILINEAR);
	
	// Data structure & memory setup
	GraphData graphData;
	graphData.points = (Vector2*)malloc(sizeof(Vector2) * MAX_POINTS);
	if(graphData.points == NULL){
		printf("Kein Speicher mehr frei\n");
		return -1;
	}

	// UI starting values
	graphData.T = START_T;
	graphData.C = START_C;
	graphData.k = START_k;
	graphData.b = START_b;

	// Initial calculation before the first frame
	MakeCalculations(&screenData, &graphData);

	while(!WindowShouldClose())
	{
		// Logic & math update
		MakeCalculations(&screenData, &graphData);

		// Input handling
		if (IsKeyPressed(KEY_R)) graphData.round = !graphData.round;
		if (IsKeyPressed(KEY_I)) graphData.inspect = !graphData.inspect;
		if (IsKeyPressed(KEY_M)) graphData.showMaxG = !graphData.showMaxG;
		if (IsKeyPressed(KEY_D)) graphData.showUI = !graphData.showUI;

		// Rendering
		BeginDrawing();
			ClearBackground(RAYWHITE);
			DrawCOS(&screenData);
			DrawFunction(&graphData);
			if (graphData.showMaxG) DrawMaxThroughput(&screenData, &graphData);
			if (graphData.inspect) MakeHower(&screenData, &graphData);
			if (graphData.showUI) DrawUI(&screenData, &graphData);
		EndDrawing();
	}

	// Memory cleanup & exit
	CloseWindow();
	free(graphData.points);
	UnloadFont(screenData.mainFont);
	return 0;
}
