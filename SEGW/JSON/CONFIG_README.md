# Search Engine Configuration Guide

## Overview
This guide explains how to configure the Search Engine application using the `config.json` file.

## Configuration File Structure

### Main Configuration Section
The `config` section contains all the application settings:

```json
{
  "config": {
    "name": "Search Engine",
    "version": "1.0",
    "max_responses": 5,
    "thread_pool_size": 4,
    "max_file_size_mb": 10,
    "supported_extensions": [".txt", ".md"],
    "log_level": "INFO",
    "auto_discover_files": true,
    "max_files_to_process": 5,
    "resources_directory": "resources"
  }
}
```

## Field Descriptions

### Required Fields
- **`name`** (string): Application name - MUST be present and non-empty

### Optional Fields
- **`version`** (string): Application version (default: "0.1")
- **`max_responses`** (number): Maximum search results per query (default: 5, minimum: 1)
- **`thread_pool_size`** (number): Number of processing threads (default: 4)
- **`max_file_size_mb`** (number): Maximum file size in MB (default: 10)
- **`supported_extensions`** (array): File extensions to process (default: [".txt", ".md"])
- **`log_level`** (string): Logging verbosity - DEBUG, INFO, WARNING, ERROR (default: "INFO")
- **`auto_discover_files`** (boolean): Automatically find files in directory (default: false)
- **`max_files_to_process`** (number): Maximum files when auto-discovery enabled (default: 10)
- **`resources_directory`** (string): Directory name for file search (default: "resources")

## Configuration Modes

### 1. Auto-Discovery Mode
Automatically finds and processes files from the resources directory:

```json
{
  "config": {
    "name": "Search Engine",
    "auto_discover_files": true,
    "max_files_to_process": 10,
    "resources_directory": "resources"
  }
}
```

**Features:**
- Automatically finds all supported files in the specified directory
- Processes files in alphabetical order
- Respects the `max_files_to_process` limit
- No need to manually specify file paths

### 2. Manual Mode
Manually specify which files to process:

```json
{
  "config": {
    "name": "Search Engine",
    "auto_discover_files": false
  },
  "files": [
    "resources/file001.txt",
    "resources/file002.txt",
    "resources/special_document.md"
  ]
}
```

**Features:**
- Complete control over which files are processed
- Can include files from different directories
- Must ensure all specified files exist

## Usage Examples

### Basic Configuration
```json
{
  "config": {
    "name": "My Search Engine",
    "auto_discover_files": true,
    "max_files_to_process": 5
  }
}
```

### High-Performance Configuration
```json
{
  "config": {
    "name": "Search Engine",
    "max_responses": 20,
    "thread_pool_size": 8,
    "max_file_size_mb": 50,
    "auto_discover_files": true,
    "max_files_to_process": 100,
    "log_level": "WARNING"
  }
}
```

### Testing Configuration
```json
{
  "config": {
    "name": "Search Engine",
    "max_responses": 3,
    "auto_discover_files": true,
    "max_files_to_process": 2
  }
}
```

## Troubleshooting

### Common Issues

**No files found:**
- Check if `resources_directory` exists
- Ensure directory contains files with supported extensions
- Verify file permissions

**Files not loading:**
- Check file paths are correct
- Ensure files are readable
- Verify file extensions are supported

**Too many files:**
- Reduce `max_files_to_process`
- Use manual mode to specify exact files
- Check if files are being duplicated

**Performance issues:**
- Increase `thread_pool_size`
- Reduce `max_files_to_process`
- Set `log_level` to "WARNING" or "ERROR"

## File Processing Order

When using auto-discovery mode, files are processed in alphabetical order by filename. This ensures consistent results across runs.

## Supported File Types

Currently supported file extensions:
- `.txt` - Plain text files
- `.md` - Markdown files

## Configuration Validation

The application will validate the configuration on startup and report any issues:
- Missing required fields
- Invalid values
- File access problems
- Directory existence

## Examples File

See `config_examples.json` for more detailed configuration examples and use cases.
