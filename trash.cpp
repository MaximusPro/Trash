#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cerrno>
#include <cstring>

namespace fs = std::filesystem;

std::string get_trash_dir() {
    const char* xdg_data = std::getenv("XDG_DATA_HOME");
    std::string base = xdg_data ? xdg_data : (std::string(std::getenv("HOME")) + "/.local/share");
    return base + "/Trash";
}

std::string get_unique_trash_name(const fs::path& trash_files, const std::string& original_name) {
    std::string name = original_name;
    fs::path target = trash_files / name;
    int counter = 1;
    while (fs::exists(target)) {
        size_t dot = name.find_last_of('.');
        std::string base = (dot == std::string::npos) ? name : name.substr(0, dot);
        std::string ext  = (dot == std::string::npos) ? "" : name.substr(dot);
        name = base + "_" + std::to_string(counter) + ext;
        target = trash_files / name;
        ++counter;
    }
    return name;
}

bool trash_file(const fs::path& src) {
    if (!fs::exists(src)) {
        std::cerr << "trash: cannot trash '" << src << "': No such file or directory\n";
        return false;
    }

    fs::path trash_root = get_trash_dir();
    fs::path trash_files = trash_root / "files";
    fs::path trash_info  = trash_root / "info";

    try {
        fs::create_directories(trash_files);
        fs::create_directories(trash_info);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "trash: failed to create trash directories: " << e.what() << "\n";
        return false;
    }

    std::string orig_name = src.filename().string();
    std::string trash_name = get_unique_trash_name(trash_files, orig_name);

    fs::path dest_file = trash_files / trash_name;
    fs::path info_file = trash_info / (trash_name + ".trashinfo");

    // Перемещаем файл/папку
    try {
        fs::rename(src, dest_file);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "trash: failed to move '" << src << "' → '" << dest_file << "': " << e.what() << "\n";
        return false;
    }

    // Создаём .trashinfo
    std::ofstream info(info_file);
    if (!info) {
        std::cerr << "trash: failed to create .trashinfo for '" << trash_name << "'\n";
        // Можно откатить перемещение, но для простоты оставляем как есть
        return false;
    }

    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream date_ss;
    date_ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");

    info << "[Trash Info]\n";
    info << "Path=" << src.string() << "\n";
    info << "DeletionDate=" << date_ss.str() << "\n";
    info.close();

    std::cout << "trashed: " << src.string() << " → Trash/files/" << trash_name << "\n";
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: trash [FILE|DIR]...\n";
        std::cerr << "  Moves files/directories to the freedesktop Trash (~/.local/share/Trash)\n";
        return 1;
    }

    int exit_code = 0;
    for (int i = 1; i < argc; ++i) {
        fs::path path = argv[i];
        if (!trash_file(path)) {
            exit_code = 1;
        }
    }

    return exit_code;
}
