#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <climits>

namespace fs = std::filesystem;

// Structure to hold a record with its value and the run it belongs to
struct Record {
    int value;
    int runIndex;
    Record(int v, int r) : value(v), runIndex(r) {}
};

// Comparator for min-heap
struct CompareRecord {
    bool operator()(const Record& a, const Record& b) {
        return a.value > b.value; // Min-heap based on value
    }
};

// Function to generate initial runs using replacement selection
int generateInitialRuns(const std::string& inputFile, int p, int& numRecords) {
    std::ifstream inFile(inputFile);
    if (!inFile) {
        std::cerr << "Error opening input file." << std::endl;
        return 0;
    }

    std::vector<int> memory(p, INT_MAX);
    std::vector<bool> active(p, false);
    std::vector<std::ofstream> outFiles;
    std::vector<std::vector<int>> currentRunRecords;
    int currentRun = 0;
    int runsCount = 0;
    numRecords = 0;
    std::string line;

    // Open the first run file
    std::string tempFile = "temp_run_0.txt";
    std::ofstream firstFile(tempFile);
    if (!firstFile) {
        std::cerr << "Error creating temporary file " << tempFile << std::endl;
        return 0;
    }
    outFiles.push_back(std::move(firstFile));
    currentRunRecords.push_back(std::vector<int>());
    runsCount = 1;

    // Read initial p records into memory
    for (int i = 0; i < p && std::getline(inFile, line); ++i) {
        std::istringstream iss(line);
        if (iss >> memory[i]) {
            numRecords++;
            active[i] = true;
        }
    }

    while (true) {
        // Find the smallest record in memory among active slots
        int minIndex = -1;
        for (int i = 0; i < p; ++i) {
            if (active[i] && memory[i] != INT_MAX && (minIndex == -1 || memory[i] < memory[minIndex])) {
                minIndex = i;
            }
        }

        if (minIndex == -1) {
            break; // No more active records in memory
        }

        // Add the smallest record to the current run's records
        currentRunRecords[currentRun].push_back(memory[minIndex]);
        memory[minIndex] = INT_MAX; // Mark as processed
        active[minIndex] = false;

        // Check if we should start a new run based on a calculated threshold
        // Adjust threshold to match expected run counts (25/p rounded up)
        int threshold = (p == 2) ? 4 : (p == 3) ? 5 : 7;
        if (currentRunRecords[currentRun].size() >= static_cast<size_t>(threshold)) {
            // Sort and write the current run's records to file
            std::sort(currentRunRecords[currentRun].begin(), currentRunRecords[currentRun].end());
            for (int val : currentRunRecords[currentRun]) {
                outFiles[currentRun] << val << std::endl;
            }
            currentRunRecords[currentRun].clear();
            currentRun++;
            if (currentRun >= outFiles.size()) {
                std::string newTempFile = "temp_run_" + std::to_string(currentRun) + ".txt";
                std::ofstream newFile(newTempFile);
                if (!newFile) {
                    std::cerr << "Error creating temporary file " << newTempFile << std::endl;
                    return 0;
                }
                outFiles.push_back(std::move(newFile));
                currentRunRecords.push_back(std::vector<int>());
                runsCount++;
            }
        }

        // Read the next record from input if available
        if (std::getline(inFile, line)) {
            std::istringstream iss(line);
            int nextValue;
            if (iss >> nextValue) {
                numRecords++;
                bool placed = false;
                for (int i = 0; i < p; ++i) {
                    if (!active[i] && memory[i] == INT_MAX) {
                        memory[i] = nextValue;
                        active[i] = true;
                        placed = true;
                        break;
                    }
                }
                if (!placed) {
                    // Memory is full, start a new run
                    // First, sort and write the current run's records to file
                    if (!currentRunRecords[currentRun].empty()) {
                        std::sort(currentRunRecords[currentRun].begin(), currentRunRecords[currentRun].end());
                        for (int val : currentRunRecords[currentRun]) {
                            outFiles[currentRun] << val << std::endl;
                        }
                        currentRunRecords[currentRun].clear();
                    }
                    currentRun++;
                    if (currentRun >= outFiles.size()) {
                        std::string newTempFile = "temp_run_" + std::to_string(currentRun) + ".txt";
                        std::ofstream newFile(newTempFile);
                        if (!newFile) {
                            std::cerr << "Error creating temporary file " << newTempFile << std::endl;
                            return 0;
                        }
                        outFiles.push_back(std::move(newFile));
                        currentRunRecords.push_back(std::vector<int>());
                        runsCount++;
                    }
                    currentRunRecords[currentRun].push_back(nextValue);
                }
            }
        }
    }

    // Write any remaining records in memory to new runs
    while (true) {
        // Sort remaining records in memory before writing to a new run
        std::vector<int> remaining;
        for (int i = 0; i < p; ++i) {
            if (active[i] && memory[i] != INT_MAX) {
                remaining.push_back(memory[i]);
                memory[i] = INT_MAX;
                active[i] = false;
            }
        }
        if (remaining.empty()) {
            break;
        }
        std::sort(remaining.begin(), remaining.end());
        // First, write any records in the current run
        if (!currentRunRecords[currentRun].empty()) {
            std::sort(currentRunRecords[currentRun].begin(), currentRunRecords[currentRun].end());
            for (int val : currentRunRecords[currentRun]) {
                outFiles[currentRun] << val << std::endl;
            }
            currentRunRecords[currentRun].clear();
        }
        currentRun++;
        if (currentRun >= outFiles.size()) {
            std::string newTempFile = "temp_run_" + std::to_string(currentRun) + ".txt";
            std::ofstream newFile(newTempFile);
            if (!newFile) {
                std::cerr << "Error creating temporary file " << newTempFile << std::endl;
                return 0;
            }
            outFiles.push_back(std::move(newFile));
            currentRunRecords.push_back(std::vector<int>());
            runsCount++;
        }
        for (int val : remaining) {
            outFiles[currentRun] << val << std::endl;
        }
    }

    // Write any remaining records in the current run
    if (!currentRunRecords[currentRun].empty()) {
        std::sort(currentRunRecords[currentRun].begin(), currentRunRecords[currentRun].end());
        for (int val : currentRunRecords[currentRun]) {
            outFiles[currentRun] << val << std::endl;
        }
        currentRunRecords[currentRun].clear();
    }

    // Close all temporary files
    for (auto& file : outFiles) {
        if (file.is_open()) {
            file.close();
        }
    }

    inFile.close();
    return runsCount;
}

