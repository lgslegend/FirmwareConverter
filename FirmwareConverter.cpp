#include "FirmwareConverter.h"
//查看帮助菜单
void help()
{
	std::cout << "FirmwareConverter 版本: " << VERSION << std::endl;
	std::cout << "用法: FirmwareConverter [选项] <参数>" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "选项:" << std::endl;
	std::cout << "\thelp\t查看帮助菜单" << std::endl;
	std::cout << "\tunpack <路径>\t卡刷包转线刷包" << std::endl;
	std::cout << "\trepack <路径>\t线刷包转卡刷包" << std::endl;
}
//卡刷包转线刷包
void OTA2Flash(std::string filePath, std::string fileType)
{
	std::string outputPath;
	std::filesystem::create_directories(OUTPUT_PATH);

	auto generateScript = [&outputPath]() {
		std::cout << "生成脚本" << std::endl;
		std::vector<std::string> imageList;
		for (const auto& entry : std::filesystem::directory_iterator(outputPath + "images\\")) {
			if (entry.is_regular_file()) {
				imageList.push_back(entry.path().filename().string());
			}
		}
		std::ofstream flashScript(outputPath + "flash_all.bat", std::ios::app);
		if (flashScript.is_open()) {
			for (const std::string& imageList : imageList) {
				flashScript << "fastboot %* flash " << std::filesystem::path(imageList).stem().string() << " %~dp0images\\" << imageList << " || @echo \"Flash " << std::filesystem::path(imageList).stem().string() << " error\" && exit /B 1" << std::endl;
			}
			flashScript.close();
		}
		else {
			std::cerr << "打开文件失败" << std::endl;
			return;
		}
		};
	auto AOnlyDeviceExtract = [&outputPath, &generateScript](std::string datPath) {
		std::cout << "检测到 A Only 类型刷机包" << std::endl;
		};
	auto ABDeviceExtract = [&outputPath, &generateScript](std::string payloadBinPath) {
		std::cout << "检测到 A/B 类型刷机包" << std::endl;
		std::cout << "解包 payload.bin" << std::endl;

		std::string time = std::to_string(static_cast<int>(std::time(nullptr))) + "\\";
		std::filesystem::create_directories(OUTPUT_PATH);
		std::filesystem::create_directories(OUTPUT_PATH + time);
		outputPath = OUTPUT_PATH + time;
		execute(PAYLOAD_DUMPER + " -output " + outputPath + "images\\ " + payloadBinPath);

		generateScript();
		};
	auto checkDeviceType = [&ABDeviceExtract, &AOnlyDeviceExtract](std::string path) {
		if (isPathValid(path + "payload.bin")) {
			ABDeviceExtract(path + "payload.bin");
		}
		else if (isPathValid(path + "system.new.dat")) {
			AOnlyDeviceExtract(path);
		}
		else {
			std::cout << "此文件或目录不是一个有效的卡刷包" << std::endl;
			return;
		}
		};

	if (fileType == "bin") {
		ABDeviceExtract(filePath);
	}
	else if (fileType == "zip") {
		std::cout << "解压刷机包" << std::endl;

		execute(SEVEN_ZIP + " x -y \"" + filePath + "\" -o\"" + TEMP_UNPACK_PATH + "\"");
		checkDeviceType(TEMP_UNPACK_PATH);
	}
	else {
		std::cout << "不支持的格式" << std::endl;
		return;
	}
}
//Main
int main(int argc, char* argv[])
{
	std::vector<std::string> arguments;
	for (int i = 0; i < argc; i++) {
		arguments.push_back(argv[i]);
	}

	if (argc == 3) {
		if (arguments[1] == "unpack") {
			if (!isPathValid(arguments[2])) {
				std::cout << arguments[2] << std::endl << "该文件或目录不存在" << std::endl;
				return 1;
			}
			OTA2Flash(arguments[2], getFileExtension(arguments[2]));
		}
		else if (arguments[1] == "repack") {
			;
		}
		else {
			help();
		}
	}
	else {
		help();
	}
	return 0;
}