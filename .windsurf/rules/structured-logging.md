---
trigger: glob
globs: ["*.go"]
---

# Structured Logging Standards

## Log Levels

| Level | Usage |
|-------|-------|
| `Error` | Errors that need attention, failures |
| `Warn` | Unexpected but handled situations |
| `Info` | Significant business events |
| `Debug` | Detailed diagnostic information |
| `Trace` | Very detailed diagnostic (e.g., full payloads in dev) |

## Universal Context Fields

These three fields MUST appear in ALL log entries:

| Field | Meaning | Values |
|-------|---------|--------|
| `x_request_id` | Transaction ID — correlates all logs for one operation | UUID, `job-checkpayment-<uuid>` |
| `source` | Source Channel — where the request originated | Client IP, `scheduler`, `nats`, `cli`, `startup` |
| `userid` | Actor — who initiated the action | Username, `system`, `scheduler`, `anonymous` |

## Context Propagation Patterns

### Pattern A: HTTP Handlers (gin.Context)

```go
func (h *Handler) HandleSomething(c *gin.Context) {
    // ✅ GOOD: Uses context-aware logger
    getLog(c).Infof("Processing request")
    getLog(c).WithField("inst_id", instID).Infof("Business operation")
    
    // ❌ BAD: Loses context (no x_request_id, source, userid)
    log.Infof("Processing request")
}
```

### Pattern B: Background Jobs / Scheduled Tasks

```go
func (h *Handler) checkPaymentThread() {
    ctx := xrequestid.SetupContext(context.Background(), xrequestid.RequestData{
        XRequestId: "job-checkpayment-" + uuid.NewString(),
        Source:     "scheduler",
        User:       "system",
    })
    
    logger := xrequestid.GetLog(ctx)
    logger.Infof("Starting check payment routine")
    h.processPayments(ctx)
}
```

### Pattern C: Database / Service Methods

```go
// ✅ GOOD: Accepts context, preserves tracing
func (db *PGDB) UpdateFilingRequest(ctx context.Context, domain string, param *Param) error {
    logger := xrequestid.GetLog(ctx)
    logger.WithField("api_ref", param.APIRefNo).Infof("Updating filing request")
}

// ❌ BAD: No context, loses tracing
func (db *PGDB) UpdateFilingRequest(domain string, param *Param) error {
    log.Infof("Updating filing request")  // No x_request_id!
}
```

### Pattern D: Startup / Configuration

```go
func main() {
    startupLog := log.WithFields(log.Fields{
        "component": "startup",
        "source":    "main",
        "userid":    "system",
    })
    
    startupLog.Infof("Loading configuration")
}
```

## Structured Logging Best Practices

```go
// ✅ GOOD: Structured with context
getLog(c).WithFields(log.Fields{
    "action":  "submit_filing",
    "inst_id": instID,
    "api_ref": apiRefNo,
}).Info("filing submitted successfully")

// ❌ BAD: String interpolation (harder to parse, query)
log.Infof("filing %s submitted for api_ref %s", instID, apiRefNo)
```

## Required Log Fields by Operation

| Operation | Required Fields | Optional Fields |
|-----------|-----------------|------------------|
| **All Operations** | `x_request_id`, `source`, `userid` | `duration_ms` |
| **Business Events** | `action`, `req_id` | `api_ref`, `inst_ref`, `status` |
| **External Calls** | `endpoint` | `response_code`, `response_message`, `retry_count` |
| **Errors** | `error` | `error_code`, `stack_trace` |

## Sensitive Data - NEVER LOG

| Data Type | Example |
|-----------|---------|
| Passwords | `password`, `secret` |
| API Keys | `api_key`, `token` |
| Personal IDs | `tax_id`, `citizen_id` |
| Financial Data | `account_number`, `credit_card` |
| Request Bodies | Full JSON with sensitive fields |
| JWT Tokens | Full token strings |

```go
// ❌ BAD: Logging sensitive data
log.Infof("User login: %s, password: %s", username, password)
log.Infof("Request body: %s", string(body))

// ✅ GOOD: Sanitized logging
log.WithField("username", username).Info("user login attempt")
log.WithField("content_length", len(body)).Info("request received")
```

## Forbidden Patterns

- ❌ Logging without context fields
- ❌ String interpolation instead of structured fields
- ❌ Logging sensitive data
- ❌ Using global logger without context
- ❌ Excessive logging (log significant events only)
