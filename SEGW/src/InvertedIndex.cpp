#include "InvertedIndex.h"
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <cctype>

using namespace std;

void InvertedIndex::UpdateDocumentBase(const vector<string>& input_docs) {
    docs_ = input_docs;
    freq_dictionary_.clear();

    for (size_t doc_id = 0; doc_id < docs_.size(); ++doc_id) {
        unordered_map<string, size_t> word_counts;

        istringstream iss(docs_[doc_id]);
        string word;
        while (iss >> word) {
            // Нормализация слова
            string normalized_word;
            for (char c : word) {
                if (isalpha(static_cast<unsigned char>(c))) {
                    normalized_word += tolower(static_cast<unsigned char>(c));
                }
            }
            
            if (!normalized_word.empty() && normalized_word.length() <= 100) {
                ++word_counts[normalized_word];
            }
        }

        for (const auto& [word, count] : word_counts) {
            freq_dictionary_[word].push_back({doc_id, count});
        }
    }
}

vector<Entry> InvertedIndex::GetWordCount(const string& word) const {
    if (auto it = freq_dictionary_.find(word); it != freq_dictionary_.end()) {
        return it->second;
    }
    return {};
}

// Добавляем недостающие методы для SearchServer
bool InvertedIndex::ContainsWord(const string& word) const {
    return freq_dictionary_.find(word) != freq_dictionary_.end();
}

IndexStats InvertedIndex::GetStats() const {
    IndexStats stats;
    stats.totalDocuments = docs_.size();
    stats.totalWords = freq_dictionary_.size();
    
    for (const auto& [word, entries] : freq_dictionary_) {
        stats.totalEntries += entries.size();
    }
    
    return stats;
}
