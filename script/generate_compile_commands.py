#!/bin/python

import sys
import os

import json

if __name__ == "__main__":
    build_dir = os.path.normpath(sys.argv[1])
    compile_commands_path = os.path.join(build_dir, "compile_commands.json")

    print(f'generating "{compile_commands_path}"...')

    intermediate_dirs: list[str] = []

    # collect intermediate build dirs
    for subdir, dirs, fils in os.walk(build_dir):
        if subdir.endswith(".dir"):
            print(f"intermediate directory found: {subdir}")
            intermediate_dirs.append(subdir)
            dirs.clear()  # do not recurse anymore

    command_logfiles: list[str] = []

    # collect CL.command.X.tlog files
    for intdir in intermediate_dirs:
        for subdir, dirs, files in os.walk(intdir):
            if not subdir.endswith(".tlog"):
                continue
            for file in files:
                if file.startswith("CL.command"):
                    command_logfiles.append(os.path.join(subdir, file))
            dirs.clear()  # should not be any directories under subdir, but can't hurt clearing

    compile_commands: list[dict[str, str]] = []

    for logfile in command_logfiles:
        with open(logfile, encoding="utf-16") as file:
            print(f'processing "{logfile}"...')
            while line := file.readline():
                line = line[1:].rstrip()
                command = {
                    "directory": os.path.dirname(line),
                    "file": os.path.basename(line),
                }
                line = file.readline()
                if line:
                    command["command"] = "cl.exe " + line.rstrip()
                else:
                    print(f'failed processing "{logfile}": missing command line')
                    exit(1)

                compile_commands.append(command)

    with open(compile_commands_path, "w") as file:
        print(f'writing "{compile_commands_path}"...')
        json.dump(compile_commands, file, indent=2)

    print("compile_commands.json generation done")
