#pragma once

namespace utils {
    // Read an entire file into a buffer, and return said buffer
    // NOTE: the caller is responsible for freeing the returned buffer once they're doing using it
    char *readFile(char* filePath);
} // namespace utils
