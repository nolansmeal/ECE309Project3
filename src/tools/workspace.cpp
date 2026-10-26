// src/tools/workspace.cpp — YOU IMPLEMENT (spec §4.4).
//
// Workspace::resolve: a purely lexical check.
// Do not touch the filesystem here.

#include "tools/workspace.h"

namespace fs = std::filesystem;

// Converts a user-provided path into a safe workspace path
bool Workspace::resolve(std::string_view user_path, fs::path* out,
                        std::string* reason) const {

    // Reject empty paths
    if (user_path.empty()) {
        *reason = "Path cannot be empty";
        return false;
    }

    // Reject paths containing null characters
    if (user_path.find('\0') != std::string_view::npos) {
        *reason = "Path contains a null character";
        return false;
    }

    // Convert the input string into a filesystem path
    fs::path path{std::string(user_path)};

    // Reject absolute paths or paths with a root component
    if (path.has_root_path()) {
        *reason = "Absolute paths are not allowed";
        return false;
    }

    // Keep track of how many directories deep we are
    int depth = 0;

    // Examine each component of the path
    for (const auto& part : path) {

        // Ignore the current-directory component
        if (part == ".") {
            continue;
        }

        // Move up one directory
        if (part == "..") {
            depth--;

            // Reject paths that escape the workspace root
            if (depth < 0) {
                *reason = "Path escapes the workspace";
                return false;
            }
        }
        else if (!part.empty()) {
            // Move down one directory
            depth++;
        }
    }

    // Remove unnecessary "." and ".." components
    fs::path normalized = path.lexically_normal();

    // Combine the normalized path with the workspace root
    *out = root() / normalized;

    // Clear any previous error message
    reason->clear();

    return true;
}
