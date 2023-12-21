#!/usr/bin/python3

import json
import sys

def format_json(json_str):
    try:
        # Remove '\n' characters from the input JSON string
        json_str = json_str.replace('\\n', '')

        # Parse the input JSON string
        data = json.loads(json_str)

        # Dump the formatted JSON string
        formatted_json = json.dumps(data, indent=4)

        # Print the formatted JSON
        print(formatted_json)

    except json.JSONDecodeError as e:
        print("Error parsing JSON:", e)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py 'json_string'")
        sys.exit(1)

    # Get the JSON input from script arguments
    input_json = sys.argv[1]

    # Call the function to format and print the JSON
    format_json(input_json)
