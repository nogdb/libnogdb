# Spec Kit Workflow (DEPRECATED)

> **⚠️ DEPRECATED:** This workflow has been replaced by the **aspec workflow** (Steps 1-20). Use `/aspec-01-project-spec` through `/aspec-20-implement` instead. See `aspec-*.md` workflow files for the current process.

Transform specifications into working code using AI agents.

## Commands

| Command | Purpose | Primary Agent |
|---------|---------|---------------|
| `/constitution` | Establish project principles | Tech Lead |
| `/specify` | Define requirements | Cascade |
| `/clarify` | Resolve ambiguities | Cascade |
| `/plan` | Choose tech stack | Tech Lead |
| `/tasks` | Break down into items | Testing Agent |
| `/analyze` | Analyze implementation | Review Agent |
| `/implement` | Generate code | Cascade |

## Steps

### 1. Initialize Project
```bash
specify init
```

### 2. Establish Constitution
```
/constitution
- Define project principles
- Set constraints and boundaries
- Establish coding standards
```

### 3. Write Specification
```
/specify [detailed feature description]

Example:
/specify Create a user authentication service with:
- Login with email/password
- JWT token generation
- Password reset via email
- Rate limiting (5 attempts per minute)
```

### 4. Clarify Requirements
```
/clarify [ambiguous requirement]

Example:
/clarify Should password reset tokens expire?
```

### 5. Plan Architecture
```
/plan

Spec Kit + Tech Lead will:
- Choose appropriate tech stack
- Define database schema
- Plan API endpoints
- Identify dependencies
```

### 6. Generate Tasks
```
/tasks

Output:
- Task 1: Set up database schema
- Task 2: Implement JWT service
- Task 3: Create login endpoint
- Task 4: Create password reset flow
- Task 5: Add rate limiting
```

### 7. Implement
```
/implement

Cascade generates:
- Complete service code
- Database migrations
- API endpoints
- Input validation
- Unit tests
```

### 8. Analyze
```
/analyze

Review Agent checks:
- Security vulnerabilities
- Performance issues
- Best practices compliance
```

## Agent Collaboration

```
/specify → Cascade generates spec
    ↓
/plan → Tech Lead reviews architecture
    ↓
/tasks → Testing Agent validates tasks
    ↓
/implement → Cascade writes code
    ↓
Testing Agent → Generates tests
    ↓
Review Agent → Security review
    ↓
Documentation Agent → Updates docs
```

## Best Practices

1. **Be specific** in your `/specify` command
2. **Use `/clarify`** for any ambiguous requirements
3. **Review `/plan`** output before implementing
4. **Run `/analyze`** after implementation
5. **Let agents collaborate** - don't skip steps
