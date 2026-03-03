import os
import sys

def generate_songs_h(folder_name):
    # Get all _arduino.h files in the folder
    files = sorted([
        f for f in os.listdir(folder_name)
        if f.endswith("_arduino.h")
    ])

    if not files:
        print(f"No *_arduino.h files found in '{folder_name}'")
        return

    includes = []
    entries = []

    for filename in files:
        # Strip _arduino.h to get the base prefix (e.g. "dream_land")
        prefix = filename[: -len("_arduino.h")]

        # Build display name: underscores → spaces, title case
        display_name = prefix.replace("_", " ").title()

        includes.append(f'#include "{folder_name}/{filename}"')
        entries.append(
            f'  {{ {prefix}_notes,    {prefix}_dur,    {prefix}_length, "{display_name}"    }},'
        )

    # Remove trailing comma from last entry
    entries[-1] = entries[-1].rstrip(",")

    output_lines = (
        includes
        + ["Song songs[] = {"]
        + entries
        + ["};", ""]
    )

    output_path = "songs.h"
    with open(output_path, "w") as f:
        f.write("\n".join(output_lines))

    print(f"Generated '{output_path}' with {len(files)} song(s).")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python generate_songs_h.py <folder_name>")
        sys.exit(1)

    generate_songs_h(sys.argv[1])
