---
name: commit-work
description: Git workflow automation with conventional commits and best practices
---

# Commit Work Skill

Structured approach to committing code changes.

## When to Use

- Ready to commit changes
- Preparing pull requests
- Maintaining clean git history

## Conventional Commits

```
<type>(<scope>): <description>

[optional body]

[optional footer(s)]
```

### Types

| Type | Description | Example |
|------|-------------|---------|
| `feat` | New feature | `feat(auth): add password reset` |
| `fix` | Bug fix | `fix(api): handle null response` |
| `docs` | Documentation | `docs(readme): update install steps` |
| `style` | Formatting | `style: fix indentation` |
| `refactor` | Code restructure | `refactor(user): extract validation` |
| `test` | Add/fix tests | `test(auth): add login tests` |
| `chore` | Maintenance | `chore(deps): update typescript` |
| `ci` | CI/CD changes | `ci: add deploy workflow` |
| `perf` | Performance | `perf(query): add index` |

### Scope

Optional, indicates the affected area:

```bash
feat(auth): add OAuth support
fix(api/users): handle missing email
refactor(components/button): simplify props
```

### Breaking Changes

```bash
feat(api)!: change response format

BREAKING CHANGE: API responses now use camelCase keys instead of snake_case.

Migration:
- response.user_id → response.userId
- response.created_at → response.createdAt
```

## Pre-Commit Checklist

```bash
# 1. Check what's staged
git status
git diff --staged

# 2. Run tests
npm test

# 3. Run linting
npm run lint

# 4. Type check
npm run typecheck

# 5. Review changes one more time
git diff --staged --stat
```

## Commit Workflow

### Single Logical Change

```bash
# Stage specific files
git add src/services/auth.ts
git add tests/auth.test.ts

# Commit with message
git commit -m "feat(auth): add password reset flow

- Add resetPassword method to AuthService
- Create password reset email template
- Add rate limiting (5 requests/hour)

Closes #123"
```

### Interactive Staging

```bash
# Stage parts of files
git add -p

# For each hunk:
# y - stage this hunk
# n - don't stage
# s - split into smaller hunks
# e - manually edit
```

### Amending Commits

```bash
# Fix last commit message
git commit --amend -m "fix(auth): correct typo in error message"

# Add forgotten files to last commit
git add forgotten-file.ts
git commit --amend --no-edit
```

## Commit Message Templates

### Feature

```
feat(<scope>): <short description>

<What was added and why>

- <Bullet point of change 1>
- <Bullet point of change 2>

Closes #<issue>
```

### Bug Fix

```
fix(<scope>): <what was fixed>

<Root cause of the bug>

<How it was fixed>

Fixes #<issue>
```

### Refactor

```
refactor(<scope>): <what was refactored>

<Why the refactor was needed>

No functional changes.
```

## Git Hooks

### Pre-commit Hook

```bash
#!/bin/sh
# .husky/pre-commit

npm run lint-staged
npm run typecheck
```

### Commit-msg Hook

```bash
#!/bin/sh
# .husky/commit-msg

npx --no -- commitlint --edit "$1"
```

### Commitlint Config

```javascript
// commitlint.config.js
module.exports = {
  extends: ['@commitlint/config-conventional'],
  rules: {
    'type-enum': [
      2,
      'always',
      ['feat', 'fix', 'docs', 'style', 'refactor', 'test', 'chore', 'ci', 'perf']
    ],
    'scope-case': [2, 'always', 'kebab-case'],
    'subject-case': [2, 'always', 'lower-case'],
    'body-max-line-length': [2, 'always', 100]
  }
};
```

## Branch Strategy

```bash
# Create feature branch
git checkout -b feature/user-auth

# Work and commit
git add .
git commit -m "feat(auth): implement login"

# Keep up to date with main
git fetch origin
git rebase origin/main

# Push for PR
git push -u origin feature/user-auth
```

## Squashing Commits

```bash
# Interactive rebase to squash
git rebase -i HEAD~3

# In editor, change 'pick' to 'squash' or 's'
pick abc123 feat(auth): add login
squash def456 fix typo
squash ghi789 add tests

# Save and edit combined commit message
```

## Undoing Commits

```bash
# Undo last commit, keep changes staged
git reset --soft HEAD~1

# Undo last commit, keep changes unstaged
git reset HEAD~1

# Undo last commit, discard changes (careful!)
git reset --hard HEAD~1

# Revert a pushed commit (creates new commit)
git revert <commit-hash>
```

## Best Practices

1. **Atomic commits** - One logical change per commit
2. **Test before commit** - All tests should pass
3. **Clear messages** - Future you will thank you
4. **Reference issues** - Link to tickets/issues
5. **Don't commit secrets** - Use .gitignore and .env
6. **Review diffs** - Check what you're committing

## Quick Reference

```bash
# Stage all changes
git add .

# Stage specific file
git add path/to/file

# Unstage file
git reset HEAD path/to/file

# Discard changes
git checkout -- path/to/file

# View commit history
git log --oneline -10

# View specific commit
git show <commit-hash>
```
