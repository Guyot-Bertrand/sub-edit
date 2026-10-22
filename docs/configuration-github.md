# GitHub repository configuration

The `subedit` repository is **public**. This document lists the configuration to
apply in the GitHub interface — it does not live in the code and would otherwise
be lost.

## What a public repository does not allow

To be stated up front so as to avoid false expectations: on a public repository,
GitHub **offers no way to prevent forks or the creation of issues**. What can be
controlled is what lands in the repository, and there the lockdown is complete.

If one day the requirement becomes "no outside interaction possible", the only
complete answer is a private repository. The GPL-3.0 imposes nothing as long as
the binary is not distributed.

## Applying it

`src/scripts/setup-github.sh` applies everything that has an API — labels,
milestones, ruleset — idempotently. The rest has none and is listed further
down.

```bash
./src/scripts/setup-github.sh
```

The ruleset requires the **Administration: Read and write** permission on the
token; a fine-grained token does not grant it by default. Without it, the script
reports the failure, applies the rest and exits normally.

## Configuration to apply

### 1. No collaborators

*Settings → Collaborators and teams* — add nobody.

This is the measure that does most of the work: without write access, a third
party can only fork and propose a PR, which is closed in one click.

### 2. A ruleset on `main`

*Settings → Rules → Rulesets*, or `src/scripts/setup-github.sh`.

The labels below are those of the GitHub interface, not those of the API.

#### The `protection de l'historique` ruleset

| Interface field | Value |
| :-------------- | :---- |
| Ruleset Name | `protection de l'historique` |
| Enforcement status | **Active** |
| Bypass list | *leave empty* |
| Target branches → Add target | **Include default branch** |
| Rules | tick **Restrict deletions** and **Block force pushes** |

Since the owner is the only one who can write, the real risk is not a malicious
third party: it is an accidental `push --force` on their own part. A rule they
were exempt from would therefore protect against nothing — hence the empty
bypass list.

#### There is no longer a quality gate ruleset

**Removed in #232.** This second ruleset required the `porte de qualité` check
of `.github/workflows/ci.yml`. Actions minutes and cache storage were running
out too fast for one run per pull request: `ci.yml` and `pull-request.yml` now
run on demand only, and a rule that waits for a check nothing will produce
protects against nothing — it only teaches one to get past it through the admin
bypass.

It had been set to `disabled` on 2026-08-27, when the August quota ran out.
`setup-github.sh` no longer creates it. If it still exists on the repository, it
is deleted through *Settings → Rules → Rulesets*, or:

```console
$ gh api -X DELETE repos/Guyot-Bertrand/sub-edit/rulesets/20469201
```

**What guards the repository is `make check`, run locally**, and the pull
request states what was passed. What runs on GitHub for each version is the
publication of the packages — see section 10.

#### Checking

```bash
gh api repos/Guyot-Bertrand/sub-edit/rulesets --jq '.[] | "\(.id) \(.name) [\(.enforcement)]"'
gh api repos/Guyot-Bertrand/sub-edit/rulesets/<id> \
  --jq '{bypass: [.bypass_actors[]? | "\(.actor_type)/\(.actor_id)"], rules: [.rules[].type]}'
```

Expected: `protection de l'historique` alone, with no bypass, with the
`deletion` and `non_fast_forward` rules.

### 3. Approval of fork workflows

*Settings → Actions → General → Fork pull request workflows from outside collaborators*
→ **Require approval for all external contributors**

Prevents a PR from a stranger from triggering the CI, and thus consuming minutes
or running unreviewed code.

### 4. Interaction limits

*Settings → Moderation → Interaction limits* → **Limit to prior contributors**

Renewable in six-month periods. It is the closest thing to "nobody else steps
in" on a public repository. To be renewed explicitly: GitHub lets the limit
expire without warning.

### 5. Reduce the public surface

*Settings → General → Features* — disable **Wiki**, **Discussions** and
**Projects** as long as they are not used.

### 6. Code ownership and contribution

- `.github/CODEOWNERS` naming the owner on `*`.
- `CONTRIBUTING.md` stating that the project does not accept outside
  contributions for now. It is a courtesy as much as a protection: it spares a
  third party from working for nothing.

### 7. Log and artifact retention

*Settings → Actions → General → Artifact and log retention* → **7 days**
(adjustable from 1 to 90; 90 by default, but this repository was **already at
30** — measured, not assumed, from the gap between the artifacts' `created_at`
and `expires_at`).

Together with the cache, it is the only billed storage item of the repository.
A single artifact is uploaded — `couverture`, the HTML report produced by
`ci.yml` on each run of the gate, invariably 321 KiB. **Since #232, `ci.yml`
runs on demand only**, so the calculation that follows describes the former
regime; it remains the reason for the setting. At a dozen runs a day, the
plateau is about 103 MiB at 30 days and **~24 MiB at 7 days**, out of the
500 MiB of a free account. The 105 artifacts present weighed only 23.4 MiB
because only ten days of accumulation separated them from the start: the plateau
had not been reached.

