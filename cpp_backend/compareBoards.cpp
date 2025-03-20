#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>
#include <algorithm>
#include <cctype>
#include <iterator>

using namespace std;

// Helper: trim leading and trailing whitespace.
string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Normalize a board configuration line:
// Split by commas, trim each token, sort tokens, and rejoin them.
string normalizeBoardLine(const string& line) {
    stringstream ss(line);
    string token;
    vector<string> cells;
    while (getline(ss, token, ',')) {
        string trimmed = trim(token);
        if (!trimmed.empty())
            cells.push_back(trimmed);
    }
    sort(cells.begin(), cells.end());
    string normalized;
    for (size_t i = 0; i < cells.size(); i++) {
        if (i > 0)
            normalized += ",";
        normalized += cells[i];
    }
    return normalized;
}

// Read a file line-by-line (without normalization) into a vector.
vector<string> readLines(const string& filename) {
    vector<string> lines;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: could not open file: " << filename << "\n";
        return lines;
    }
    string line;
    while (getline(file, line)) {
        lines.push_back(trim(line));
    }
    file.close();
    return lines;
}

// Write normalized version of a file to an output file.
void writeNormalizedFile(const string& inputFilename, const string& outputFilename) {
    vector<string> lines = readLines(inputFilename);
    ofstream out(outputFilename);
    if (!out.is_open()) {
        cerr << "Error: could not open output file: " << outputFilename << "\n";
        return;
    }
    for (const auto& line : lines) {
        string norm = normalizeBoardLine(line);
        out << norm << "\n";
    }
    out.close();
}

void compareBoardsAndMoves(const string& desiredFilename,
    const string& actualBoardFilename,
    const string& movesFilename) {

    // First, normalize both files and write them out.
    writeNormalizedFile(desiredFilename, "testNormalized.board");
    writeNormalizedFile(actualBoardFilename, "boardsNormalized.txt");

    // Read normalized files into vectors.
    vector<string> normalizedDesired = readLines("testNormalized.board");
    vector<string> normalizedActual = readLines("boardsNormalized.txt");

    // Also build sets (unique normalized lines) for set operations.
    set<string> desiredSet(normalizedDesired.begin(), normalizedDesired.end());
    set<string> actualSet(normalizedActual.begin(), normalizedActual.end());

    // Compute legal (the intersection)
    set<string> legalSet;
    set_intersection(desiredSet.begin(), desiredSet.end(),
        actualSet.begin(), actualSet.end(),
        inserter(legalSet, legalSet.begin()));

    // Compute missing: desired configurations not in actual.
    set<string> missingSet;
    set_difference(desiredSet.begin(), desiredSet.end(),
        actualSet.begin(), actualSet.end(),
        inserter(missingSet, missingSet.begin()));

    // Compute illegal/extra: configurations in actual not in desired.
    set<string> illegalSet;
    set_difference(actualSet.begin(), actualSet.end(),
        desiredSet.begin(), desiredSet.end(),
        inserter(illegalSet, illegalSet.begin()));

    // Next, count line-by-line matches.
    // For each line in normalizedActual (which came from boardsNormalized.txt),
    // count if it exists in desiredSet.
    int matchCount = 0;
    for (const auto& line : normalizedActual) {
        if (desiredSet.find(line) != desiredSet.end())
            matchCount++;
    }
    // Missing count as per your definition: 
    // (total lines in testNormalized.board) - (count of matching lines from boardsNormalized.txt)
    int missingCount = normalizedDesired.size() - matchCount;

    // Output results.
    cout << "\n=== Legal Board Configurations (unique, normalized) ===\n";
    if (legalSet.empty())
        cout << "None\n";
    else {
        for (const auto& line : legalSet)
            cout << line << "\n";
    }
    cout << "Count of legal (unique normalized) configurations: " << legalSet.size() << "\n";

    cout << "\n=== Count of lines in boardsNormalized.txt that appear in testNormalized.board ===\n";
    cout << matchCount << " out of " << normalizedActual.size() << "\n";

    cout << "\n=== Missing Board Configurations (in desired but not in actual) ===\n";
    if (missingSet.empty())
        cout << "None\n";
    else {
        for (const auto& line : missingSet)
            cout << line << "\n";
    }
    cout << "Missing count (as defined): " << missingCount << "\n";

    cout << "\n=== Illegal Board Configurations (in actual but not in desired) ===\n";
    if (illegalSet.empty())
        cout << "None\n";
    else {
        for (const auto& line : illegalSet)
            cout << line << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <desired_board_file> <actual_board_file> <moves_file>\n";
        return 1;
    }
    string desiredFilename = argv[1];
    string actualFilename = argv[2];
    string movesFilename = argv[3];

    compareBoardsAndMoves(desiredFilename, actualFilename, movesFilename);
    return 0;
}
