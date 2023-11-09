#include "FirmwareConverter.h"
//查看帮助菜单
void help()
{
	std::cout << "FirmwareConverter 版本: " << VERSION << std::endl;
	std::cout << "用法: FirmwareConverter [选项] <参数>" << std::endl << std::endl;
	std::cout << "选项:" << std::endl;
	std::cout << "\thelp\t\t查看帮助菜单" << std::endl;
	std::cout << "\tunpack <路径>\t卡刷包转线刷包" << std::endl;
	std::cout << "\trepack <路径>\t线刷包转卡刷包" << std::endl;
}
//卡刷包转线刷包
void OTA2Flash(std::string filePath, std::string fileType)
{
	std::string outputPath = OUTPUT_PATH + std::to_string(static_cast<int>(std::time(nullptr))) + "\\";

	std::filesystem::create_directories(outputPath + "\\images");
	std::filesystem::create_directories(TEMP_PATH);

	auto generateScript = [&outputPath](std::string slot = "") {
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
				flashScript << "fastboot %* flash " << std::filesystem::path(imageList).stem().string() << slot << " %~dp0images\\" << imageList << " || @echo \"Flash " << std::filesystem::path(imageList).stem().string() << " error\" && exit /B 1" << std::endl;
			}
			flashScript.close();
		}
		else {
			std::cerr << "打开文件失败" << std::endl;
			return;
		}
		};
	auto checkDeviceType = [&outputPath, &generateScript]() {
		if (isPathValid(TEMP_PATH + "payload.bin")) {
			std::cout << "检测到 A/B 类型刷机包" << std::endl;
			std::cout << "提取 payload.bin 中的系统镜像" << std::endl;
			execute(PAYLOAD_DUMPER + " -output " + outputPath + "images\\ " + TEMP_PATH + "payload.bin");
			std::string slot; std::cout << "刷入到<a/b/ab>槽位:"; std::cin >> slot; slot.insert(0, "_");
			generateScript(slot);
		}
		else if (isPathValid(TEMP_PATH + "system.new.dat") || isPathValid(TEMP_PATH + "vendor.new.dat") || isPathValid(TEMP_PATH + "system.new.dat.br") || isPathValid(TEMP_PATH + "vendor.new.dat.br")) {
			std::cout << "检测到 A Only 类型刷机包" << std::endl;

			if (isPathValid(TEMP_PATH + "system.new.dat.br")) {
				std::cout << "将 system.new.dat.br 转换成 system.new.dat" << std::endl;
				execute(BROTLI + " -d system.new.dat.br");
			}
			if (isPathValid(TEMP_PATH + "vendor.new.dat.br")) {
				std::cout << "将 vendor.new.dat.br 转换成 vendor.new.dat" << std::endl;
				execute(BROTLI + " -d vendor.new.dat.br");
			}
			if (isPathValid(TEMP_PATH + "system.new.dat")) {
				std::cout << "将 system.new.dat 转换成 system.img" << std::endl;
				execute(SDAT2IMG + " " + TEMP_PATH + "system.transfer.list " + TEMP_PATH + "system.new.dat " + outputPath + "images\\system.img");
			}
			if (isPathValid(TEMP_PATH + "vendor.new.dat")) {
				std::cout << "将 vendor.new.dat 转换成 vendor.img" << std::endl;
				execute(SDAT2IMG + " " + TEMP_PATH + "vendor.transfer.list " + TEMP_PATH + "vendor.new.dat " + outputPath + "images\\vendor.img");
			}
			std::cout << "提取刷机包中剩下的系统镜像" << std::endl;
			for (auto& entry : std::filesystem::recursive_directory_iterator(TEMP_PATH)) {
				std::string fileType = getFileExtension(entry.path().string());
				if (fileType == "img" || fileType == "mbn" || fileType == "bin" || fileType == "elf") {
					std::filesystem::rename(entry.path().string(), outputPath + "images\\" + entry.path().filename().string());
				}
			}
			generateScript();
		}
		else {
			std::cout << "此文件或目录不是一个有效的卡刷包" << std::endl;
			return;
		}
		};

	if (fileType == "bin") {
		checkDeviceType();
	}
	else if (fileType == "zip") {
		std::cout << "解压刷机包" << std::endl;

		execute(SEVEN_ZIP + " x -y \"" + filePath + "\" -o\"" + TEMP_PATH + "\"");
		checkDeviceType();
	}
	else {
		std::cout << "不支持的格式:" << fileType << std::endl;
		return;
	}
	std::cout << "转换成功, 输出到:" << outputPath << std::endl;
}
//Main
int main(int argc, char* argv[])
{
	std::filesystem::remove_all(TEMP_PATH);
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
	std::filesystem::remove_all(TEMP_PATH);
	return 0;
}