#include "AZLAutopatcher.h"

using namespace std;

// By Roiniti

#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include <algorithm>

struct parameters {
    //-a apk_path
    std::string apk_file;
    //-d (if present true, if not false)
    bool android_debug;
    //-r (if present true, if not false)
    bool reuse_extracted;
};

/// Prints the program usage
void print_help();
/// <summary>
/// Parse the args from the command line
/// </summary>
/// <returns>the parsed parameters</returns>
struct parameters parse_args(int argc, char* argv[]);
/// <summary>
/// Initialize the default args
/// </summary>
/// <returns>the raw parameters</returns>
struct parameters create_raw_parameters();
int main(int argc, char* argv[]);
/// <summary>
/// Extracts the apk to .\extracted\apk
/// </summary>
/// <param name="path">Apk path</param>
void extract_apk(std::string path);
/// <summary>
///  Repacks the apk from .\extracted\apk
/// </summary>
/// <param name="path">apk out file path</param>
void repack_apk(std::string path);
/// <summary>
/// 
/// </summary>
/// <param name="path"></param>
void clear_folder(std::string path);
/// <summary>
/// Apply the patch to the folder .\extracted\apk
/// </summary>
void patch_formidable_ml(bool android_debug);
/// <summary>
/// Simple function to copy a file
/// </summary>
/// <param name="sourcePath">Source path</param>
/// <param name="destinationPath">Destination path</param>
/// <returns></returns>
bool copyLibrary(const std::string& sourcePath, const std::string& destinationPath);
bool replaceDebuggableAttribute(const std::string& filePath);

int main(int argc, char* argv[]) {
    struct parameters args = parse_args(argc, argv);

    // Now you can use args.apk_file and args.android_debug in your program
    std::cout << "APK File: " << args.apk_file << std::endl;
    std::cout << "Android Debug: " << (args.android_debug ? "true" : "false") << std::endl;
    std::cout << "Android Reuse: " << (args.reuse_extracted ? "true" : "false") << std::endl;

    if (!args.reuse_extracted) {
        std::cout << "Extracting apk..." << std::endl;
        extract_apk(args.apk_file);
    }
    std::cout << "Applying patch..." << std::endl;
    patch_formidable_ml(args.android_debug);
    std::cout << "Repacking apk..." << std::endl;
    repack_apk(args.apk_file);
    return 0;
}

struct parameters create_raw_parameters() {
    struct parameters parameters;
    parameters.apk_file = "";
    parameters.android_debug = false;
    parameters.reuse_extracted = false;
    return parameters;
}

struct parameters parse_args(int argc, char* argv[]) {
    struct parameters args = create_raw_parameters();

    // Initialize defaults
    args.android_debug = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-a" && ++i < argc) {
            args.apk_file = argv[i];
        }
        else if (arg == "-d") {
            args.android_debug = true;
        }
        else if (arg == "-r") {
            args.reuse_extracted = true;
        }
        else if (arg == "-h") {
            print_help();
            exit(0);
        }
        else {
            // Handle unrecognized arguments or provide usage information
            std::cerr << "Error: Unrecognized argument '" << arg << "'" << std::endl;
            /// Print usage
            print_help();
            // Exit the program
            exit(1);
        }
    }
    if (args.apk_file=="") {
        print_help();
        exit(0);

    }

    return args;
}
void print_help() {
    std::cout << "Usage: ./azlautopatcher -a <APK_PATH> [-d]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -a <APK_PATH>    Specify the path to the APK file." << std::endl;
    std::cout << "  -d               Enable Android debug mode." << std::endl;
    std::cout << "  -r               Reuse the extracted apk." << std::endl;
}

// Function to find the index of a method in the lines
int findMethodIndex(const std::vector<std::string>& lines, const std::string& methodName) {
    auto it = std::find(lines.begin(), lines.end(), methodName);
    if (it != lines.end()) {
        return std::distance(lines.begin(), it);
    }
    return -1;
}

// Function to insert lines at a specific index in a vector of strings
std::vector<std::string> insertLines(const std::vector<std::string>& lines, const std::vector<std::string>& linesToInsert, int index) {
    std::vector<std::string> result;
    result.reserve(lines.size() + linesToInsert.size());

    result.insert(result.end(), lines.begin(), lines.begin() + index);
    result.insert(result.end(), linesToInsert.begin(), linesToInsert.end());
    result.insert(result.end(), lines.begin() + index, lines.end());

    return result;
}


