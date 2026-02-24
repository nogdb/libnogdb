---
name: code-review
description: Comprehensive code review process with security, performance, and quality checks
---

# Code Review Skill

This skill guides you through a thorough code review process.

## Review Areas

### Security Review
- Input validation
- SQL injection prevention
- XSS prevention
- Authentication checks
- Authorization checks
- Sensitive data handling

### Performance Review
- Database query efficiency
- N+1 query detection
- Caching opportunities
- Response time impact
- Memory usage

### Quality Review
- Test coverage
- Error handling
- Logging
- Documentation
- Code style consistency

### License Compliance
- No licensed code in AI context
- Interface-first pattern used
- Adapter implementation correct

## Review Commands
```bash
# Run security scan
npm run security:scan

# Check test coverage
npm run test:coverage

# Run performance analysis
npm run perf:analyze
```

## Feedback Template
```
## Summary
[Brief overview]

## Security
- [ ] Passed / [Issues found]

## Performance
- [ ] Passed / [Issues found]

## Quality
- [ ] Passed / [Issues found]

## Recommendation
[Approve / Request Changes]
```