The setting cannot be scripted: the Actions API answers
`403 Resource not accessible by personal access token`. It is therefore set by
hand, like the six previous ones.

**It does not do the pruner's job, and vice versa.** The retention period erases
the logs and artifacts; it leaves the run in the list, with its steps empty.
That is why both are needed — one bounds the storage, the other the count.

#### Checking

The setting cannot be read through the API without an administration token. What
can be read, and is enough, is the gap carried by the artifacts themselves:

```bash
gh api repos/Guyot-Bertrand/sub-edit/actions/artifacts --paginate \
  --jq '.artifacts[] | "\(.created_at)\t\(.expires_at)"' |
  while IFS=$'\t' read -r c e; do
      echo $(( ( $(date -u -d "$e" +%s) - $(date -u -d "$c" +%s) ) / 86400 ))
  done | sort -u
```

Expected: `6`, since the expiry is computed from the start of the run and not
from the upload. Before the setting was applied, the same command answered
`29`.

The setting applies only to artifacts created after it: earlier ones keep the
expiry date they already carried. It is the pruner that removes them, by
deleting their run.

### 8. Pruning runs

`.github/workflows/elagage.yml`, every Monday at 6:00, and by hand through
*Actions → pruning → Run workflow*.

**The rule:** for each workflow, keep the thirty most recent runs, plus the most
recent one whose branch is `main` if it is not already among them; delete the
rest; never touch a run that has not finished.

Unlike the seven previous settings, this one lives in the repository and not in
the interface — and it had no choice. A personal token cannot delete a run;
only a workflow's `GITHUB_TOKEN` can, with `permissions: actions: write`. The
rule itself is in `src/scripts/prune-runs.sh`, whose selection
`verify-gates.sh` proves.

The exception for `main` protects the full Monday analysis, whose period is
seven days: without it, that run would fall out of the thirty within three
days. It is singular, which prevents it from resurrecting the forty-seven runs
of the `push` trigger removed in #106.

#### What it gives

| | |
| :--- | ---: |
| before | 146 |
| to delete | 86 |
| kept | 60 |

Taken with `--dry-run` on 2026-08-17, before the workflow had run: it is
therefore the selection that is measured, not yet the deletion. The first real
pass is triggered by hand once this file is on `main`, and its summary carries
the actual count.

The 60 split into 30 `ci` and 30 `pull request`. The weekly Monday run was
already among the thirty: the exception had nothing to spare that day, which is
the normal case — it only matters on days when the full analysis has aged by
more than thirty runs. The steady state settles at 60, instead of growing by
fifteen or so a day.

#### Checking

```bash
./src/scripts/prune-runs.sh --dry-run
```

Read-only, and therefore runnable with a personal token: it writes to standard
error the total count, the number to delete and the number kept, and to standard
output the identifiers marked for deletion. After a pass of the pruner, the
number to delete should be zero or close to it.

Each run of the workflow writes its own before/after in its job summary — a
figure frozen in this document would go stale, that one does not.

### 9. The `.rpm` on Fedora

`.github/workflows/fedora.yml`, every Tuesday at 4:00, and by hand through
*Actions → fedora → Run workflow* — issue #266.

**What it does:** builds the `.rpm`, installs it on a `fedora:42` container, and
runs what comes out of it. It is the only check in the repository that plays a
real package manager transaction; everything else inspects the package without
installing it, and a package can be flawless on inspection and refused at
installation — which is exactly what it found on its first pass.

**Why weekly and not per pull request.** It downloads Qt and its dependencies,
close to three hundred megabytes, and the Actions quota has already run out
once. What it exercises changes only with the packaging, which rarely changes.
What guards pull requests in the meantime is local and free:
`check-installation.sh` refuses an `.rpm` that owns a directory outside
`share/subedit`, which was the cause of the defect found.

**No ruleset requires it**, and none must: it does not run on a pull request, so
requiring it would mean waiting forever.

**Tuesday and not Monday**, so as not to get in the way of the full analysis of
`ci.yml` at 4:00 and of the pruner at 6:00.

#### Checking

```bash
make rpm-check
```

The same gesture as the workflow, on the development machine. It needs podman or
docker, and says which one it picked.

### 10. Packages on each tag

`.github/workflows/release.yml`, when a `vX.Y.Z` tag is pushed, and by hand
through *Actions → release → Run workflow* to republish an existing tag — issue
#232.

**What it does, in order:**

1. refuses a name that is not `vX.Y.Z`, even before the checkout;
2. `src/scripts/check-release-tag.sh` — the `project(VERSION)` of the tagged
   commit carries the same number, and that commit is on `main`;
3. installs the toolchain through `setup-toolchain.sh` and runs
   `make packages`;
4. writes the notes through `git-cliff --current`, the changelog section of
   that version;
5. creates the release with the `.deb` and the `.rpm` — or, for a tag already
   published, replaces its files and its notes;
6. `src/scripts/prune-releases.sh` — prunes the patch releases of past
   milestones, except on demand: see retention below.

**No cache and no artifact.** The cache is the storage item that cost; each run
therefore compiles from cold, once per version rather than once per push. The
packages go straight into the release, whose files do not count towards Actions
storage.

