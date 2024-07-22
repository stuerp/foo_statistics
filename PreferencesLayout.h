
/** $VER: PreferencesLayout.h (2024.07.22) **/

#pragma once

#define W_A00   332 // Dialog width as set by foobar2000, in dialog units
#define H_A00   288 // Dialog height as set by foobar2000, in dialog units

#define H_LBL     8 // Label

#define W_BTN    50 // Button
#define H_BTN    14 // Button

#define H_EBX    14 // Edit box
#define H_CBX    14 // Combo box

#define W_CHB    10 // Check box
#define H_CHB    10 // Check box

#define DX        7
#define DY        7

#define IX        4 // Spacing between two related controls
#define IY        3

#pragma region Pin To

// Label
#define X_A11   0
#define Y_A11   0
#define W_A11   85
#define H_A11   H_LBL

// EditBox
#define X_A12   X_A11 + W_A11 + IX
#define Y_A12   Y_A11
#define W_A12   W_A00 - W_A11 - DX
#define H_A12   H_EBX

#pragma endregion

#pragma region Threshold Format

// Label
#define X_A13   0
#define Y_A13   Y_A12 + H_A12 + IY
#define W_A13   85
#define H_A13   H_LBL

// EditBox
#define X_A14   X_A13 + W_A13 + IX
#define Y_A14   Y_A13
#define W_A14   W_A00 - W_A13 - DX
#define H_A14   H_EBX

#pragma endregion

#pragma region Retention Period

// Label
#define X_A20   0
#define Y_A20   Y_A14 + H_A14 + IY
#define W_A20   85
#define H_A20   H_LBL

// EditBox : Value
#define X_A21   X_A20 + W_A20 + IX
#define Y_A21   Y_A20
#define W_A21   30
#define H_A21   H_EBX

// ComboBox: Unit
#define X_A22   X_A21 + W_A21 + IX
#define Y_A22   Y_A20
#define W_A22   44
#define H_A22   H_CBX

#pragma endregion

// Checkbox: Write to tags
#define X_A50   0
#define Y_A50   Y_A21 + H_A21 + IY
#define W_A50   160
#define H_A50   H_LBL
