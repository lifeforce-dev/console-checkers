import json
from pathlib import Path

BASE_PATH = Path(__file__).parent.parent
GAME_STRINGS_PATH = BASE_PATH / 'resources' / 'game-text.json'
SOURCE_PATH = BASE_PATH / 'Source/console-checkers/Source'
HEADER_OUTPUT_PATH = SOURCE_PATH / 'UITextStrings.h'

def load_ui_texts(path):
    """Load UI texts from a JSON file."""
    try:
        with path.open('r', encoding='utf8') as file:
            return json.load(file)
    except json.JSONDecodeError:
        print("Failed to parse the JSON file. Please check its format.")
        exit(1)

def to_camel_case(name):
    """Convert UPPER_CASE to s_camelCase"""
    components = name.lower().split('_')
    return 's_' + components[0] + ''.join(x.capitalize() for x in components[1:])

def generate_header_content(ui_texts):
    """Generate the content for the header file from UI texts using tabs for indentation."""
    header_content = '''//---------------------------------------------------------------
//
// UITextStrings.h
//

///////////////////////////////////////////////////////
//
// THIS FILE IS GENERATED!! CHANGES HERE WILL BE LOST!
//
///////////////////////////////////////////////////////

#pragma once

#include <string_view>

namespace Checkers {
\tnamespace UIText {

'''
    for key, entry in ui_texts.items():
        try:
            text = entry["text"]
            comment = entry.get("comment")
            if comment is None:
                raise ValueError(f"Missing comment for {key}")
            variable_name = to_camel_case(key)
            header_content += f'\t\t// {comment}\n\t\tstatic constexpr std::string_view {variable_name} = "{text}";\n\n'
        except KeyError as e:
            print(f"Error in JSON structure: missing key {e} for item '{key}'.")
            exit(1)

    header_content += '''\t}
}
'''
    return header_content

def write_header_file(path, content):
    """Write the generated header content to a file."""
    try:
        with path.open('w', encoding='utf8') as file:
            file.write(content)
        print("Header file 'GameStrings.h' generated successfully.")
    except Exception as e:
        print(f"Failed to write the header file: {e}")
        exit(1)

def main():
    ui_texts = load_ui_texts(GAME_STRINGS_PATH)
    header_content = generate_header_content(ui_texts)
    write_header_file(HEADER_OUTPUT_PATH, header_content)

if __name__ == "__main__":
    main()