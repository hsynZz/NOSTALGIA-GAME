"""Run all checks available without Unreal. Never labels these as an engine build."""
import argparse
import datetime
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]


def run():
    parser = argparse.ArgumentParser()
    parser.add_argument("--sanitize", action="store_true", help="Enable GCC/Clang address and undefined behavior sanitizers")
    args = parser.parse_args()
    os.chdir(ROOT)
    output = ROOT / "Artifacts/Validation"
    output.mkdir(parents=True, exist_ok=True)
    report = {"started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
              "unreal_build": "not_run", "unreal_automation": "not_run", "human_playtest": "not_run",
              "sanitizers": args.sanitize}
    result_code = 1
    try:
        suite = unittest.defaultTestLoader.discover(str(ROOT / "Tests"), pattern="test_*.py")
        result = unittest.TextTestRunner(verbosity=2).run(suite)
        report["python"] = {"run": result.testsRun, "failed": len(result.failures), "errors": len(result.errors), "skipped": len(result.skipped)}
        compiler = os.environ.get("CXX") or shutil.which("g++") or shutil.which("clang++")
        if not compiler:
            raise RuntimeError("C++20 compiler missing. Install GCC/Clang or set CXX to its executable.")
        report["compiler"] = subprocess.run([compiler, "--version"], capture_output=True, text=True, check=True).stdout.splitlines()[0]
        executable = ROOT / "Tests/.build" / ("core_tests.exe" if os.name == "nt" else "core_tests")
        executable.parent.mkdir(parents=True, exist_ok=True)
        command = [compiler, "-std=c++20", "-Wall", "-Wextra", "-Werror", "-Wpedantic", "-O1", "-g",
                   "-I", str(ROOT / "Source/Nostalgia/Public"), str(ROOT / "Tests/core_tests.cpp"),
                   str(ROOT / "Source/Nostalgia/Private/Core/NostalgiaCore.cpp"), "-o", str(executable)]
        if args.sanitize:
            command += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
        subprocess.run(command, check=True)
        process = subprocess.run([str(executable)], capture_output=True, text=True)
        (output / "core.log").write_text(process.stdout + process.stderr, encoding="utf-8")
        print(process.stdout, end="")
        print(process.stderr, end="", file=sys.stderr)
        match = re.search(r"CORE_RESULT passed=(\d+) failed=(\d+) total=(\d+)", process.stdout)
        if not match or int(match[3]) == 0:
            raise RuntimeError("Core runner did not complete a nonzero number of tests.")
        report["core"] = dict(zip(("passed", "failed", "total"), map(int, match.groups())))
        result_code = 0 if process.returncode == 0 and result.wasSuccessful() and result.testsRun > 0 else 1
    except (OSError, RuntimeError, subprocess.SubprocessError) as error:
        report["error"] = str(error)
        print(error, file=sys.stderr)
    report["result"] = "passed" if result_code == 0 else "failed"
    report["completed_utc"] = datetime.datetime.now(datetime.timezone.utc).isoformat()
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"Offline report: {output / 'results.json'}; Unreal execution: NOT RUN")
    return result_code


if __name__ == "__main__":
    sys.exit(run())
