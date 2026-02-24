---
name: session-handoff
description: Multi-session work continuity with context preservation
---

# Session Handoff Skill

Preserve context between AI sessions for long-running projects.

## When to Use

- Ending a work session
- Starting a new session
- Handing off to another developer
- Documenting progress for future reference

## Session End Protocol

### 1. Create Handoff Document

```markdown
# Session Handoff - [Date]

## What Was Accomplished
- [x] Implemented user authentication flow
- [x] Added password reset functionality
- [x] Created unit tests for auth service

## Current State
- Branch: `feature/user-auth`
- Last commit: `abc123 - feat(auth): add password reset`
- All tests passing: Yes
- Build status: Green

## In Progress
- [ ] Email verification flow (50% complete)
  - Created email service interface
  - Need to implement SendGrid adapter

## Blockers
- Waiting for SendGrid API key from DevOps

## Next Steps
1. Complete SendGrid adapter implementation
2. Add email verification to registration flow
3. Write integration tests for email flow
4. Update API documentation

## Key Files Modified
- `src/services/auth.ts` - Main auth service
- `src/services/email.ts` - Email service interface
- `tests/auth.test.ts` - Auth tests

## Important Context
- Using argon2 for password hashing (not bcrypt)
- Email templates are in `templates/email/`
- Rate limiting is 5 attempts per 15 minutes

## Commands to Resume
```bash
git checkout feature/user-auth
npm install
npm test
```
```

### 2. Save to Project

```bash
# Create handoff file
echo "$(cat handoff.md)" > .handoff/$(date +%Y-%m-%d).md

# Or use a dedicated location
mkdir -p docs/handoffs
mv handoff.md docs/handoffs/$(date +%Y-%m-%d)-auth-feature.md
```

## Session Start Protocol

### 1. Read Previous Handoff

```bash
# Find latest handoff
ls -la .handoff/ | tail -1

# Or check docs
cat docs/handoffs/latest.md
```

### 2. Verify State

```bash
# Check current branch
git branch --show-current

# Check for uncommitted changes
git status

# Run tests to verify state
npm test

# Check build
npm run build
```

### 3. Resume Checklist

- [ ] Read handoff document
- [ ] Checkout correct branch
- [ ] Pull latest changes
- [ ] Install dependencies
- [ ] Run tests
- [ ] Review blockers
- [ ] Start on next steps

## Handoff Templates

### Feature Development

```markdown
# Feature: [Name] - Session [N]

## Progress: [X]% Complete

### Completed This Session
- 

### Remaining Work
- 

### Technical Decisions Made
- 

### Tests Added
- 

### Resume Command
```bash
git checkout [branch] && npm install && npm test
```
```

### Bug Fix

```markdown
# Bug Fix: [Issue #] - Session Handoff

## Bug Description
[Original bug description]

## Root Cause
[What we found]

## Fix Status
- [ ] Root cause identified
- [ ] Fix implemented
- [ ] Tests added
- [ ] Verified fix works
- [ ] No regressions

## Investigation Notes
[What was tried, what worked, what didn't]

## Files to Review
- 

## Reproduction Steps
1. 
```

### Code Review

```markdown
# Code Review: PR #[Number] - Handoff

## Review Status
- [ ] Security review
- [ ] Performance review
- [ ] Test coverage review
- [ ] Documentation review

## Comments Made
- [file:line] - [comment]

## Pending Discussion
- 

## Blocking Issues
- 

## Approval Status
[Approved / Changes Requested / Pending]
```

## Automated Handoff

```typescript
// scripts/create-handoff.ts
import { execSync } from 'child_process';
import { writeFileSync } from 'fs';

function createHandoff() {
  const branch = execSync('git branch --show-current').toString().trim();
  const lastCommit = execSync('git log -1 --oneline').toString().trim();
  const status = execSync('git status --short').toString().trim();
  const date = new Date().toISOString().split('T')[0];
  
  const handoff = `# Session Handoff - ${date}

## Current State
- Branch: \`${branch}\`
- Last commit: \`${lastCommit}\`
- Uncommitted changes: ${status ? 'Yes' : 'No'}

${status ? `### Uncommitted Files\n\`\`\`\n${status}\n\`\`\`` : ''}

## What Was Accomplished
- 

## Next Steps
1. 

## Important Context
- 
`;

  writeFileSync(`docs/handoffs/${date}-handoff.md`, handoff);
  console.log(`Created handoff: docs/handoffs/${date}-handoff.md`);
}

createHandoff();
```

## Best Practices

1. **Write handoffs as you go** - Don't wait until end of session
2. **Be specific** - Include file names, line numbers, exact commands
3. **Document decisions** - Why, not just what
4. **Include blockers** - What's preventing progress
5. **Test before handoff** - Ensure clean state
6. **Commit before handoff** - No uncommitted work
