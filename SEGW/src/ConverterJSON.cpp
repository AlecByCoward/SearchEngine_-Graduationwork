#include "ConverterJSON.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <stdexcept>

// Конструктор
ConverterJSON::ConverterJSON() : max_responses(5), auto_discover_files(false), max_files_to_process(10), resources_directory("resources") {
    loadConfig();
}

// Деструктор
ConverterJSON::~ConverterJSON() {}

// Поиск файла в возможных локациях
std::string ConverterJSON::findFile(const std::string& filename) const {
    // Получаем путь к исполняемому файлу
    std::string executablePath = std::filesystem::current_path();
    
    // Определяем базовую директорию проекта
    std::string projectRoot;
    
    // Если мы в папке build, поднимаемся на уровень выше
    if (executablePath.length() >= 6 && 
        (executablePath.substr(executablePath.length() - 6) == "/build" ||
         executablePath.substr(executablePath.length() - 6) == "\\build")) {
        projectRoot = executablePath.substr(0, executablePath.length() - 6); // убираем "/build"
    } else {
        // Проверяем, есть ли папка SEGW в текущей директории
        if (std::filesystem::exists(executablePath + "/SEGW")) {
            projectRoot = executablePath + "/SEGW";
        } else {
            // Иначе используем текущую директорию
            projectRoot = executablePath;
        }
    }
    
    std::vector<std::string> searchPaths = {
        filename,                                    // Текущая директория
        "./" + filename,                            // Явно текущая директория
        "../" + filename,                           // Родительская директория
        "../../" + filename,                        // На два уровня выше
        projectRoot + "/" + filename,               // В корне проекта
        projectRoot + "/JSON/" + filename,          // В папке JSON проекта
        projectRoot + "/resources/" + filename,      // В папке resources проекта
        "./JSON/" + filename,                       // В поддиректории JSON
        "../JSON/" + filename,                      // JSON в родительской
        "./config/" + filename,                     // В поддиректории config
        "../config/" + filename,                    // config в родительской
        "../../JSON/" + filename,                   // JSON на два уровня выше
        "../../resources/" + filename                // resources на два уровня выше
    };

    for (const auto& path : searchPaths) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }

    throw std::runtime_error("File not found: " + filename +
                           ". Searched in multiple locations including: " + 
                           projectRoot + "/JSON/");
}

// Загрузка конфигурации
void ConverterJSON::loadConfig() {
    try {
        std::string configPath = findFile("config.json");
        std::ifstream configFile(configPath);

        if (!configFile.is_open()) {
            throw std::runtime_error("Cannot open config file: " + configPath);
        }

        nlohmann::json configJson;
        configFile >> configJson;

        // ИСПРАВЛЕНИЕ: Проверка наличия обязательного поля "name"
        if (!configJson.contains("config")) {
            throw std::runtime_error("Missing 'config' section in config.json");
        }

        auto config = configJson["config"];

        if (!config.contains("name")) {
            throw std::runtime_error("Missing required field 'name' in config.json");
        }

        if (config["name"].get<std::string>().empty()) {
            throw std::runtime_error("Field 'name' cannot be empty in config.json");
        }

        // Сохранение данных конфигурации
        appName = config["name"].get<std::string>();

        if (config.contains("version")) {
            version = config["version"].get<std::string>();
        } else {
            version = "0.1"; // Значение по умолчанию
        }

        if (config.contains("max_responses")) {
            max_responses = config["max_responses"].get<size_t>();
            if (max_responses == 0) {
                std::cerr << "Warning: max_responses is 0, setting to 1" << std::endl;
                max_responses = 1;
            }
        }

        // Загрузка новых параметров
        if (config.contains("auto_discover_files")) {
            auto_discover_files = config["auto_discover_files"].get<bool>();
        }

        if (config.contains("max_files_to_process")) {
            max_files_to_process = config["max_files_to_process"].get<size_t>();
        }

        if (config.contains("resources_directory")) {
            resources_directory = config["resources_directory"].get<std::string>();
        }

        // Загрузка списка файлов или автоматическое обнаружение
        if (auto_discover_files) {
            discoverFiles();
        } else {
            // Старый способ - загрузка из списка files
            if (configJson.contains("files") && configJson["files"].is_array()) {
                for (const auto& file : configJson["files"]) {
                    if (file.is_string()) {
                        filePaths.push_back(file.get<std::string>());
                    }
                }
            }
        }

        if (filePaths.empty()) {
            throw std::runtime_error("No files found for processing");
        }

        std::cout << "Configuration loaded successfully:" << std::endl;
        std::cout << "  Name: " << appName << std::endl;
        std::cout << "  Version: " << version << std::endl;
        std::cout << "  Max responses: " << max_responses << std::endl;
        std::cout << "  Auto discover files: " << (auto_discover_files ? "enabled" : "disabled") << std::endl;
        if (auto_discover_files) {
            std::cout << "  Max files to process: " << max_files_to_process << std::endl;
            std::cout << "  Resources directory: " << resources_directory << std::endl;
        }
        std::cout << "  Files count: " << filePaths.size() << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
        throw;
    }
}

