#!/usr/bin/env python3
"""The revision a ratchet is compared with, for the checks whose lists only descend.

`check-comment-language.py` (`.french-allowlist`) and `check-translatable-strings.py`
(`.untranslated-ratchet`) both refuse a list that grew since the merge base with `UPSTREAM`,
and both carried the same way of finding it. It is written here once.

    git         runs git in a repository, never raising: the caller reads `returncode`
    merge_base  the revision named by `--base`, otherwise the merge base of HEAD with
                `UPSTREAM`; or None and why there is none — a shallow clone, an archive,
                a repository without `origin/main`

A module beside the scripts rather than under `lib/`, as `doc_vocabulary.py` is: a directory
without `__init__.py` is a namespace package, which any `lib` package installed further down
`sys.path` would shadow.
"""

from __future__ import annotations

import pathlib
import subprocess
import sys

UPSTREAM = "origin/main"


def git(root: pathlib.Path, *arguments: str) -> subprocess.CompletedProcess:
    return subprocess.run(["git", "-C", str(root), *arguments], capture_output=True, text=True,
                          check=False)


def merge_base(root: pathlib.Path, base: str | None) -> tuple[str | None, str]:
    """The revision a run is compared with, or None and why there is none."""
    if base is not None:
        if git(root, "rev-parse", "--verify", "--quiet", f"{base}^{{commit}}").returncode != 0:
            return None, f"{base} is not a revision of this repository"
        return base, base
    if git(root, "rev-parse", "--verify", "--quiet", f"{UPSTREAM}^{{commit}}").returncode != 0:
        return None, f"{UPSTREAM} is not reachable"
    found = git(root, "merge-base", "HEAD", UPSTREAM)
    if found.returncode != 0:
        return None, f"HEAD and {UPSTREAM} have no merge base (shallow clone?)"
    revision = found.stdout.strip()
    return revision, f"the merge base with {UPSTREAM} ({revision[:10]})"


if __name__ == "__main__":
    sys.exit("this module is imported, not run: see check-comment-language.py")
