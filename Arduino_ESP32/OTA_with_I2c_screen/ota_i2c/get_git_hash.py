import subprocess
import os


def get_git_hash():
    try:
        # Check if repository is dirty
        dirty_check = subprocess.run(
            ["git", "diff", "--quiet"], capture_output=True
        )
        is_dirty = dirty_check.returncode == 1

        # Get the short git commit hash
        result = subprocess.run(
            ["git", "rev-parse", "--short", "HEAD"], capture_output=True, text=True
        )
        if result.returncode == 0:
            git_hash = result.stdout.strip()
            return f"[dirty]{git_hash}" if is_dirty else git_hash
    except Exception as e:
        print(f"Error getting git hash: {e}")
    return "unknown"


def generate_version_header(git_hash):
    header_content = f'''#ifndef GIT_VERSION_H
#define GIT_VERSION_H

#define FIRMWARE_GIT_COMMIT_HASH "{git_hash}"

#endif // GIT_VERSION_H
'''
    with open("src/git_version.h", "w") as f:
        f.write(header_content)

git_hash = get_git_hash()
print(f"current git hash: {git_hash}")
generate_version_header(git_hash)
