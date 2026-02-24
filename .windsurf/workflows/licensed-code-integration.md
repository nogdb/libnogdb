# Licensed Code Integration Workflow

Safe integration of licensed code without AI exposure.

## Steps

1. **Analyze Licensed Code (Human Only)**
   - Review the licensed code manually
   - Identify public API surface
   - Document behavior and contracts

2. **Create Interface**
   - Define TypeScript/Go interface
   - Document each method
   - Specify input/output types
   - Do NOT include implementation details

3. **Implement Adapter (Human Only)**
   - Create adapter class/struct
   - Connect interface to licensed code
   - Add error handling
   - Write integration tests

4. **Share Interface with AI**
   - Only share the interface file
   - Never share the adapter implementation
   - Never share the licensed code

5. **AI Generates Consumer Code**
   - AI works with interface only
   - Generates code that uses the interface
   - Tests against mock implementations

6. **Verify Compliance**
   - Check no licensed code in AI context
   - Validate adapter performance (<10% overhead)
   - Document the integration
