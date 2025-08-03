#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fstream>
#include "vm_app.h"
#include <string>

using std::cout;

int main()
{
    // Specify the filenames
    std::string sourceFilename = "data3.bin";
    std::string destinationFilename = "test001.cpp";

    // Open the source file for reading
    std::ifstream sourceFile(sourceFilename);
    if (!sourceFile.is_open()) {
        std::cerr << "Error: Unable to open source file " << sourceFilename << std::endl;
        return 1;
    }

    // Open the destination file for writing
    std::ofstream destinationFile(destinationFilename);
    if (!destinationFile.is_open()) {
        std::cerr << "Error: Unable to open destination file " << destinationFilename << std::endl;
        sourceFile.close(); // Close the source file
        return 1;
    }

    // Copy content from source file to destination file
    std::string line;
    while (std::getline(sourceFile, line)) {
        destinationFile << line << std::endl;
    }

    // Close the files
    sourceFile.close();
    destinationFile.close();

    std::cout << "Content copied from " << sourceFilename << " to " << destinationFilename << " successfully." << std::endl;

    // Start of actual code
    
    char *p0 = (char *)vm_map(nullptr, 0);
    p0[0] = 'H';
    printf("%c\n", p0[0]);

    strcpy(p0, "test001.cpp");
    printf("Where to find King of 482: %s\n", p0);

    char *p1 = (char *)vm_map(p0, 0);

    p1[9] = '\0';
    for (size_t i=0; i<9; ++i) {
        static char* tmp = p1;
        printf("%c", *tmp++);
    }
    printf("\np1 (file content): %s\n", p1);

    std::ifstream inputFile(destinationFilename);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Unable to open file " << destinationFilename << " for reading." << std::endl;
        return 1;
    }
    // Read existing content of the file
    std::string existingContent;
    while (std::getline(inputFile, line)) {
        existingContent += line + "\n";
    }

    // Close the input file
    inputFile.close();

    // Open the file in write mode to overwrite its content
    std::ofstream outputFile(destinationFilename);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Unable to open file " << destinationFilename << " for writing." << std::endl;
        return 1;
    }

    // Write smiley face at the beginning of the file
    outputFile << ":-)\n";

    // Write the existing content after the smiley face
    outputFile << existingContent;

    // Close the output file
    outputFile.close();

    std::cout << "Smiley face added at the beginning of the file " << destinationFilename << " successfully." << std::endl;

    char *p3 = (char *)vm_map(nullptr, 0);
    p3[0] = 'S';
    char *p4 = (char *)vm_map(nullptr, 0);
    p4[0] = 'L';
    char *p5 = (char *)vm_map(nullptr, 0);
    p5[0] = 'A';
    char *p6 = (char *)vm_map(nullptr, 0);
    p6[0] = 'V';
    char *p7 = (char *)vm_map(nullptr, 0);
    p7[0] = 'I';
    char *p8 = (char *)vm_map(nullptr, 0);
    p8[0] = 'A';
    char *p9 = (char *)vm_map(nullptr, 0);
    p9[0] = 'R';
    char *p10 = (char *)vm_map(nullptr, 0);
    p10[0] = 'O';
    char *p11 = (char *)vm_map(nullptr, 0);
    p11[0] = '!';

    printf("%c%c%c%c%c%c%c%c%c\n", p7[0], p8[0], p9[0], p10[0], p3[0], p4[0], p5[0], p6[0], p11[0]);
    // Now p1 should have been evicted
    printf("Amazing!\n");

    char *p2 = (char *)vm_map(p0, 0);
    for (size_t i=0; i<9; ++i) {
        static char* tmp = p2;
        printf("%c", *tmp++);
    }
}