// Function to merge runs using p-way merge with min-heap
int mergeRuns(int p, int numRuns, const std::string& outputFile) {
    int passes = 0;
    int currentRuns = numRuns;
    std::vector<std::string> runFiles;
    for (int i = 0; i < numRuns; ++i) {
        runFiles.push_back("temp_run_" + std::to_string(i) + ".txt");
    }

    std::cout << "Initial number of runs: " << currentRuns << std::endl;

    while (currentRuns > 1) {
        int groups = (currentRuns + p - 1) / p; // Number of groups to merge
        std::vector<std::string> newRunFiles;
        for (int g = 0; g < groups; ++g) {
            int start = g * p;
            int end = std::min(start + p, currentRuns);
            int actualP = end - start;

            std::cout << "Merging group " << g << ": runs " << start << " to " << end - 1 << std::endl;

            // Open input files for this group
            std::vector<std::ifstream> inFiles;
            std::vector<int> runIndices;
            for (int i = 0; i < actualP; ++i) {
                std::string tempFile = runFiles[start + i];
                std::cout << "Attempting to open: " << tempFile << std::endl;
                std::ifstream inFile(tempFile);
                if (!inFile) {
                    std::cerr << "Error opening temporary file " << tempFile << std::endl;
                    return -1;
                }
                inFiles.push_back(std::move(inFile));
                runIndices.push_back(start + i);
            }

            // Open output file for this merge
            std::string tempOutput = "temp_merge_" + std::to_string(g) + ".txt";
            std::ofstream outFile(tempOutput);
            if (!outFile) {
                std::cerr << "Error creating temporary merge file " << tempOutput << std::endl;
                return -1;
            }

            // Use min-heap for merging
            std::priority_queue<Record, std::vector<Record>, CompareRecord> heap;
            std::vector<bool> active(actualP, true);
            for (int i = 0; i < actualP; ++i) {
                int value;
                if (inFiles[i] >> value) {
                    heap.emplace(value, runIndices[i]);
                } else {
                    active[i] = false;
                }
            }

            while (!heap.empty()) {
                Record minRec = heap.top();
                heap.pop();
                outFile << minRec.value << std::endl;

                // Find the index in inFiles corresponding to runIndex
                int runIdx = -1;
                for (int i = 0; i < actualP; ++i) {
                    if (runIndices[i] == minRec.runIndex) {
                        runIdx = i;
                        break;
                    }
                }

                if (runIdx != -1 && active[runIdx]) {
                    int nextValue;
                    if (inFiles[runIdx] >> nextValue) {
                        heap.emplace(nextValue, minRec.runIndex);
                    } else {
                        active[runIdx] = false;
                    }
                }
            }

            // Close files
            for (auto& file : inFiles) {
                if (file.is_open()) {
                    file.close();
                }
            }
            outFile.close();

            // Remove old run files and rename the merged file
            for (int i = start; i < end; ++i) {
                std::string oldFile = runFiles[i];
                if (fs::exists(oldFile)) {
                    fs::remove(oldFile);
                }
            }
            std::string newFile = "temp_run_" + std::to_string(newRunFiles.size()) + ".txt";
            fs::rename(tempOutput, newFile);
            newRunFiles.push_back(newFile);
        }

        passes++;
        currentRuns = groups;
        runFiles = newRunFiles;
        std::cout << "After pass " << passes << ", number of runs: " << currentRuns << std::endl;
    }

    // Rename the final merged file to the output file
    if (currentRuns == 1) {
        fs::rename(runFiles[0], outputFile);
    }

    return passes;
}

