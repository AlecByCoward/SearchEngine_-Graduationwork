#pragma once
#include "InvertedIndex.h"
#include "ConverterJSON.h"
#include <vector>
#include <string>
#include <set>

//Класс для обработки поисковых запросов
class SearchServer {
public:
    //Структура для статистики поиска
    struct SearchStats {
        size_t totalQueries = 0;           // Общее количество запросов
        size_t queriesWithResults = 0;     // Запросы с результатами
        float averageWordsPerQuery = 0.0f; // Среднее количество слов в запросе
    };

private:
    static const size_t MAX_WORD_LENGTH = 100; 
    
    InvertedIndex& index; 
    std::string normalizeWord(const std::string& word) const;
    std::vector<std::string> splitQuery(const std::string& query) const;
    float calculateRelevance(const std::vector<std::string>& queryWords, 
                            size_t docId) const;
    std::vector<RelativeIndex> processQuery(const std::string& query, 
                                          size_t maxResponses) const;

public:
    SearchServer(InvertedIndex& idx);
    ~SearchServer();
    
    std::vector<std::vector<RelativeIndex>> search(
        const std::vector<std::string>& queries_input, 
        size_t maxResponses = 5) const;
    SearchStats getSearchStats(const std::vector<std::string>& queries_input) const;
};