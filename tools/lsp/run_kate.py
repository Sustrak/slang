#!/bin/python3

import os
import subprocess
import time
import argparse

SV_FILE = "/home/jsans/git/avispado/rtl/common_cells/compr42.sv"
qt_logging_rules = "katelspclientplugin=true"


def launch_kate(gdb):
    if gdb:
        # Launch kate with specified environment variable
        subprocess.Popen(["kate", "-b", SV_FILE],
                         env=dict(os.environ, QT_LOGGING_RULES=qt_logging_rules, DEBUG_GDB="ON"))
        # Sleep to ensure kate starts up properly
        time.sleep(1)
    else:
        # Launch kate without gdb (run in the foreground)
        subprocess.run(["kate", "-b", SV_FILE], env=dict(os.environ, QT_LOGGING_RULES=qt_logging_rules))

    # Find the PID of kate
    try:
        kate_pid = int(subprocess.check_output(["pgrep", "-f", "kate"]).splitlines()[1])
    except:
        kate_pid = None

    # Find the PID of slang-lsp
    try:
        slang_lsp_pid = int(subprocess.check_output(["pgrep", "-f", "slang-lsp"]).splitlines()[0])
    except:
        slang_lsp_pid = None

    if kate_pid:
        print(f"KATE_PID: {kate_pid}")
    if slang_lsp_pid:
        print(f"SLANG_LSP_PID: {slang_lsp_pid}")

    return kate_pid, slang_lsp_pid


def launch_gdb_and_debug(kate_pid, slang_lsp_pid):
    # Attach gdb to slang-lsp and set the '_done' variable to 1
    if slang_lsp_pid:
        subprocess.run(["sudo", "gdb", "-q", "-p", str(slang_lsp_pid), "-ex", "up 3", "-ex", "set var _done = 1", "-ex",
                        "b startServer()", "-ex", "source /home/jsans/.gdbinit"])


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Python script to replicate the bash script.")
    parser.add_argument("--gdb", action="store_true", help="Enable gdb debugging")
    args = parser.parse_args()

    kate_pid, slang_lsp_pid = launch_kate(args.gdb)

    if args.gdb:
        launch_gdb_and_debug(kate_pid, slang_lsp_pid)

    # Terminate kate and slang-lsp
    if kate_pid:
        subprocess.run(["kill", "-9", str(kate_pid)], check=False)
    if slang_lsp_pid:
        subprocess.run(["kill", "-9", str(slang_lsp_pid)], check=False)

    print("===== KATE AND SLANG LSP TERMINATED =====")
