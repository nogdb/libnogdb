# Code Review Workflow

A structured approach to reviewing pull requests.

## Steps

1. **Checkout the PR**
   ```bash
   tea pr checkout [PR-ID]
   ```

2. **Run Automated Checks**
   ```bash
   npm test
   npm run lint
   ```

3. **AI Agent Review**
   - Security analysis
   - Performance check
   - Standards compliance

4. **Manual Review**
   - Architecture alignment
   - Business logic correctness
   - Edge cases covered

5. **License Compliance Check**
   - Verify no licensed code exposed
   - Check interface usage
   - Validate adapter implementation

6. **Provide Feedback**
   ```bash
   tea pr review [PR-ID] --comment "Feedback here"
   ```

7. **Approve or Request Changes**
   ```bash
   # If approved
   tea pr review [PR-ID] --approve
   
   # If changes needed
   tea pr review [PR-ID] --reject --comment "Changes needed"
   ```
