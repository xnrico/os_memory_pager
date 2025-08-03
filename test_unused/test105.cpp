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

    // Specify the filename
    std::string filename = std::string(p0);
    printf("Now clearing the file: %s\n", filename.c_str());
    std::ofstream outFile(filename);

    if (outFile.is_open()) {
        outFile.close();
        std::cout << "Text cleared from the file: " << filename << std::endl;
    } else {
        std::cout << "Error: Unable to clear text from the file " << filename << std::endl;
    }

    // Open the file in append mode
    outFile.open(filename, std::ios::app);

    // Write a smiley face to the file
    if (outFile.is_open()) {
        outFile << ":-)" << std::endl;
        outFile.close();
        std::cout << "Smiley face written to the file." << std::endl;
    } else {
        std::cerr << "Error: Unable to open file for writing smiley face." << std::endl;
        return 1;
    }

    // Print the content of the file
    std::ifstream inFile(filename);
    if (inFile.is_open()) {
        std::cout << "Content of the file:" << std::endl;
        std::string line;
        while (std::getline(inFile, line)) {
            std::cout << line << std::endl;
        }
        inFile.close();
    } else {
        std::cerr << "Error: Unable to open file for reading." << std::endl;
        return 1;
    }


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

    // Print the content of the file
    if (inFile.is_open()) {
        std::cout << "Content of the file (Should not be empty now):" << std::endl;
        std::string line;
        while (std::getline(inFile, line)) {
            std::cout << line << std::endl;
        }
        inFile.close();
    } else {
        std::cerr << "Error: Unable to open file for reading." << std::endl;
        return 1;
    }

    char *p2 = (char *)vm_map(p0, 0);
    for (size_t i=0; i<9; ++i) {
        static char* tmp = p2;
        printf("%c", *tmp++);
    }
}