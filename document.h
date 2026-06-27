#pragma once
#include <Arduino.h>
#include <vector>

class Document {
public:
    bool begin();
    std::vector<String> listDocuments() const;
    bool open(const String& path);
    bool createNew();

    const String& getText() const;
    const String& getPath() const;
    String getTitle() const;

    void insertChar(char c);
    void backspace();

    bool save();

private:
    String currentPath;
    String text;
};

extern Document document;
