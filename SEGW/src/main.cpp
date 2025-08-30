#include <iostream>
#include <chrono>
#include "ConverterJSON.h"
#include "InvertedIndex.h"
#include "SearchServer.h"

int main() {
    try {
        std::cout << "=== Search Engine Starting ===" << std::endl;
        
        // Засекаем время выполнения
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Инициализация JSON конвертера (с проверкой поля "name")
        std::cout << "\n1. Loading configuration..." << std::endl;
        ConverterJSON converter;
        
        std::cout << "Application: " << converter.GetAppName() 
                  << " v" << converter.GetVersion() << std::endl;
        
        // Загрузка документов
        std::cout << "\n2. Loading documents..." << std::endl;
        std::vector<std::string> documents = converter.GetTextDocuments();
        
        if (documents.empty()) {
            std::cerr << "Error: No documents loaded for indexing!" << std::endl;
            return 1;
        }
        
        // Создание и обновление инвертированного индекса (с многопоточностью)
        std::cout << "\n3. Building inverted index..." << std::endl;
        InvertedIndex index;
        
        auto indexStartTime = std::chrono::high_resolution_clock::now();
        index.UpdateDocumentBase(documents);
        auto indexEndTime = std::chrono::high_resolution_clock::now();
        
        auto indexDuration = std::chrono::duration_cast<std::chrono::milliseconds>
                            (indexEndTime - indexStartTime);
        
        // Получение статистики индекса
        auto stats = index.GetStats();
        std::cout << "Index built successfully:" << std::endl;
        std::cout << "  - Documents: " << stats.totalDocuments << std::endl;
        std::cout << "  - Unique words: " << stats.totalWords << std::endl;
        std::cout << "  - Total entries: " << stats.totalEntries << std::endl;
        std::cout << "  - Indexing time: " << indexDuration.count() << " ms" << std::endl;
        
        // Загрузка поисковых запросов
        std::cout << "\n4. Loading search requests..." << std::endl;
        std::vector<std::string> requests = converter.GetRequests();
        
        if (requests.empty()) {
            std::cout << "Warning: No search requests found. Creating example request." << std::endl;
            requests.push_back("example search query");
        }
        
        // Инициализация поискового сервера
        std::cout << "\n5. Processing search requests..." << std::endl;
        SearchServer searchServer(index);
        
        // Получение статистики поиска
        auto searchStats = searchServer.getSearchStats(requests);
        std::cout << "Search statistics:" << std::endl;
        std::cout << "  - Total queries: " << searchStats.totalQueries << std::endl;
        std::cout << "  - Queries with results: " << searchStats.queriesWithResults << std::endl;
        std::cout << "  - Average words per query: " << searchStats.averageWordsPerQuery << std::endl;
        
        // Выполнение поиска с многопоточностью
        auto searchStartTime = std::chrono::high_resolution_clock::now();
        std::vector<std::vector<RelativeIndex>> searchResults = 
            searchServer.search(requests, converter.GetResponsesLimit());
        auto searchEndTime = std::chrono::high_resolution_clock::now();
        
        auto searchDuration = std::chrono::duration_cast<std::chrono::milliseconds>
                             (searchEndTime - searchStartTime);
        
        std::cout << "Search completed in " << searchDuration.count() << " ms" << std::endl;
        
        // Сохранение результатов
        std::cout << "\n6. Saving results..." << std::endl;
        converter.putAnswers(searchResults);
        
        // Вывод краткой информации о результатах
        size_t totalResults = 0;
        size_t successfulQueries = 0;
        
        for (size_t i = 0; i < searchResults.size(); ++i) {
            if (!searchResults[i].empty()) {
                successfulQueries++;
                totalResults += searchResults[i].size();
                
                std::cout << "Query " << (i + 1) << ": " << searchResults[i].size() 
                         << " results" << std::endl;
            } else {
                std::cout << "Query " << (i + 1) << ": no results" << std::endl;
            }
        }
        
        // Общее время выполнения
        auto endTime = std::chrono::high_resolution_clock::now();
        auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>
                            (endTime - startTime);
        
        std::cout << "\n=== Search Engine Summary ===" << std::endl;
        std::cout << "Total execution time: " << totalDuration.count() << " ms" << std::endl;
        std::cout << "Successful queries: " << successfulQueries << "/" << requests.size() << std::endl;
        std::cout << "Total results found: " << totalResults << std::endl;
        std::cout << "Results saved to JSON/answers.json" << std::endl;
        std::cout << "\nSearch engine finished successfully!" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        std::cerr << "\nPlease check:" << std::endl;
        std::cerr << "1. config.json exists and contains required 'name' field" << std::endl;
        std::cerr << "2. All document files are accessible" << std::endl;
        std::cerr << "3. requests.json is properly formatted" << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred!" << std::endl;
        return 1;
    }
}