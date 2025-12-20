import os
import subprocess
Import("env")

def run_cleanup(*args, **kwargs):
    print("--- Starting Code Cleanup ---")
    
    # 1. Run clang-format (Style)
    print("Running clang-format...")
    # -i: In-place, --style=file: use .clang-format
    subprocess.run(["clang-format", "-i", "--style=file", "src/*.cpp", "include/*.h"], shell=True)

    # 2. Run clang-tidy (Fix Yoda Conditions)
    print("Running clang-tidy fixes...")
    # -fix: apply changes, -checks: only look for Yoda issues
    subprocess.run([
        "clang-tidy", 
        "-fix", 
        "-checks=-*,readability-yoda-condition", 
        "src/*.cpp", 
        "--", 
        "-Iinclude"
    ], shell=True)
    
    print("--- Cleanup Complete ---")

# Add the button to the PlatformIO sidebar
env.AddCustomTarget(
    name="cleanup",
    dependencies=None,
    actions=[run_cleanup],
    title="Clean & Un-Yoda",
    description="Formats code and fixes Yoda conditions"
)