import subprocess
import os


def get_git_hash():
    try:
        # Get the short git commit hash
        result = subprocess.run(
            ["git", "rev-parse", "--short", "HEAD"], capture_output=True, text=True
        )
        if result.returncode == 0:
            return result.stdout.strip()
    except Exception as e:
        print(f"Error getting git hash: {e}")
    return "unknown"


if __name__ == "__main__":
    git_hash = get_git_hash()
    print(f"Current git hash: {git_hash}")
    # Set the environment variable for PlatformIO
    os.environ["GIT_COMMIT_HASH"] = git_hash
else:
    # This part is for PlatformIO
    Import("env")
    git_hash = get_git_hash()
    print(f"Current git hash: {git_hash}")
    env.Append(CPPDEFINES=[("GIT_COMMIT_HASH", f'\\"{git_hash}\\"')])
