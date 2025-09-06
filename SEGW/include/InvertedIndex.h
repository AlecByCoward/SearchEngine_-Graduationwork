#pragma once

#include <string>
#include <vector>
#include <map>

using namespace std;

struct Entry {
    size_t doc_id;
    size_t count;

    bool operator==(const Entry& other) const {
        return doc_id == other.doc_id && count == other.count;
    }
};

// Структура для статистики индекса
struct IndexStats {
    size_t totalDocuments = 0;
    size_t totalWords = 0;
    size_t totalEntries = 0;
};

class InvertedIndex {
public:
    void UpdateDocumentBase(const vector<string>& input_docs);
    vector<Entry> GetWordCount(const string& word) const;
    size_t GetDocumentCount() const { return docs_.size(); }
    bool ContainsWord(const string& word) const;
    IndexStats GetStats() const;

private:
    vector<string> docs_;
    map<string, vector<Entry>> freq_dictionary_;
};
