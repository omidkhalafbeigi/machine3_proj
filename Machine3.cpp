
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <filesystem>
#include <Windows.h>

using namespace std;

string preprocess_data(string File_Path) 
{
    fstream reader;
    string buffer;
    string line_buffer;
    string dataset;
    int comma_iteraiton = 0;

    reader.open(File_Path);

    while (getline(reader, buffer))
    {
        for (char c : buffer)
        {
            if (c == ',')
            {
                comma_iteraiton += 1;
            }
            else
            {
                comma_iteraiton = 0;
            }

            if (comma_iteraiton > 1)
            {
                continue;
            }
            else
            {
                line_buffer += c;
            }
        }

        dataset += line_buffer;
        line_buffer = "";
        dataset += "\n";

        comma_iteraiton = 0;
    }
    reader.close();

    return dataset;
}

void save_dataset(string File_Path, string Text)
{
    ofstream writer(File_Path);
    writer << Text;
    writer.close();
}

float get_value(string file_path, int x, int y)
{
    string buffer;
    fstream reader;
    reader.open(file_path);

    for (int row = 1; row < x; row++)
    {
        getline(reader, buffer);
    }

    getline(reader, buffer, ',');
    
    for (int column = 1; column <= y; column++)
    {
        getline(reader, buffer, ',');
    }

    return stof(buffer);
}

float ILM_calculator(double MotorSize, double LL_Left, double PowerFactor)
{
    return ((MotorSize * 1000) / (1.732 * LL_Left * PowerFactor));
}

float IrsM_calculator_empty(double ILM, double LL_Left, double LocakedRotorCurrentMultiplier_Max)
{
    return ((ILM * LL_Left * LocakedRotorCurrentMultiplier_Max * 1.732) / 1000);
}

float IrsM_calculator_not_empty(double ILM, double LL_Left, double IrushCurrentStarting)
{
    return ((ILM * LL_Left, IrushCurrentStarting * 1.732) / 1000);
}