// Получение списка файлов для индексации
std::vector<std::string> ConverterJSON::GetTextDocuments() const {
    std::vector<std::string> documents;
    documents.reserve(filePaths.size());

    for (size_t i = 0; i < filePaths.size(); ++i) {
        try {
            std::string fullPath = findFile(filePaths[i]);
            std::ifstream file(fullPath);

            if (!file.is_open()) {
                std::cerr << "Warning: Cannot open file: " << filePaths[i] << std::endl;
                documents.push_back(""); // Добавляем пустой документ
                continue;
            }

            std::string content;
            std::string line;

            while (std::getline(file, line)) {
                content += line + " ";
            }

            documents.push_back(content);
            std::cout << "Loaded document " << i << ": " << filePaths[i]
                     << " (" << content.length() << " chars)" << std::endl;

        } catch (const std::exception& e) {
            std::cerr << "Error reading file " << filePaths[i] << ": " << e.what() << std::endl;
            documents.push_back(""); // Добавляем пустой документ
        }
    }

    return documents;
}

// Получение поисковых запросов
std::vector<std::string> ConverterJSON::GetRequests() const {
    std::vector<std::string> requests;

    try {
        std::string requestsPath = findFile("requests.json");
        std::ifstream requestsFile(requestsPath);

        if (!requestsFile.is_open()) {
            std::cerr << "Warning: Cannot open requests file: " << requestsPath << std::endl;
            return requests;
        }

        nlohmann::json requestsJson;
        requestsFile >> requestsJson;

        if (requestsJson.contains("requests") && requestsJson["requests"].is_array()) {
            for (const auto& request : requestsJson["requests"]) {
                if (request.is_string() && !request.get<std::string>().empty()) {
                    requests.push_back(request.get<std::string>());
                }
            }
        }

        std::cout << "Loaded " << requests.size() << " requests" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error reading requests: " << e.what() << std::endl;
    }

    return requests;
}

// Сохранение результатов поиска
void ConverterJSON::putAnswers(const std::vector<std::vector<RelativeIndex>>& answers) const {
    nlohmann::json answersJson;

    for (size_t i = 0; i < answers.size(); ++i) {
        std::string requestId = "request" + std::string(3 - std::to_string(i + 1).length(), '0') + std::to_string(i + 1);

        if (answers[i].empty()) {
            answersJson["answers"][requestId]["result"] = false;
        } else {
            answersJson["answers"][requestId]["result"] = true;

            // Ограничиваем количество результатов согласно max_responses
            size_t responseCount = std::min(answers[i].size(), max_responses);

            if (responseCount == 1) {
                // Если только один результат, сохраняем как объект
                answersJson["answers"][requestId]["docid"] = answers[i][0].doc_id;
                answersJson["answers"][requestId]["rank"] = answers[i][0].rank;
            } else {
                // Если несколько результатов, сохраняем как массив
                for (size_t j = 0; j < responseCount; ++j) {
                    nlohmann::json relevanceItem;
                    relevanceItem["docid"] = answers[i][j].doc_id;
                    relevanceItem["rank"] = answers[i][j].rank;
                    answersJson["answers"][requestId]["relevance"].push_back(relevanceItem);
                }
            }
        }
    }

    try {
        // Создаем директорию JSON если её нет
        std::filesystem::create_directories("JSON");

        std::ofstream answersFile("JSON/answers.json");
        if (!answersFile.is_open()) {
            std::cerr << "Error: Cannot create answers file" << std::endl;
            return;
        }

        answersFile << answersJson.dump(4);
        std::cout << "Results saved to JSON/answers.json" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error saving answers: " << e.what() << std::endl;
    }
}

// Получение максимального количества ответов
size_t ConverterJSON::GetResponsesLimit() const {
    return max_responses;
}

// Получение имени приложения
std::string ConverterJSON::GetAppName() const {
    return appName;
}

// Получение версии приложения
std::string ConverterJSON::GetVersion() const {
    return version;
}

// Автоматическое обнаружение файлов в папке resources
void ConverterJSON::discoverFiles() {
    try {
        // Получаем путь к папке resources
        std::string resourcesPath = findFile(resources_directory);
        
        if (!std::filesystem::exists(resourcesPath)) {
            throw std::runtime_error("Resources directory not found: " + resources_directory);
        }

        if (!std::filesystem::is_directory(resourcesPath)) {
            throw std::runtime_error("Resources path is not a directory: " + resourcesPath);
        }

        // Получаем список всех файлов в папке
        std::vector<std::string> allFiles;
        for (const auto& entry : std::filesystem::directory_iterator(resourcesPath)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                
                // Проверяем расширение файла
                bool supportedExtension = false;
                std::vector<std::string> supportedExtensions = {".txt", ".md"};
                for (const auto& ext : supportedExtensions) {
                    if (filename.length() >= ext.length() && 
                        filename.substr(filename.length() - ext.length()) == ext) {
                        supportedExtension = true;
                        break;
                    }
                }
                
                if (supportedExtension) {
                    allFiles.push_back(resources_directory + "/" + filename);
                }
            }
        }

        // Сортируем файлы по имени для предсказуемого порядка
        std::sort(allFiles.begin(), allFiles.end());

        // Ограничиваем количество файлов
        size_t filesToAdd = std::min(allFiles.size(), max_files_to_process);
        filePaths.reserve(filesToAdd);

        for (size_t i = 0; i < filesToAdd; ++i) {
            filePaths.push_back(allFiles[i]);
        }

        std::cout << "Auto-discovered " << filePaths.size() << " files from " << resources_directory << std::endl;
        if (allFiles.size() > max_files_to_process) {
            std::cout << "  (Limited to " << max_files_to_process << " files)" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error discovering files: " << e.what() << std::endl;
        throw;
    }
}
