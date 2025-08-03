PAGE_SIZE = 65536  # Size of one page in bytes
NUM_PAGES_IN_FILENAME = 2  # Number of pages in filename
CHAR_TO_WRITE = 'A'  # Character to write

# Generate the filename with 5 pages
filename = CHAR_TO_WRITE * (NUM_PAGES_IN_FILENAME * PAGE_SIZE)
filename += ".txt"  # Add file extension

# Open the file
with open(filename, 'w') as outFile:
    # Write the content for 5 pages
    # Generate the content for one page
    pageContent1 = 'a' * PAGE_SIZE
    pageContent2 = 'b' * PAGE_SIZE
    pageContent3 = 'c' * PAGE_SIZE
    pageContent4 = 'd' * PAGE_SIZE
    pageContent5 = 'e' * PAGE_SIZE

    outFile.write(pageContent1 + pageContent2 + pageContent3 + pageContent4 + pageContent5)

print("File", filename, "generated successfully.")