void patch_formidable_ml(bool android_debug) {

    std::vector<std::string> lines;
    std::string line;
    std::string filePath;

    //path for EN server
    filePath = "extracted\\apk\\smali_classes2\\com\\unity3d\\player\\UnityPlayerActivity.smali";
    if (!std::filesystem::exists(filePath))
        //Path for CN server
        filePath = "extracted\\apk\\smali\\com\\unity3d\\player\\UnityPlayerActivity.smali";
    //TODO check for other servers paths


    if (!std::filesystem::exists(filePath)) {
        std::cerr << "Error: UnityPlayerActivity.smali not found." << std::endl;
        exit(-1);
    }

    std::ifstream inputFile(filePath);

    if (!inputFile.is_open()) {
        std::cerr << "Error: Unable to open input file for reading." << std::endl;
        exit(-1);
    }

    bool patchFound = false;
    // Read lines from the file
    while (std::getline(inputFile, line)) {
        if (line.find("FormidableML") != std::string::npos) {
            patchFound = true;
            break;
        }
        lines.push_back(line);

    }

    // Close the input file
    inputFile.close();

    if (!patchFound) {
        // Find the index of the method
        int indexToInsert = findMethodIndex(lines, ".method protected onCreate(Landroid/os/Bundle;)V");

        if (indexToInsert == -1) {
            std::cout << "Method onCreate not found in the Smali file." << std::endl;
            exit(-1);
        }
        // Lines to insert before and after
        std::vector<std::string> insertLinesBefore = {
            ".method private static native init(Landroid/content/Context;)V",
            ".end method",
            ""
        };

        std::vector<std::string> insertLinesAfter = {
            "",
            "	const-string v0, \"FormidableML\"",
            "	invoke-static {v0}, Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V",
            "	invoke-static {p0}, Lcom/unity3d/player/UnityPlayerActivity;->init(Landroid/content/Context;)V"
        };

        // Insert lines
        lines = insertLines(lines, insertLinesAfter, indexToInsert + 2);
        lines = insertLines(lines, insertLinesBefore, indexToInsert);


        // Write lines back to the file
        std::ofstream outputFile(filePath);
        if (!outputFile.is_open()) {
            std::cerr << "Error: Unable to open output file for writing." << std::endl;
            exit(-1);
        }

        for (const auto& modifiedLine : lines) {
            outputFile << modifiedLine << '\n';
        }
        outputFile.close();
        std::cout << "Smali has been patched successfully." << std::endl;
    }
    else {
        std::cout << "Smali patch found, skipping..." << std::endl;
    }
    if (android_debug) {
        replaceDebuggableAttribute("extracted\\apk\\AndroidManifest.xml");
    }

    bool loadedLib = false;

    // Copy libFormidableML.so for arm64-v8a
    if (std::filesystem::exists(".\\libs\\arm64-v8a\\libFormidableML.so") && std::filesystem::exists(".\\extracted\\apk\\lib\\arm64-v8a\\")) {
        loadedLib = true;
        copyLibrary(".\\libs\\arm64-v8a\\libFormidableML.so", ".\\extracted\\apk\\lib\\arm64-v8a\\libFormidableML.so");
        std::cout << "Installed libFormidableML for arm64-v8a" << std::endl;
    }

    // Copy libFormidableML.so for armeabi-v7a
    if (std::filesystem::exists(".\\libs\\armeabi-v7a\\libFormidableML.so") && std::filesystem::exists(".\\extracted\\apk\\lib\\armeabi-v7a\\")) {
        loadedLib = true;
        copyLibrary(".\\libs\\armeabi-v7a\\libFormidableML.so", ".\\extracted\\apk\\lib\\armeabi-v7a\\libFormidableML.so");
        std::cout << "Installed libFormidableML for armeabi-v7a" << std::endl;
    }

    // Copy libFormidableML.so for x86
    if (std::filesystem::exists(".\\libs\\x86\\libFormidableML.so") && std::filesystem::exists(".\\extracted\\apk\\lib\\x86\\")) {
        loadedLib = true;
        copyLibrary(".\\libs\\x86\\libFormidableML.so", ".\\extracted\\apk\\lib\\x86\\libFormidableML.so");
        std::cout << "Installed libFormidableML for x86" << std::endl;
    }

    if (!loadedLib) {
        std::cout << "WARNING! No FormidableML lib or destination found!" << std::endl;
    }
}
bool replaceDebuggableAttribute(const std::string& filePath) {
    std::ifstream inputFile(filePath);
    std::ofstream outputFile(filePath + ".tmp");

    if (inputFile.is_open() && outputFile.is_open()) {
        std::string line;

        //TODO: make it more efficient (less .find functions etc)
        // Read lines from the file
        while (std::getline(inputFile, line)) {
            if (line.find("android:debuggable=\"false\"") != std::string::npos)
            {
                size_t pos = line.find("android:debuggable=\"false\"");
                line.replace(pos, std::string("android:debuggable=\"false\"").length(), "android:debuggable=\"true\"");
                inputFile.close();
                outputFile.close();
                return true;
            }
            if (line.find("android:debuggable=\"true\"") != std::string::npos)
            {
                inputFile.close();
                outputFile.close();
                return true;
            }
            size_t pos = line.find("<application");
            if (pos != std::string::npos) {
                line.insert(pos + 12, " android:debuggable=\"true\"");
            }
            outputFile << line << '\n';
        }

        // Close the input and output files
        inputFile.close();
        outputFile.close();

        // Rename the temporary file to the original file
        std::filesystem::rename(filePath + ".tmp", filePath);

        return true;
    }
    else {
        std::cerr << "Error: Unable to open input or output file for reading/writing." << std::endl;
        return false;
    }
}

