# Release Workflow

Steps for creating and deploying a release.

## Steps

1. **Prepare Release**
   ```bash
   git checkout develop
   git pull origin develop
   ```

2. **Create Release Branch**
   ```bash
   git checkout -b release/v[X.Y.Z]
   ```

3. **Update Version**
   - Update package.json / go.mod
   - Update CHANGELOG.md
   - Commit changes

4. **Run Full Test Suite**
   ```bash
   npm run test:all
   npm run test:e2e
   ```

5. **Create Release PR**
   ```bash
   tea pr create --base main --title "release: v[X.Y.Z]"
   ```

6. **Deploy to Staging**
   - Verify staging deployment
   - Run smoke tests
   - Check monitoring

7. **Merge to Main**
   ```bash
   tea pr merge --style squash [PR-ID]
   ```

8. **Tag Release**
   ```bash
   git tag v[X.Y.Z]
   git push origin v[X.Y.Z]
   ```

9. **Deploy to Production**
   - Monitor deployment
   - Verify health checks
   - Have rollback ready
