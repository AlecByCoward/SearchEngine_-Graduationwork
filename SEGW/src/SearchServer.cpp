#include "SearchServer.h"
#include <algorithm>
#include <sstream>
#include <cmath>
#include <thread>
#include <mutex>
#include <future>

// Конструктор
SearchServer::SearchServer(InvertedIndex& idx) : index(idx) {}

// Деструктор
SearchServer::~SearchServer() {}

// Нормализация слова (аналогично InvertedIndex)
std::string SearchServer::normalizeWord(const std::string& word) const {
    std::string normalized;
    normalized.reserve(word.size());
    
    for (char c : word) {
        if (std::isalpha(static_cast<unsigned char>(c))) {
            normalized += std::tolower(static_cast<unsigned char>(c));
        }
    }
    
    if (normalized.length() > MAX_WORD_LENGTH) {
        normalized = normalized.substr(0, MAX_WORD_LENGTH);
    }
    
    return normalized;
}

// Разбивка запроса на слова
std::vector<std::string> SearchServer::splitQuery(const std::string& query) const {
    std::vector<std::string> words;
    std::istringstream stream(query);
    std::string word;
    
    while (stream >> word) {
        std::string normalizedWord = normalizeWord(word);
        if (!normalizedWord.empty()) {
            words.push_back(normalizedWord);
        }
    }
    
    // Удаляем дубликаты и сортируем для оптимизации
    std::sort(words.begin(), words.end());
    words.erase(std::unique(words.begin(), words.end()), words.end());
    
    return words;
}

// Вычисление релевантности документа для запроса
float SearchServer::calculateRelevance(const std::vector<std::string>& queryWords, 
                                     size_t docId) const {
    float relevance = 0.0f;
    size_t totalWordsInQuery = queryWords.size();
    
    if (totalWordsInQuery == 0) {
        return 0.0f;
    }
    
    // Подсчитываем абсолютную релевантность
    size_t absoluteRelevance = 0;
    
    for (const std::string& word : queryWords) {
        auto wordCount = index.GetWordCount(word);
        
        // Ищем документ в списке
        for (const auto& entry : wordCount) {
            if (entry.doc_id == docId) {
                absoluteRelevance += entry.count;
                break;
            }
        }
    }
    
    // Нормализуем релевантность (простая нормализация)
    if (absoluteRelevance > 0) {
        // Можно использовать более сложные алгоритмы типа TF-IDF
        relevance = static_cast<float>(absoluteRelevance);
    }
    
    return relevance;
}

// Обработка одного запроса
std::vector<RelativeIndex> SearchServer::processQuery(const std::string& query, 
                                                    size_t maxResponses) const {
    std::vector<RelativeIndex> result;
    
    // Разбиваем запрос на слова
    std::vector<std::string> queryWords = splitQuery(query);
    
    if (queryWords.empty()) {
        return result; // Пустой результат для пустого запроса
    }
    
    // Находим все документы, содержащие хотя бы одно слово из запроса
    std::set<size_t> candidateDocuments;
    
    for (const std::string& word : queryWords) {
        if (index.ContainsWord(word)) {
            auto wordCount = index.GetWordCount(word);
            for (const auto& entry : wordCount) {
                candidateDocuments.insert(entry.doc_id);
            }
        }
    }
    
    // Вычисляем релевантность для каждого документа-кандидата
    std::vector<std::pair<size_t, float>> documentRelevance;
    documentRelevance.reserve(candidateDocuments.size());
    
    for (size_t docId : candidateDocuments) {
        float relevance = calculateRelevance(queryWords, docId);
        if (relevance > 0.0f) {
            documentRelevance.emplace_back(docId, relevance);
        }
    }
    
    // Сортируем по убыванию релевантности
    std::sort(documentRelevance.begin(), documentRelevance.end(),
              [](const auto& a, const auto& b) {
                  if (std::abs(a.second - b.second) < 0.001f) {
                      return a.first < b.first; // При равной релевантности сортируем по ID
                  }
                  return a.second > b.second;
              });
    
    // Нормализуем релевантность к диапазону [0, 1]
    if (!documentRelevance.empty()) {
        float maxRelevance = documentRelevance[0].second;
        if (maxRelevance > 0.0f) {
            for (auto& [docId, relevance] : documentRelevance) {
                relevance /= maxRelevance;
            }
        }
    }
    
    // Ограничиваем количество результатов
    size_t resultCount = std::min(documentRelevance.size(), maxResponses);
    result.reserve(resultCount);
    
    for (size_t i = 0; i < resultCount; ++i) {
        result.emplace_back(documentRelevance[i].first, documentRelevance[i].second);
    }
    
    return result;
}

// Обработка множественных запросов с многопоточностью
std::vector<std::vector<RelativeIndex>> SearchServer::search(
    const std::vector<std::string>& queries_input, size_t maxResponses) const {
    
    std::vector<std::vector<RelativeIndex>> results(queries_input.size());
    
    if (queries_input.empty()) {
        return results;
    }
    
    // Определяем количество потоков
    const size_t numThreads = std::min(
        static_cast<size_t>(std::thread::hardware_concurrency()),
        queries_input.size()
    );
    
    if (numThreads <= 1) {
        // Однопоточная обработка для малого количества запросов
        for (size_t i = 0; i < queries_input.size(); ++i) {
            results[i] = processQuery(queries_input[i], maxResponses);
        }
    } else {
        // Многопоточная обработка
        std::vector<std::future<std::vector<RelativeIndex>>> futures;
        futures.reserve(queries_input.size());
        
        // Запускаем асинхронные задачи для каждого запроса
        for (size_t i = 0; i < queries_input.size(); ++i) {
            futures.emplace_back(
                std::async(std::launch::async, [this, &queries_input, i, maxResponses]() {
                    return processQuery(queries_input[i], maxResponses);
                })
            );
        }
        
        // Собираем результаты
        for (size_t i = 0; i < futures.size(); ++i) {
            results[i] = futures[i].get();
        }
    }
    
    return results;
}

// Получение статистики поиска
SearchServer::SearchStats SearchServer::getSearchStats(
    const std::vector<std::string>& queries_input) const {
    
    SearchStats stats;
    stats.totalQueries = queries_input.size();
    stats.averageWordsPerQuery = 0.0f;
    stats.queriesWithResults = 0;
    
    if (queries_input.empty()) {
        return stats;
    }
    
    size_t totalWords = 0;
    
    for (const std::string& query : queries_input) {
        std::vector<std::string> words = splitQuery(query);
        totalWords += words.size();
        
        // Проверяем, есть ли результаты для запроса
        bool hasResults = false;
        for (const std::string& word : words) {
            if (index.ContainsWord(word)) {
                hasResults = true;
                break;
            }
        }
        
        if (hasResults) {
            stats.queriesWithResults++;
        }
    }
    
    stats.averageWordsPerQuery = static_cast<float>(totalWords) / 
                                static_cast<float>(queries_input.size());
    
    return stats;
}