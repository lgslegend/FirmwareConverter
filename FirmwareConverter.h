#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

//配置
const std::string VERSION = "0.1.0";

const std::string OUTPUT_PATH = "output\\";
const std::string TEMP_PATH = "temp\\";

const std::string PAYLOAD_DUMPER = "bin\\payload-dumper\\payload-dumper-go.exe";
const std::string SEVEN_ZIP = "bin\\7zip\\7z.exe";
const std::string BROTLI = "bin\\brotli\\brotli.exe";
const std::string SDAT2IMG = "bin\\sdat2img\\sdat2img.exe";

std::vector<std::string> imagesDirectoryPath = { "firmware-update\\" };





//执行程序
std::string execute(std::string command) {
    std::string result;
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe) {
        return "Failed to execute command.";
    }
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result.append(buffer);
    }
    _pclose(pipe);
    return result;
}
//判断文件类型
std::string getFileExtension(std::string fileName = "")
{
    size_t lastDotPos = fileName.find_last_of('.');
    if (lastDotPos != std::string::npos) {
        return(fileName.substr(lastDotPos + 1));
    }
    else {
        return "";
    }
}
//判断路径是否有效
bool isPathValid(const std::string filePath) {
    try {
        std::filesystem::path fsPath(filePath);
        std::string pathStr = fsPath.generic_string();

        if (std::filesystem::exists(pathStr)) {
            return true;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return false;
}