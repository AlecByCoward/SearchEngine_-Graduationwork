#pragma once
#include <vector>
#include <string>
#include <nlohmann/json.hpp>

//Структура для хранения относительного индекса релевантности
struct RelativeIndex {
    size_t doc_id;  
    float rank;     
    
    RelativeIndex() = default;
    RelativeIndex(size_t id, float r) : doc_id(id), rank(r) {}
    
    bool operator==(const RelativeIndex& other) const {
        return doc_id == other.doc_id && std::abs(rank - other.rank) < 0.001f;
    }
    
    bool operator>(const RelativeIndex& other) const {
        return rank > other.rank;
    }
    
    bool operator<(const RelativeIndex& other) const {
        return rank < other.rank;
    }
};

//Класс для работы с JSON-файлами конфигурации, запросов и ответов
class ConverterJSON {
private:
    std::vector<std::string> filePaths; 
    size_t max_responses;               
    std::string appName;                
    std::string version;                 

    std::string findFile(const std::string& filename) const;
    void loadConfig();

public:
    ConverterJSON();
    ~ConverterJSON();
    
    std::vector<std::string> GetTextDocuments() const;
    std::vector<std::string> GetRequests() const;
    void putAnswers(const std::vector<std::vector<RelativeIndex>>& answers) const;
    size_t GetResponsesLimit() const;
    std::string GetAppName() const;
    std::string GetVersion() const;
};