**The tag is put on the merge commit in `main`**, once the pull request carrying
the bump is merged:

```console
$ git -C /home/beber/Projects/subedit tag -a v0.10.10 -m "v0.10.10 — …" <merge commit>
$ git -C /home/beber/Projects/subedit push origin v0.10.10
```

A tag put elsewhere — on a working branch, or on a commit whose
`CMakeLists.txt` carries another number — is refused at step 2, and nothing is
built.

#### Retention

| Version | Release and packages | Tag |
| :------ | :------------------- | :-- |
| `vX.Y.0`, minor or major | always kept | kept |
| `vX.Y.Z`, `Z > 0`, current milestone | kept | kept |
| `vX.Y.Z`, `Z > 0`, past milestone | **deleted** | **kept** |

"Current milestone" is the highest `X.Y` pair among the releases. When `v0.11.0`
is published, the releases `0.10.1` to `0.10.N` go with their packages;
`v0.10.0` stays, and so do all the tags.

**A fix on a past milestone is pruned straight away**: a `v0.9.1` pushed after
`v0.10.0` is built, published, then deleted by the same run. The rule makes no
exception for the most recent one.

**Three clarifications, born of #399:**

- **An on-demand rebuild does not prune.** Started through *Run workflow*, the
  workflow republishes the package of the given tag and stops there: the script
  reads `GITHUB_EVENT_NAME`, sees `workflow_dispatch`, says so and exits without
  deleting anything. Without this, rebuilding `v0.9.3` while `v0.10.0` exists
  would recreate the release, then delete it in the same job.
- **A draft is not a version.** Drafts are set aside before the current
  milestone is inferred and before anything is selected: a `v0.12.0` draft
  created by hand does not take away the 0.11 patches.
- **A refusal is red.** The script attempts every deletion, then exits with an
  error naming each release left behind; the publishing job does not stay green
  over a release that was believed pruned.

**After a refusal, the remedy is *Re-run failed jobs*, not *Run workflow*.**
Re-running a red run keeps its event, `push`: the job rebuilds the packages from
cold, replaces the release's files, then prunes — and retries the deletions that
had been refused. *Run workflow* is a `workflow_dispatch`: the script sees it and
does not prune, so the release left behind would stay. For the same reason,
re-running through *Run workflow* the tag that opens a milestone — `v0.11.0` —
does not take away the patches of the previous one; the next push, which
re-reads the list of releases, catches up with them.

`verify-gates.sh` proves the selection on a hand-written list, these three
behaviours against a makeshift `gh` that touches no repository, and two refusals
of the tag guard.

#### Checking

```bash
./src/scripts/prune-releases.sh --dry-run
./src/scripts/check-release-tag.sh v0.10.9
gh release list --repo Guyot-Bertrand/sub-edit
```

The first is read-only and works with a personal token: it writes the tags whose
release would go, and the count to standard error. Each run of the workflow
writes its own before/after in its job summary.

## Issue taxonomy

### Labels

| Family | Values |
| :----- | :----- |
| `type:` | `bug`, `story`, `enhancement`, `task`, `doc` |
| `area:` | `build`, `ci`, `cli`, `core`, `doc`, `format`, `gui`, `i18n`, `scripts`, `test`, `text`, `video` |
| `prio:` | `high`, `medium`, `low` |
| state | `blocked`, `needs-decision` |

The values of `area:` are **the same as the scopes of commit messages**. An
issue and the commits that address it therefore share the same vocabulary, which
makes the generated CHANGELOG directly traceable to the tickets.

### Issue templates

`.github/ISSUE_TEMPLATE/` holds **forms**, not Markdown files: GitHub turns them
into fields, and an empty required field prevents the issue from being opened.
Blank issues are disabled, which forces going through one of them.

| Template | What for | Labels set |
| :------- | :------- | :--------- |
| `story.yml` | a feature seen from the user's side | `type:story` |
| `task.yml` | technical work with no direct effect | `type:task` |
| `bug.yml` | a defect | `type:bug` |
| `enhancement.yml` | an improvement to something existing | `type:enhancement` |
| `phase-review.yml` | the review that closes a phase | `type:task`, `area:doc`, `prio:high` |

**The last one is the only one that arrives pre-filled**, and that is what sets
it apart: the end-of-phase review has lines of inquiry that apply to every
phase — comparing the spec with what was built, propagating cross-references,
re-reading the manual and the ADRs, comparing the measurements — and
rediscovering them every time means forgetting one. What is pre-filled is a
starting point to edit, not a formula to leave as is; two fields say so
explicitly, because the part that matters is the one that changes from one
phase to the next.

What the template **does not try** to pre-fill: what is particular to the phase.
The "points to look at" field is required and its description asks what was
surprising during the phase — a review without a point of its own is a review
nobody did.

### Milestones

One per phase, in the order of the roadmap. The first eight lead to the MVP, the
following ones complete feature parity. `src/scripts/setup-github.sh` creates
them and keeps them up to date; the reference list is in
[`feuille-de-route.md`](feuille-de-route.md).
