#!/usr/bin/env python3

# This script generates C++ files from HTML/JS files from the input folder.

import os
import sys
import textwrap

def append_gen_file(filepath, content="", init=False):
    if init:
        # Generate the head for file
        head = """\
        // DO NOT EDIT THIS FILE
        // Generated by tools/gen-pages.py

        """
        content = textwrap.dedent(head) + content

    with open(filepath, "a") as file:
        file.write(content)

def gen_page_setup_function(dictionary_header_filename, pages_header_filename, filename, counter):
    # Generate the variable name from the filename
    info = os.path.splitext(filename)
    variable_basename = info[0].replace("-", "_")
    ext = info[1][1:]

    # Read the contents of the HTML file
    with open(os.path.join(input_folder, filename), "r") as file:
        file_content = file.read()

    dictionary_entry = f"RMS_PAGE_{variable_basename.upper()}_{ext.upper()}"

    # Append an incremental #define in _dictionary.h named from the variable in UPPERCASE
    append_gen_file(dictionary_header_filename, f"#define {dictionary_entry} {counter}\n")
    counter += 1

    append_gen_file(pages_header_filename, f"    // Generated from {filename}\n")
    append_gen_file(pages_header_filename, f"    R\"=====(\n{file_content}\n)=====\",\n")
    return counter

def generate_files(input_folder, output_folder):
    # Remove all .h and .cpp files from the output folder
    for filename in os.listdir(output_folder):
        if filename.endswith(".h") or filename.endswith(".cpp"):
            file_path = os.path.join(output_folder, filename)
            os.remove(file_path)

    dictionary_header_filename = os.path.join(output_folder, "_dictionary.h")
    pages_header_filename = os.path.join(output_folder, "_pages.h")

    append_gen_file(dictionary_header_filename, "#pragma once\n\n", init=True);

    content = """\
    #pragma once

    #include "_dictionary.h"

    const char* pages[] =
    {
    """

    append_gen_file(pages_header_filename, textwrap.dedent(content), init=True);

    counter = 0
    # Loop through the input folder
    for filename in os.listdir(input_folder):
        if filename.endswith(".html") or filename.endswith(".js"):
            counter = gen_page_setup_function(dictionary_header_filename, pages_header_filename, filename, counter)

    content = """\
        "" // dummy content to avoid trailing comma
    };

    """
    append_gen_file(pages_header_filename, textwrap.dedent(content));

# Check if the correct number of command line arguments is provided
if len(sys.argv) != 3:
    # Display help message
    print("Usage: python gen-pages.py <input_folder> <output_folder>")
    sys.exit(1)

# Read the input and output folder paths from command args
input_folder = sys.argv[1]
output_folder = sys.argv[2]

# Call the function to generate the header files
generate_files(input_folder, output_folder)