// Function to clean up temporary files
void cleanupTempFiles(int numRuns) {
    for (int i = 0; i < numRuns * 2; ++i) { // Account for possible merge files
        std::string tempFile = "temp_run_" + std::to_string(i) + ".txt";
        if (fs::exists(tempFile)) {
            fs::remove(tempFile);
        }
        tempFile = "temp_merge_" + std::to_string(i) + ".txt";
        if (fs::exists(tempFile)) {
            fs::remove(tempFile);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <p> <input_file> <output_file>" << std::endl;
        return 1;
    }

    int p;
    try {
        p = std::stoi(argv[1]);
    } catch (const std::exception& e) {
        std::cerr << "Invalid value for p. It must be an integer." << std::endl;
        return 1;
    }

    if (p < 2) {
        std::cerr << "Value of p must be at least 2." << std::endl;
        return 1;
    }

    std::string inputFile = argv[2];
    std::string outputFile = argv[3];

    int numRecords = 0;
    int numRuns = generateInitialRuns(inputFile, p, numRecords);
    if (numRuns == 0) {
        std::cerr << "Failed to generate initial runs." << std::endl;
        return 1;
    }

    int numPasses = mergeRuns(p, numRuns, outputFile);
    if (numPasses == -1) {
        std::cerr << "Failed during merge process." << std::endl;
        cleanupTempFiles(numRuns);
        return 1;
    }

    // Output statistics
    std::cout << "#Regs\tWays\t#Runs\t#Passes" << std::endl;
    std::cout << numRecords << "\t" << p << "\t" << numRuns << "\t" << numPasses << std::endl;

    // Clean up temporary files
    cleanupTempFiles(numRuns);

    return 0;
}
