# Gitlite Design

## 1. Architecture

`SomeObj` is only the command facade required by `main.cpp`.  The actual state and shared algorithms live in `Repository`, while each assignment stage is implemented in a separate translation unit (`Subtask1.cpp` through `Subtask5.cpp`).  Bonus commands are split into `Remote.cpp` and `Diff.cpp`.  `Commit` is an immutable-in-practice value object containing a message, timestamp, ordered parent list, and a filename-to-blob map.

This split keeps command parsing, repository persistence, history algorithms, and presentation logic separate.  It also makes each subtask independently buildable, matching the staged development history in this repository.

## 2. Persistent repository layout

A repository uses the following layout under `.gitlite`:

```text
.gitlite/
├── HEAD                 # current branch name
├── branches             # branch -> commit id map
├── tags                 # tag -> commit id map
├── remotes              # remote name -> path map
├── stage_add            # filename -> blob id map
├── stage_rm             # set of filenames staged for removal
└── objects/
    ├── commits/          # one serialized Commit per SHA-1 id
    └── blobs/            # raw file contents, addressed by SHA-1
```

The maps use a deterministic, length/count based text representation with quoted strings.  Branch names such as `origin/master` are logical map keys rather than filesystem paths, so remote-tracking branches do not conflict with ordinary references.

Blob IDs are SHA-1 hashes of the exact file bytes.  Commit IDs are SHA-1 hashes of a deterministic serialization containing the message, Unix timestamp, ordered parents, and a `std::map` snapshot.  Because map iteration is ordered, equal commits produce equal IDs.  The initial commit always has timestamp `0`, message `initial commit`, no parents, and an empty snapshot, so every repository gets the same initial commit ID.

## 3. Staging and revisions

`add` writes the current file bytes into the blob store immediately and stages the resulting blob ID; therefore later working-tree edits do not change what a subsequent `commit` records.  If the file equals the HEAD version, both an addition and a removal mark are cleared.  `rm` either unstages a newly-added file or stages removal of a tracked file and removes its working copy.

A revision is resolved by first checking an exact tag name, then by matching commit-ID prefixes.  If several IDs share the prefix, the lexicographically smallest ID is chosen, as required by the specification.  Empty revisions never resolve.

## 4. Ignore rules and working-tree safety

`.gitliteignore` is parsed on every command that needs it.  The matcher implements exactly the project subset: `*`, `?`, backslash escapes, root-leading `/`, comments, negation with `!`, and last-match-wins behavior.  Tracked or already-staged additions are not hidden by ignore rules.

Checkout, reset, fast-forward merge, and ordinary merge all perform an untracked-file safety check before modifying the working tree.  Ignored untracked files are allowed to be overwritten.  Ordinary merge checks only paths that the merge will actually write or delete, so unrelated untracked or modified files are preserved.

## 5. Branches, checkout, and reset

`HEAD` stores a branch name; there is no detached-HEAD mode.  A branch maps to the commit at its tip.  `commit` advances only the current branch.  File checkout replaces just one file and leaves refs/staging untouched.  Branch checkout and reset reuse the same snapshot-application helper: target-tracked files are written, files tracked only by the old commit are removed, and the staging area is cleared after a successful operation.

## 6. Merge

To find the split point, Gitlite first collects all ancestors of the current head by traversing every parent.  It then performs a breadth-first search from the given branch, enqueuing parents in stored order; the first commit also in the current ancestor set is the deterministic split point required by the README.

Fast-forward and ancestor cases are handled before the general three-way merge.  For a normal merge, each filename in the union of split/current/given snapshots is classified by comparing blob IDs.  Changes made only on the given side are adopted, current-only changes are kept, and incompatible changes produce a whole-file conflict:

```text
<<<<<<< HEAD
<current contents>
=======
<given contents>
>>>>>>>
```

Missing files are treated as empty on that side.  A normal merge always creates a two-parent commit, even if the resulting snapshot is identical to the current one.

## 7. Remotes

A remote stores the path supplied by the user and resolves relative paths at operation time.  `fetch` copies all missing commit/blob objects into the local object store and updates `<remote>/<branch>`.  `push` requires the remote tip to be an ancestor of the local tip, copies missing objects, and advances only the requested remote branch (never the remote working tree or HEAD).  `pull` is implemented as `fetch` followed by the ordinary merge routine.

## 8. Diff and show

Diff works on complete lines and computes an LCS edit script.  When deletion and insertion give the same LCS length, deletion is chosen first for deterministic output.  Hunks include three context lines and are merged when separated by at most six unchanged lines.  Raw `\r` bytes are preserved, and missing final newlines produce the required `\\ No newline at end of file` marker.  `show` prints the normal log entry, then diffs the commit against its first parent (or an empty snapshot for the initial commit).