int main()
{
    string file_name;
    char working_directory[MAX_PATH + 1];
    GetCurrentDirectoryA(sizeof(working_directory), working_directory);
    string current_path = working_directory;

    cout << "Copy file on this location: " << current_path << endl;

    cout << "Enter file name: ";
    cin >> file_name;

    string data = preprocess_data(current_path + "\\" + file_name);
    save_dataset(current_path + "\\" + "New_" + file_name, data);

    string file_path;
    file_path = current_path + "\\" + "New_" + file_name;

    double TransformerSize_Left_Details = get_value(file_path, 2, 2);
    double TransformerSize_Right_Details = get_value(file_path, 2, 5);
    double TransformerSecondaryVoltage_Details_Left = get_value(file_path, 3, 2);
    double TransformerSecondaryVoltage_Details_Right = get_value(file_path, 3, 5);
    double Impedance_Left = get_value(file_path, 4, 2);
    double Impedance_Right = get_value(file_path, 4, 4);
    double AllowableVoltageDropinSystem_Left = get_value(file_path, 5, 2);
    double AllowableVoltageDropinSystem_Right = get_value(file_path, 5, 4);
    double MotorSize = get_value(file_path, 7, 2);
    double LL_Left = get_value(file_path, 8, 2);
    double LL_Right = get_value(file_path, 9, 4);
    double TotalKWThreePhaseMotors = get_value(file_path, 7, 5);
    double TotalKWSinglePhaseMotors = get_value(file_path, 8, 5);
    double PowerFactor_Left = get_value(file_path, 9, 2);
    double LocakedRotorCurrentMultiplier_Max = get_value(file_path, 71, 1);
    double LocakedRotorCurrentMultiplier_Min = get_value(file_path, 70, 1);
    double L1 = 0;
    double L2 = 0;
    double ILX = 0;
    double Isc_Left_Calc = 0;
    double Q1 = 0;
    double Vd = 0;
    double TransformerSecondaryVoltage_Left_Calculations = 0;
    int HowManyTimes = 0;

    string ILM_str;
    string IrsM_str;

    double ILM = 0;
    double IrsM = 0;

    // Motor Detail & Transformer Detail (Left side)

    cout << "InRush Current at Starting : (Enter 'empty' If you know otherwise leave it blank): " << endl;
    cin >> IrsM_str;
    cout << "Enter Motor Full Load Current(Enter 'empty' If you know otherwise leave it blank): " << endl;
    cin >> ILM_str;
    cout << "How many times motor starts per hours: " << endl;
    cin >> HowManyTimes;

    if (ILM_str == "empty") { ILM = ILM_calculator(MotorSize, LL_Left, PowerFactor_Left); }
    else { ILM = stof(ILM_str); }

    if (IrsM_str == "empty") { IrsM = IrsM_calculator_empty(ILM, LL_Left, LocakedRotorCurrentMultiplier_Max); }
    else { IrsM = IrsM_calculator_not_empty(ILM, LL_Left, IrsM); }

    L1 = ILM * LocakedRotorCurrentMultiplier_Min;
    L2 = ILM * LocakedRotorCurrentMultiplier_Max;

    ILX = (TransformerSize_Left_Details * 1000) / (TransformerSecondaryVoltage_Details_Left * 1.732);
    Isc_Left_Calc = (ILX / Impedance_Left);
    Q1 = ((TransformerSecondaryVoltage_Details_Left * Isc_Left_Calc * 1.732) / 1000) * 100;
    Vd = (IrsM / Q1) * 100;
    TransformerSecondaryVoltage_Left_Calculations = (TransformerSecondaryVoltage_Details_Left * (100 - Vd)) / 100;

    //-----------------------------


    // Results section

    string MotorFullLoadCurrent_Left_Less_65;
    if ((ILX * 65) >= ILM) { MotorFullLoadCurrent_Left_Less_65 = "Yes"; }
    else { MotorFullLoadCurrent_Left_Less_65 = "No"; }

    string VoltageDrop_Left;
    if (Vd < AllowableVoltageDropinSystem_Left) { VoltageDrop_Left = "Low"; }
    else { VoltageDrop_Left = "High"; }

    double TransformerSize = 0;
    if (HowManyTimes == 1) { TransformerSize = TransformerSize_Left_Details; }
    else { TransformerSize = ((TransformerSize_Left_Details * 0.2) + TransformerSize_Left_Details); }
     
    //-----------------------------

    // Motor Detail & Transformer Detail (Right side) & Calculations

    double LN = (LL_Right / 1.732);
    double PowerFactor_Right = get_value(file_path, 11, 3);
    double LocakedRotorCurrent_Max = get_value(file_path, 12, 3);
    double TotalThreePhaseMotorsFullLoadCurrent = ((TotalKWThreePhaseMotors * 1000) / (LL_Right * PowerFactor_Right * 1.732));
    double TotalSinglePhaseMotorsFullLoadCurrent = ((TotalKWSinglePhaseMotors * 1000) / (PowerFactor_Right * LN));
    double TotalMotorsFullLoadCurrent = TotalThreePhaseMotorsFullLoadCurrent + TotalSinglePhaseMotorsFullLoadCurrent;
    double MotorInRushKVAStarting = (TotalMotorsFullLoadCurrent * LL_Right * (LocakedRotorCurrent_Max / 100) * 1.732) / 1000;
    double IL = (TransformerSize_Right_Details * 1000) / (TransformerSecondaryVoltage_Details_Right * 1.732);
    double Isc_Right_Calc = (IL / (Impedance_Right / 100));
    double MaxKVATransformerIsc = ((TransformerSecondaryVoltage_Details_Right * Isc_Right_Calc * 1.732) / 1000);
    double VoltageDropTransformerSecondary_Right_Calc = (MotorInRushKVAStarting / MaxKVATransformerIsc) * 100;
    double TransformerSecondryVoltage_Right_Calc = (TransformerSecondaryVoltage_Details_Right * (100 - VoltageDropTransformerSecondary_Right_Calc)) / 100;


    // Results (Right side)

    string MotorFullLoadCurrent_Right_Less_65;
    if ((IL * 65) >= TotalMotorsFullLoadCurrent) { MotorFullLoadCurrent_Right_Less_65 = "Yes"; }
    else { MotorFullLoadCurrent_Right_Less_65 = "No"; }

    string VoltageDrop_Right;
    if (VoltageDropTransformerSecondary_Right_Calc < AllowableVoltageDropinSystem_Right) { VoltageDrop_Right = "Low"; }
    else { VoltageDrop_Right = "High"; }

    double TransformerSize_Right_Result = TransformerSize_Right_Details;

    //-----------------------------


    cout << "TransformerSize_Left_Details: " << TransformerSize_Left_Details << endl;
    cout << "TransformerSize_Right_Details: " << TransformerSize_Right_Details << endl;
    cout << "TransformerSecondaryVoltage_Details_Left: " << TransformerSecondaryVoltage_Details_Left << endl;
    cout << "TransformerSecondaryVoltage_Details_Right: " << TransformerSecondaryVoltage_Details_Right << endl;
    cout << "Impedance_Left: " << Impedance_Left << endl;
    cout << "Impedance_Right: " << Impedance_Right << endl;
    cout << "AllowableVoltageDropinSystem_Left: " << AllowableVoltageDropinSystem_Left << endl;
    cout << "AllowableVoltageDropinSystem_Right: " << AllowableVoltageDropinSystem_Right << endl;
    cout << "MotorSize: " << MotorSize << endl;
    cout << "LL_Left: " << LL_Left << endl;
    cout << "LL_Right: " << LL_Right << endl;
    cout << "TotalKWThreePhaseMotors: " << TotalKWThreePhaseMotors << endl;
    cout << "TotalKWSinglePhaseMotors: " << TotalKWSinglePhaseMotors << endl;
    cout << "PowerFactor_Left: " << PowerFactor_Left << endl;
    cout << "LocakedRotorCurrentMultiplier_Max: " << LocakedRotorCurrentMultiplier_Max << endl;
    cout << "LocakedRotorCurrentMultiplier_Min: " << LocakedRotorCurrentMultiplier_Min << endl;
    cout << "L1: " << L1 << endl;
    cout << "L2: " << L2 << endl;
    cout << "ILX: " << ILX << endl;
    cout << "Isc_Left_Calc: " << Isc_Left_Calc << endl;
    cout << "Q1: " << Q1 << endl;
    cout << "Vd: " << Vd << endl;
    cout << "TransformerSecondaryVoltage_Left_Calculations: " << TransformerSecondaryVoltage_Left_Calculations << endl;
    cout << "HowManyTimes: " << HowManyTimes << endl;
    cout << "ILM: " << ILM << endl;
    cout << "IrsM: " << IrsM << endl;
    cout << "Isc_Left_Calc: " << Isc_Left_Calc << endl;
    cout << "ILX: " << ILX << endl;
    cout << "TransformerSize" << TransformerSize << endl;
    cout << "MotorFullLoadCurrent_Left_Less_65: " << MotorFullLoadCurrent_Left_Less_65 << endl;
    cout << "VoltageDrop_Left: " << VoltageDrop_Left << endl;
    cout << "LN: " << LN << endl;
    cout << "PowerFactor_Right: " << ILX << endl;
    cout << "LocakedRotorCurrent_Max: " << LocakedRotorCurrent_Max << endl;
    cout << "TotalThreePhaseMotorsFullLoadCurrent: " << TotalThreePhaseMotorsFullLoadCurrent << endl;
    cout << "TotalSinglePhaseMotorsFullLoadCurrent: " << TotalSinglePhaseMotorsFullLoadCurrent << endl;
    cout << "TotalMotorsFullLoadCurrent: " << TotalMotorsFullLoadCurrent << endl;
    cout << "MotorInRushKVAStarting: " << MotorInRushKVAStarting << endl;
    cout << "IL: " << IL << endl;
    cout << "Isc_Right_Calc: " << Isc_Right_Calc << endl;
    cout << "MaxKVATransformerIsc: " << MaxKVATransformerIsc << endl;
    cout << "VoltageDropTransformerSecondary_Right_Calc: " << VoltageDropTransformerSecondary_Right_Calc << endl;
    cout << "TransformerSecondryVoltage_Right_Calc: " << TransformerSecondryVoltage_Right_Calc << endl;
    cout << "MotorFullLoadCurrent_Right_Less_65: " << MotorFullLoadCurrent_Right_Less_65 << endl;
    cout << "VoltageDrop_Right: " << VoltageDrop_Right << endl;
    cout << "TransformerSize_Right_Result: " << TransformerSize_Right_Details << endl;

   
    return 0;
}
