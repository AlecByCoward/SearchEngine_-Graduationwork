#include <gtest/gtest.h>
#include "../SEGW/include/ConverterJSON.h"
#include <fstream>
#include <filesystem>

class ConverterJSONTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создаем тестовые JSON файлы
        createTestConfigFile();
        createTestRequestsFile();
        createTestDocuments();
    }
    
    void TearDown() override {
        // Удаляем тестовые файлы
        cleanupTestFiles();
    }
    
    void createTestConfigFile() {
        nlohmann::json configJson = {
            {"config", {
                {"name", "Test Search Engine"},
                {"version", "1.0"},
                {"max_responses", 3}
            }},
            {"files", {
                "test_resources/test_file1.txt",
                "test_resources/test_file2.txt"
            }}
        };
        
        std::ofstream file("config.json");
        file << configJson.dump(4);
    }
    
    void createTestRequestsFile() {
        nlohmann::json requestsJson = {
            {"requests", {
                "test query one",
                "another test query",
                "final search"
            }}
        };
        
        std::ofstream file("requests.json");
        file << requestsJson.dump(4);
    }
    
    void createTestDocuments() {
        std::filesystem::create_directories("test_resources");
        
        std::ofstream file1("test_resources/test_file1.txt");
        file1 << "This is a test document with various words for testing search functionality.";
        
        std::ofstream file2("test_resources/test_file2.txt");
        file2 << "Another document containing different content for comprehensive testing purposes.";
    }
    
    void cleanupTestFiles() {
        std::filesystem::remove("config.json");
        std::filesystem::remove("requests.json");
        std::filesystem::remove_all("test_resources");
        std::filesystem::remove_all("JSON");
    }
};

// Тест проверки обязательного поля "name" в config.json
TEST_F(ConverterJSONTest, RequiredNameFieldPresent) {
    // Создаем конфигурацию с полем "name"
    nlohmann::json validConfig = {
        {"config", {
            {"name", "Valid Search Engine"},
            {"version", "1.0"},
            {"max_responses", 5}
        }},
        {"files", {"test.txt"}}
    };
    
    std::ofstream file("config.json");
    file << validConfig.dump(4);
    file.close();
    
    // Должно работать без исключений
    EXPECT_NO_THROW({
        ConverterJSON converter;
        EXPECT_EQ(converter.GetAppName(), "Valid Search Engine");
    });
}

// Тест отсутствия обязательного поля "name"
TEST_F(ConverterJSONTest, RequiredNameFieldMissing) {
    // Создаем конфигурацию БЕЗ поля "name"
    nlohmann::json invalidConfig = {
        {"config", {
            {"version", "1.0"},
            {"max_responses", 5}
        }},
        {"files", {"test.txt"}}
    };
    
    std::ofstream file("config.json");
    file << invalidConfig.dump(4);
    file.close();
    
    // Должно выбросить исключение
    EXPECT_THROW({
        ConverterJSON converter;
    }, std::runtime_error);
}

// Тест пустого поля "name"
TEST_F(ConverterJSONTest, EmptyNameField) {
    nlohmann::json invalidConfig = {
        {"config", {
            {"name", ""},  // Пустое имя
            {"version", "1.0"},
            {"max_responses", 5}
        }},
        {"files", {"test.txt"}}
    };
    
    std::ofstream file("config.json");
    file << invalidConfig.dump(4);
    file.close();
    
    // Должно выбросить исключение для пустого имени
    EXPECT_THROW({
        ConverterJSON converter;
    }, std::runtime_error);
}

// Тест отсутствия секции "config"
TEST_F(ConverterJSONTest, MissingConfigSection) {
    nlohmann::json invalidConfig = {
        {"files", {"test.txt"}}
        // Нет секции "config"
    };
    
    std::ofstream file("config.json");
    file << invalidConfig.dump(4);
    file.close();
    
    EXPECT_THROW({
        ConverterJSON converter;
    }, std::runtime_error);
}

// Тест загрузки документов
TEST_F(ConverterJSONTest, LoadTextDocuments) {
    ConverterJSON converter;
    
    std::vector<std::string> documents = converter.GetTextDocuments();
    
    EXPECT_EQ(documents.size(), 2);
    EXPECT_GT(documents[0].length(), 0);
    EXPECT_GT(documents[1].length(), 0);
    
    // Проверяем содержимое
    EXPECT_TRUE(documents[0].find("test document") != std::string::npos);
    EXPECT_TRUE(documents[1].find("Another document") != std::string::npos);
}