bool copyLibrary(const std::string& sourcePath, const std::string& destinationPath) {
    try {
        std::filesystem::copy_file(sourcePath, destinationPath, std::filesystem::copy_options::overwrite_existing);
        return true;
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}


//Apk procesing
void clear_folder(std::string path) {
    if (!std::filesystem::exists(path)) {
        return;
    }

    std::filesystem::path folderPath(path);
    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::filesystem::remove(entry.path());
        }
        else if (entry.is_directory()) {
            clear_folder(entry.path().string());
            std::filesystem::remove(entry.path());
        }
    }
}
void extract_apk(std::string path) {
    clear_folder("./extracted");
    std::string command = "java -jar .\\apktool.jar d " + path + " -o .\\extracted\\apk -f";

    // Using std::system to execute the command
    int exitCode = std::system(command.c_str());

    if (exitCode == 0) {
        std::cout << "Apktool process executed successfully." << std::endl;
    }
    else {
        std::cerr << "Error: Apktool process failed with exit code " << exitCode << std::endl;
    }
}
void repack_apk(std::string path) {
    //Define commands
    //Call apktool for repack
    std::string apktool_repack = "java -jar .\\apktool.jar b .\\extracted\\apk -o extracted\\recompiled.apk -f";
    std::string zip_alignment = "zipalign.exe -f 4 extracted\\recompiled.apk extracted\\aligned.apk";
    std::string apksigner = "java -jar .\\apksigner.jar sign -ks .\\my-release-key.keystore -ks-pass pass:password --v1-signing-enabled true --v2-signing-enabled true extracted\\aligned.apk";

    // Using std::system to execute the command
    int exitCode = std::system(apktool_repack.c_str());

    if (exitCode == 0) {
        std::cout << "Apktool process executed successfully." << std::endl;
    }
    else {
        std::cerr << "Error: Apktool process failed with exit code " << exitCode << std::endl;
        exit(-1);
    }

    exitCode = std::system(zip_alignment.c_str());

    if (exitCode == 0) {
        std::cout << "Zipalign process executed successfully." << std::endl;
    }
    else {
        std::cerr << "Error: Zipalign process failed with exit code " << exitCode << std::endl;
        exit(-1);
    }

    exitCode = std::system(apksigner.c_str());

    if (exitCode == 0) {
        std::cout << "Apksigner process executed successfully." << std::endl;
    }
    else {
        std::cerr << "Error: Apksigner process failed with exit code " << exitCode << std::endl;
        exit(-1);
    }

    // Construct the source and destination paths
    std::filesystem::path sourcePath = ".\\extracted\\aligned.apk";
    std::filesystem::path destinationPath = path.substr(0, path.find_last_of('.')) + "-patched.apk";

    // Perform the file copy
    try {
        std::filesystem::copy_file(sourcePath, destinationPath, std::filesystem::copy_options::overwrite_existing);
        std::cout << path << " patched!" << std::endl;
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

