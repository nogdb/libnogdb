#!/usr/bin/env bash
# =============================================================================
# agent-merge-worktrees.sh
# =============================================================================
# Discovers all active agent worktree branches and merges them into main.
# Called by /agent-01-coordination merge.
#
# Usage:
#   agent-merge-worktrees.sh [--dry-run] <main-repo-root>
#
# Output format (one block per branch):
#   BRANCH: cascade/agent-01-coordination-start-504d25
#   STATUS: merged_clean | needs_resolution | skipped | error
#   CONFLICTS: path/to/file.rs path/to/file.ts   (only when needs_resolution)
#   ---
#
# Exit codes:
#   0 — all branches processed (some may need resolution)
#   1 — fatal error (not on main, repo dirty before start, etc.)
# =============================================================================

set -euo pipefail

# ── Args ─────────────────────────────────────────────────────────────────────
DRY_RUN=false
MAIN_ROOT=""

for arg in "$@"; do
    case "$arg" in
        --dry-run) DRY_RUN=true ;;
        *) MAIN_ROOT="$arg" ;;
    esac
done

if [[ -z "$MAIN_ROOT" ]]; then
    echo "ERROR: main-repo-root argument required"
    echo "Usage: merge-branches.sh [--dry-run] <main-repo-root>"
    exit 1
fi

# ── Validate environment ──────────────────────────────────────────────────────
cd "$MAIN_ROOT"

CURRENT_BRANCH=$(git branch --show-current)
if [[ "$CURRENT_BRANCH" != "main" ]]; then
    echo "ERROR: Must be on main branch to merge. Currently on: $CURRENT_BRANCH"
    echo "Run: git checkout main"
    exit 1
fi

# Check for uncommitted changes
if ! git diff --quiet || ! git diff --cached --quiet; then
    echo "ERROR: Working tree has uncommitted changes. Commit or stash before merging."
    exit 1
fi

# ── Discover agent branches ───────────────────────────────────────────────────
# Parse git worktree list --porcelain to get branch per worktree
AGENT_BRANCHES=()

while IFS= read -r line; do
    if [[ "$line" =~ ^branch\ refs/heads/(cascade/agent-01-coordination-start-.+)$ ]]; then
        AGENT_BRANCHES+=("${BASH_REMATCH[1]}")
    fi
done < <(git worktree list --porcelain)

if [[ ${#AGENT_BRANCHES[@]} -eq 0 ]]; then
    echo "INFO: No agent branches found (no worktrees matching cascade/agent-01-coordination-start-*)"
    exit 0
fi

echo "DISCOVERED: ${#AGENT_BRANCHES[@]} agent branch(es)"
for b in "${AGENT_BRANCHES[@]}"; do
    echo "  - $b"
done
echo "---"

# ── Process each branch ───────────────────────────────────────────────────────
MERGED_CLEAN=0
NEEDS_RESOLUTION=0
SKIPPED=0
ERRORS=0

for BRANCH in "${AGENT_BRANCHES[@]}"; do

    echo "BRANCH: $BRANCH"

    # Check if branch has any commits ahead of main
    AHEAD=$(git rev-list --count "main..${BRANCH}" 2>/dev/null || echo "0")
    if [[ "$AHEAD" -eq 0 ]]; then
        echo "STATUS: skipped"
        echo "REASON: No commits ahead of main"
        echo "---"
        SKIPPED=$((SKIPPED + 1))
        continue
    fi

    if [[ "$DRY_RUN" == "true" ]]; then
        # Dry run: use merge-tree to detect conflicts without touching working tree
        MERGE_BASE=$(git merge-base main "$BRANCH" 2>/dev/null || echo "")
        if [[ -z "$MERGE_BASE" ]]; then
            echo "STATUS: error"
            echo "REASON: Cannot find merge base"
            echo "---"
            ERRORS=$((ERRORS + 1))
            continue
        fi

        # git merge-tree exits 0 for clean, 1 for conflicts
        MERGE_TREE_OUT=$(git merge-tree --write-tree main "$BRANCH" 2>&1 || true)
        CONFLICT_FILES=$(echo "$MERGE_TREE_OUT" | grep "^CONFLICT" | awk '{print $NF}' | tr '\n' ' ' | xargs || true)

        if [[ -n "$CONFLICT_FILES" ]]; then
            echo "STATUS: needs_resolution"
            echo "CONFLICTS: $CONFLICT_FILES"
        else
            echo "STATUS: merged_clean"
        fi
        echo "---"
        continue
    fi

    # Real merge: attempt --no-commit so we can inspect conflicts
    if git merge --no-ff --no-commit "$BRANCH" 2>/dev/null; then
        # No conflicts — commit
        COMMIT_MSG="merge(agents): integrate ${BRANCH} into main"
        git commit --no-edit -m "$COMMIT_MSG" 2>/dev/null || git commit -m "$COMMIT_MSG"
        echo "STATUS: merged_clean"
        echo "---"
        MERGED_CLEAN=$((MERGED_CLEAN + 1))
    else
        # Conflicts detected
        CONFLICT_FILES=$(git diff --name-only --diff-filter=U | tr '\n' ' ' | xargs)
        echo "STATUS: needs_resolution"
        echo "CONFLICTS: $CONFLICT_FILES"
        echo "---"
        NEEDS_RESOLUTION=$((NEEDS_RESOLUTION + 1))
        # Leave merge state active so orchestrator can resolve and continue
        # Orchestrator must: resolve files → git add → git merge --continue
        # Only process one conflicting branch at a time
        break
    fi

done

# ── Summary ───────────────────────────────────────────────────────────────────
echo "SUMMARY:"
echo "  merged_clean:      $MERGED_CLEAN"
echo "  needs_resolution:  $NEEDS_RESOLUTION"
echo "  skipped:           $SKIPPED"
echo "  errors:            $ERRORS"

if [[ "$NEEDS_RESOLUTION" -gt 0 ]]; then
    echo "ACTION_REQUIRED: Resolve conflicts listed above, then run merge again."
fi