// Тест загрузки запросов
TEST_F(ConverterJSONTest, LoadRequests) {
    ConverterJSON converter;
    
    std::vector<std::string> requests = converter.GetRequests();
    
    EXPECT_EQ(requests.size(), 3);
    EXPECT_EQ(requests[0], "test query one");
    EXPECT_EQ(requests[1], "another test query");
    EXPECT_EQ(requests[2], "final search");
}

// Тест получения лимита ответов
TEST_F(ConverterJSONTest, ResponsesLimit) {
    ConverterJSON converter;
    
    EXPECT_EQ(converter.GetResponsesLimit(), 3);
}

// Тест сохранения результатов
TEST_F(ConverterJSONTest, SaveAnswers) {
    ConverterJSON converter;
    
    // Создаем тестовые результаты
    std::vector<std::vector<RelativeIndex>> answers = {
        // Первый запрос - несколько результатов
        {
            RelativeIndex(0, 0.9f),
            RelativeIndex(1, 0.7f)
        },
        // Второй запрос - один результат
        {
            RelativeIndex(1, 0.8f)
        },
        // Третий запрос - нет результатов
        {}
    };
    
    EXPECT_NO_THROW({
        converter.putAnswers(answers);
    });
    
    // Проверяем, что файл создался
    EXPECT_TRUE(std::filesystem::exists("JSON/answers.json"));
    
    // Проверяем содержимое файла
    std::ifstream file("JSON/answers.json");
    nlohmann::json savedAnswers;
    file >> savedAnswers;
    
    EXPECT_TRUE(savedAnswers["answers"]["request001"]["result"].get<bool>());
    EXPECT_EQ(savedAnswers["answers"]["request001"]["relevance"].size(), 2);
    
    EXPECT_TRUE(savedAnswers["answers"]["request002"]["result"].get<bool>());
    EXPECT_EQ(savedAnswers["answers"]["request002"]["docid"].get<size_t>(), 1);
    
    EXPECT_FALSE(savedAnswers["answers"]["request003"]["result"].get<bool>());
}

// Тест обработки отсутствующих файлов
TEST_F(ConverterJSONTest, MissingDocumentFiles) {
    // Создаем конфигурацию с несуществующим файлом
    nlohmann::json configWithMissingFile = {
        {"config", {
            {"name", "Test Engine"},
            {"version", "1.0"},
            {"max_responses", 5}
        }},
        {"files", {
            "test_resources/test_file1.txt",
            "nonexistent_file.txt",  // Этот файл не существует
            "test_resources/test_file2.txt"
        }}
    };
    
    std::ofstream file("config.json");
    file << configWithMissingFile.dump(4);
    file.close();
    
    ConverterJSON converter;
    std::vector<std::string> documents = converter.GetTextDocuments();
    
    // Должно быть 3 документа, но один пустой
    EXPECT_EQ(documents.size(), 3);
    EXPECT_GT(documents[0].length(), 0);  // Первый файл
    EXPECT_EQ(documents[1].length(), 0);  // Несуществующий файл - пустая строка
    EXPECT_GT(documents[2].length(), 0);  // Третий файл
}

// Тест валидации max_responses
TEST_F(ConverterJSONTest, MaxResponsesValidation) {
    // Тест с max_responses = 0
    nlohmann::json configZeroMax = {
        {"config", {
            {"name", "Test Engine"},
            {"max_responses", 0}
        }},
        {"files", {"test.txt"}}
    };
    
    std::ofstream file("test_config.json");
    file << configZeroMax.dump(4);
    file.close();
    
    ConverterJSON converter;
    // max_responses должно автоматически установиться в 1
    EXPECT_GE(converter.GetResponsesLimit(), 1);
}

// Тест значений по умолчанию
TEST_F(ConverterJSONTest, DefaultValues) {
    // Конфигурация с минимальным набором полей
    nlohmann::json minimalConfig = {
        {"config", {
            {"name", "Minimal Engine"}
        }},
        {"files", {"test.txt"}}
    };
    
    std::ofstream file("config.json");
    file << minimalConfig.dump(4);
    file.close();
    
    ConverterJSON converter;
    
    EXPECT_EQ(converter.GetAppName(), "Minimal Engine");
    EXPECT_EQ(converter.GetVersion(), "0.1");  // Значение по умолчанию
    EXPECT_GT(converter.GetResponsesLimit(), 0);  // Должно быть больше 0
}