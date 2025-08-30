#include "../include/InvertedIndex.h"
#include <sstream>
#include <unordered_map>

using namespace std;

void InvertedIndex::UpdateDocumentBase(const vector<string>& input_docs) {
    docs_ = input_docs;
    freq_dictionary_.clear();

    for (size_t doc_id = 0; doc_id < docs_.size(); ++doc_id) {
        unordered_map<string, size_t> word_counts;

        istringstream iss(docs_[doc_id]);
        string word;
        while (iss >> word) {
            ++word_counts[word];
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
